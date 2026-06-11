#include "SnoBeeComponent.h"
#include "SnoBeeRegistry.h"
#include "PlayerRegistry.h"
#include "LevelGrid.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "HealthComponent.h"
#include "PengoComponent.h"
#include "Pengo.h"
#include "PointsComponent.h"
#include "Scoring.h"
#include "GameEvents.h"
#include "IceBlock.h"
#include <SDL3/SDL.h>
#include <glm/geometric.hpp>
#include <random>
#include <array>
#include <cmath>
#include <cstdlib>

namespace
{
    std::mt19937& Rng()
    {
        static std::mt19937 rng{ std::random_device{}() };
        return rng;
    }
}

namespace dae
{
    SnoBeeComponent::SnoBeeComponent(GameObject* pOwner, int gridX, int gridY, bool playSpawnAnim)
        : Component(pOwner)
        , m_spawnGridPos(gridX, gridY)
        , m_gridPos(gridX, gridY)
        , m_targetGridPos(gridX, gridY)
    {
        auto& grid = LevelGrid::GetInstance();
        glm::vec2 world = grid.GridToWorld(gridX, gridY);
        pOwner->SetLocalPosition(world.x, world.y, 0.0f);

        SnoBeeRegistry::GetInstance().Register(this);

        if (playSpawnAnim)
        {
            m_state = State::Spawning;
            m_animTimer = 0.0f;
            SetAbsFrame(SPAWN_ROW, 0);
        }
        else
        {
            m_state = State::Wandering;
            SetRowFrame(WALK_ROW, 0);
        }
    }

    SnoBeeComponent::~SnoBeeComponent()
    {
        SnoBeeRegistry::GetInstance().Unregister(this);
    }

    void SnoBeeComponent::Update(float deltaTime)
    {
        switch (m_state)
        {
        case State::Spawning:
            UpdateSpawning(deltaTime);
            break;
        case State::Stunned:
            UpdateStunned(deltaTime);
            break;
        case State::Eating:
            UpdateEating(deltaTime);
            CheckPengoContact();
            break;
        case State::Wandering:
        {
            if (m_isMoving)
            {
                UpdateGridMovement(deltaTime);
            }
            else
            {
                m_decisionCooldown -= deltaTime;
                m_eatCooldown -= deltaTime;
                if (!m_playerControlled && m_decisionCooldown <= 0.0f)
                    ChooseNextMove();
            }

            m_animTimer += deltaTime;
            SetRowFrame(WALK_ROW, static_cast<int>(m_animTimer * 6.0f) % 2);

            CheckPengoContact();
            break;
        }
        case State::Carried:
            break;
        case State::Crushed:
            UpdateCrushed(deltaTime);
            break;
        }
    }

    void SnoBeeComponent::UpdateSpawning(float dt)
    {
        m_animTimer += dt;

        if (m_animTimer >= SPAWN_DURATION)
        {
            m_state = State::Wandering;
            m_animTimer = 0.0f;
            m_decisionCooldown = 0.2f;
            SetRowFrame(WALK_ROW, 0);
            return;
        }

        const int frame = static_cast<int>(
            (m_animTimer / SPAWN_DURATION) * static_cast<float>(SPAWN_FRAMES));
        SetAbsFrame(SPAWN_ROW, frame < SPAWN_FRAMES ? frame : SPAWN_FRAMES - 1);
    }

    void SnoBeeComponent::SetPlayerControlled(bool controlled)
    {
        m_playerControlled = controlled;
        if (m_state == State::Wandering)
            SetRowFrame(WALK_ROW, 0);
        else if (m_state == State::Stunned)
            SetAbsFrame(SPAWN_ROW, STUN_FRAME0);
    }

    void SnoBeeComponent::TryMove(SnoBeeDirection dir)
    {
        if (!m_playerControlled || m_state != State::Wandering || m_isMoving)
            return;

        const glm::ivec2 next = m_gridPos + DirToDelta(dir);
        m_direction = dir;
        if (LevelGrid::GetInstance().IsPositionBlocked(next.x, next.y))
            return;

        m_targetGridPos = next;
        m_isMoving = true;
    }

    void SnoBeeComponent::Stun(float duration)
    {
        if (m_state != State::Wandering && m_state != State::Stunned
            && m_state != State::Eating)
            return;

        m_state = State::Stunned;
        m_stunTimer = duration;
        m_animTimer = 0.0f;
        SetAbsFrame(SPAWN_ROW, STUN_FRAME0);
    }

    bool SnoBeeComponent::FindChaseTarget(glm::ivec2& outGridPos) const
    {
        const glm::vec3 myPos = GetOwner()->GetLocalPosition();
        const GameObject* best = nullptr;
        float bestDist = 0.f;

        for (GameObject* player : PlayerRegistry::GetInstance().GetAll())
        {
            auto* pengoComp = player->GetComponent<PengoComponent>();
            if (!pengoComp || pengoComp->GetPengo()->IsDying()) continue;

            const glm::vec3 p = const_cast<GameObject*>(player)->GetWorldPosition();
            const glm::vec2 d{ p.x - myPos.x, p.y - myPos.y };
            const float dist = d.x * d.x + d.y * d.y;
            if (!best || dist < bestDist) { best = player; bestDist = dist; }
        }
        if (!best) return false;

        const glm::vec3 p = const_cast<GameObject*>(best)->GetWorldPosition();
        outGridPos = LevelGrid::GetInstance().WorldToGrid(p.x, p.y);
        return true;
    }

    void SnoBeeComponent::UpdateEating(float dt)
    {
        auto* block = LevelGrid::GetInstance().GetBlockAt(m_eatTargetCell.x, m_eatTargetCell.y);
        if (!block || block->IsSliding() || block->GetBlockType() != BlockType::Normal)
        {
            m_state = State::Wandering;
            m_decisionCooldown = 0.1f;
            return;
        }

        m_eatTimer += dt;

        SetRowFrame(WALK_ROW, static_cast<int>(m_eatTimer * 10.0f) % 2);

        if (m_eatTimer >= EAT_DURATION)
        {
            block->Disintegrate();
            m_state = State::Wandering;
            m_eatCooldown = EAT_COOLDOWN;
            m_decisionCooldown = 0.2f;
        }
    }

    void SnoBeeComponent::UpdateStunned(float dt)
    {
        m_stunTimer -= dt;
        m_animTimer += dt;

        const int frame = STUN_FRAME0 + (static_cast<int>(m_animTimer * STUN_ANIM_FPS) % 2);
        SetAbsFrame(SPAWN_ROW, frame);

        const glm::vec3 myPos = GetOwner()->GetLocalPosition();
        for (GameObject* player : PlayerRegistry::GetInstance().GetAll())
        {
            auto* pengoComp = player->GetComponent<PengoComponent>();
            if (!pengoComp || pengoComp->GetPengo()->IsDying()) continue;

            const glm::vec3 pPos = player->GetWorldPosition();
            const glm::vec2 diff{ pPos.x - myPos.x, pPos.y - myPos.y };
            if (glm::length(diff) < CONTACT_RADIUS)
            {
                if (auto* points = player->GetComponent<PointsComponent>())
                    points->AddPoints(Scoring::StunnedSnoBeeCaught, GameEvent::EnemyDied);
                Crush(DirToDelta(m_direction));
                return;
            }
        }

        if (m_stunTimer <= 0.0f)
        {
            m_state = State::Wandering;
            m_animTimer = 0.0f;
            m_decisionCooldown = 0.1f;
            SetRowFrame(WALK_ROW, 0);
        }
    }

    void SnoBeeComponent::CheckPengoContact()
    {
        const glm::vec3 myPos = GetOwner()->GetLocalPosition();

        for (GameObject* player : PlayerRegistry::GetInstance().GetAll())
        {
            auto* pengoComp = player->GetComponent<PengoComponent>();
            if (!pengoComp) continue;

            if (pengoComp->GetPengo()->IsDying()) continue;

            const glm::vec3 pPos = player->GetWorldPosition();
            const glm::vec2 diff{ pPos.x - myPos.x, pPos.y - myPos.y };
            if (glm::length(diff) < CONTACT_RADIUS)
            {
                if (auto* health = player->GetComponent<HealthComponent>())
                    health->Die();
            }
        }
    }

    void SnoBeeComponent::ResetToSpawn()
    {
        if (m_state == State::Crushed) return;

        auto& grid = LevelGrid::GetInstance();
        m_state = State::Wandering;
        m_gridPos = m_spawnGridPos;
        m_targetGridPos = m_spawnGridPos;
        m_isMoving = false;
        m_decisionCooldown = 1.0f;

        glm::vec2 world = grid.GridToWorld(m_spawnGridPos.x, m_spawnGridPos.y);
        GetOwner()->SetLocalPosition(world.x, world.y, 0.0f);
        SetRowFrame(WALK_ROW, 0);
    }

    void SnoBeeComponent::StartCarried(glm::ivec2 slideDir)
    {
        if (m_state != State::Wandering && m_state != State::Stunned) return;

        m_state = State::Carried;
        m_isMoving = false;
        m_direction = DeltaToDir(slideDir);
        SetRowFrame(WALK_ROW, 0);
    }

    void SnoBeeComponent::SetCarriedWorldPos(float x, float y)
    {
        GetOwner()->SetLocalPosition(x, y, 0.0f);
    }

    void SnoBeeComponent::Crush(glm::ivec2 slideDir)
    {
        if (m_state == State::Crushed) return;

        m_state = State::Crushed;
        m_direction = DeltaToDir(slideDir);
        m_animTimer = 0.0f;
        SetRowFrame(CRUSH_ROW, 0);
    }

    void SnoBeeComponent::UpdateCrushed(float dt)
    {
        m_animTimer += dt;

        if (m_animTimer >= CRUSH_DURATION)
        {
            GetOwner()->MarkForDelete();
            return;
        }

        SetRowFrame(CRUSH_ROW, static_cast<int>(m_animTimer * 8.0f) % 2);
    }

    void SnoBeeComponent::ChooseNextMove()
    {
        auto& grid = LevelGrid::GetInstance();

        constexpr std::array<SnoBeeDirection, 4> dirs{
            SnoBeeDirection::Down, SnoBeeDirection::Left,
            SnoBeeDirection::Up,   SnoBeeDirection::Right
        };

        const glm::ivec2 reverseDelta = -DirToDelta(m_direction);

        std::array<SnoBeeDirection, 4> walkable{};
        int walkableCount = 0;
        bool reverseWalkable = false;

        for (SnoBeeDirection d : dirs)
        {
            const glm::ivec2 delta = DirToDelta(d);
            const glm::ivec2 next = m_gridPos + delta;
            if (grid.IsPositionBlocked(next.x, next.y))
                continue;

            if (delta == reverseDelta)
            {
                reverseWalkable = true;
                continue;
            }
            walkable[static_cast<size_t>(walkableCount++)] = d;
        }

        SnoBeeDirection chosen = m_direction;
        std::uniform_real_distribution<float> chance(0.0f, 1.0f);

        glm::ivec2 target{};
        const bool hasTarget = FindChaseTarget(target);

        if (hasTarget && m_eatCooldown <= 0.0f && chance(Rng()) < EAT_CHANCE)
        {
            const glm::ivec2 toTarget = target - m_gridPos;
            glm::ivec2 step{ 0, 0 };
            if (std::abs(toTarget.x) >= std::abs(toTarget.y) && toTarget.x != 0)
                step = { toTarget.x > 0 ? 1 : -1, 0 };
            else if (toTarget.y != 0)
                step = { 0, toTarget.y > 0 ? 1 : -1 };

            if (step.x != 0 || step.y != 0)
            {
                const glm::ivec2 cell = m_gridPos + step;
                auto* block = grid.GetBlockAt(cell.x, cell.y);
                if (block && !block->IsSliding()
                    && block->GetBlockType() == BlockType::Normal)
                {
                    m_direction = DeltaToDir(step);
                    m_eatTargetCell = cell;
                    m_eatTimer = 0.0f;
                    m_state = State::Eating;
                    return;
                }
            }
        }

        if (walkableCount > 0)
        {
            if (hasTarget && chance(Rng()) < CHASE_CHANCE)
            {
                int bestDist = 0;
                bool found = false;
                for (int i = 0; i < walkableCount; ++i)
                {
                    const glm::ivec2 next =
                        m_gridPos + DirToDelta(walkable[static_cast<size_t>(i)]);
                    const int dist = std::abs(target.x - next.x) + std::abs(target.y - next.y);
                    if (!found || dist < bestDist)
                    {
                        bestDist = dist;
                        chosen = walkable[static_cast<size_t>(i)];
                        found = true;
                    }
                }
            }
            else
            {
                bool straightOk = false;
                for (int i = 0; i < walkableCount; ++i)
                    if (walkable[static_cast<size_t>(i)] == m_direction) { straightOk = true; break; }

                if (straightOk && chance(Rng()) < 0.7f)
                {
                    chosen = m_direction;
                }
                else
                {
                    std::uniform_int_distribution<int> pick(0, walkableCount - 1);
                    chosen = walkable[static_cast<size_t>(pick(Rng()))];
                }
            }
        }
        else if (reverseWalkable)
        {
            chosen = DeltaToDir(reverseDelta);
        }
        else
        {
            m_decisionCooldown = 0.5f;
            return;
        }

        m_direction = chosen;
        m_targetGridPos = m_gridPos + DirToDelta(chosen);
        m_isMoving = true;
    }

    void SnoBeeComponent::UpdateGridMovement(float dt)
    {
        auto& grid = LevelGrid::GetInstance();

        if (grid.IsPositionBlocked(m_targetGridPos.x, m_targetGridPos.y))
        {
            m_targetGridPos = m_gridPos;
        }

        glm::vec3 currentPos = GetOwner()->GetLocalPosition();
        glm::vec2 targetWorld = grid.GridToWorld(m_targetGridPos.x, m_targetGridPos.y);

        glm::vec2 diff = targetWorld - glm::vec2(currentPos.x, currentPos.y);
        float dist = glm::length(diff);

        if (dist < 1.5f)
        {
            GetOwner()->SetLocalPosition(targetWorld.x, targetWorld.y, 0.0f);
            m_gridPos = m_targetGridPos;
            m_isMoving = false;
            m_decisionCooldown = 0.05f;
            return;
        }

        glm::vec2 dir = glm::normalize(diff);
        glm::vec3 newPos = currentPos;
        newPos.x += dir.x * MOVE_SPEED * dt;
        newPos.y += dir.y * MOVE_SPEED * dt;
        GetOwner()->SetLocalPosition(newPos);
    }

    void SnoBeeComponent::SetRowFrame(int row, int frame)
    {
        if (auto* render = GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture(CurrentTexture());

            SDL_Rect src{};
            src.x = (FrameOffsetForDirection(m_direction) + (frame % 2)) * FRAME_W;
            src.y = SHEET_Y_OFFSET + row * FRAME_H;
            src.w = FRAME_W;
            src.h = FRAME_H;

            render->SetSourceRect(src);
        }
    }

    void SnoBeeComponent::SetAbsFrame(int row, int column)
    {
        if (auto* render = GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture(CurrentTexture());

            SDL_Rect src{};
            src.x = column * FRAME_W;
            src.y = SHEET_Y_OFFSET + row * FRAME_H;
            src.w = FRAME_W;
            src.h = FRAME_H;
            render->SetSourceRect(src);
        }
    }

    SnoBeeDirection SnoBeeComponent::DeltaToDir(glm::ivec2 delta)
    {
        if (delta.y > 0) return SnoBeeDirection::Down;
        if (delta.y < 0) return SnoBeeDirection::Up;
        if (delta.x < 0) return SnoBeeDirection::Left;
        return SnoBeeDirection::Right;
    }

    glm::ivec2 SnoBeeComponent::DirToDelta(SnoBeeDirection dir)
    {
        switch (dir)
        {
        case SnoBeeDirection::Up:    return { 0, -1 };
        case SnoBeeDirection::Down:  return { 0,  1 };
        case SnoBeeDirection::Left:  return { -1, 0 };
        case SnoBeeDirection::Right: return { 1,  0 };
        }
        return { 0, 0 };
    }

    int SnoBeeComponent::FrameOffsetForDirection(SnoBeeDirection dir)
    {
        switch (dir)
        {
        case SnoBeeDirection::Down:  return 0;
        case SnoBeeDirection::Left:  return 2;
        case SnoBeeDirection::Up:    return 4;
        case SnoBeeDirection::Right: return 6;
        }
        return 0;
    }
}