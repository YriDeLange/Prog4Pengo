#include "IceBlock.h"
#include "LevelGrid.h"
#include "SnoBeeComponent.h"
#include "SnoBeeRegistry.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "PointsComponent.h"
#include "Scoring.h"
#include "GameEvents.h"
#include <glm/geometric.hpp>
#include <SDL3/SDL.h>

namespace
{
    int CrushPoints(size_t beeCount)
    {
        if (beeCount >= 3) return Scoring::ThreeSnoBeesCrushed;
        if (beeCount == 2) return Scoring::TwoSnoBeesCrushed;
        return Scoring::SnoBeeCrushed;
    }
}

namespace dae
{
    IceBlock::IceBlock(GameObject* owner, int gridX, int gridY, BlockType type)
        : Component(owner)
        , m_gridPos(gridX, gridY)
        , m_type(type)
    {
        auto& grid = LevelGrid::GetInstance();
        grid.RegisterBlock(this, gridX, gridY);

        glm::vec2 worldPos = grid.GridToWorld(gridX, gridY);
        owner->SetLocalPosition(worldPos.x, worldPos.y, 0.0f);

        SetFrame(0);
    }

    IceBlock::~IceBlock()
    {

    }

    void IceBlock::Cleanup()
    {
        LevelGrid::GetInstance().UnregisterBlock(m_gridPos.x, m_gridPos.y);
    }

    void IceBlock::Update(float dt)
    {
        switch (m_state)
        {
        case State::Idle:
            if (m_type == BlockType::Egg)
                UpdateFlash(dt);
            break;
        case State::Sliding:  UpdateSlide(dt);   break;
        case State::Dying:    UpdateDying(dt);   break;
        case State::EggDying:
        case State::Hatching: UpdateEggAnim(dt); break;
        }
    }

    void IceBlock::UpdateFlash(float dt)
    {
        if (m_flashTimer >= FLASH_DURATION)
            return;

        m_flashTimer += dt;

        if (m_flashTimer >= FLASH_DURATION)
        {
            SetFrame(0);
            return;
        }

        const bool showEgg =
            (static_cast<int>(m_flashTimer / FLASH_PERIOD) % 2) == 1;
        if (showEgg) SetEggFrame(EGG_HATCH_ROW, 0);
        else         SetFrame(0);
    }

    bool IceBlock::TryPush(glm::ivec2 direction, GameObject* pusher)
    {
        if (m_state != State::Idle) return false;

        auto& grid = LevelGrid::GetInstance();
        glm::ivec2 next = m_gridPos + direction;

        if (grid.IsPositionBlocked(next.x, next.y))
        {
            if (m_type == BlockType::Diamond)
                return false;

            grid.UnregisterBlock(m_gridPos.x, m_gridPos.y);
            m_animTimer = 0.0f;

            if (m_type == BlockType::Egg)
            {
                m_state = State::EggDying;
                SetEggFrame(EGG_DEATH_ROW, 0);
                if (pusher)
                {
                    if (auto* points = pusher->GetComponent<PointsComponent>())
                        points->AddPoints(Scoring::EggBlockDestroyed, GameEvent::EnemyDied);
                }
            }
            else
            {
                m_state = State::Dying;
                SetFrame(1);
            }
            return true;
        }

        m_slideDir = direction;
        m_state = State::Sliding;
        m_pPusher = pusher;
        m_carriedBees.clear();

        grid.MoveBlock(this, m_gridPos.x, m_gridPos.y, next.x, next.y);
        m_gridPos = next;

        return true;
    }

    void IceBlock::Disintegrate()
    {
        if (m_state != State::Idle) return;
        if (m_type == BlockType::Diamond) return;

        LevelGrid::GetInstance().UnregisterBlock(m_gridPos.x, m_gridPos.y);
        m_animTimer = 0.0f;
        if (m_type == BlockType::Egg)
        {
            m_state = State::EggDying;
            SetEggFrame(EGG_DEATH_ROW, 0);
        }
        else
        {
            m_state = State::Dying;
            SetFrame(1);
        }
    }

    void IceBlock::StartHatching()
    {
        if (!IsIdleEgg()) return;

        LevelGrid::GetInstance().UnregisterBlock(m_gridPos.x, m_gridPos.y);
        m_state = State::Hatching;
        m_animTimer = 0.0f;
        SetEggFrame(EGG_HATCH_ROW, 0);
    }

    void IceBlock::UpdateEggAnim(float dt)
    {
        m_animTimer += dt;

        const int frame = static_cast<int>(m_animTimer * EGG_ANIM_FPS);
        if (frame >= EGG_FRAMES)
        {
            GetOwner()->MarkForDelete();
            return;
        }

        SetEggFrame(m_state == State::Hatching ? EGG_HATCH_ROW : EGG_DEATH_ROW, frame);
    }

    void IceBlock::UpdateSlide(float dt)
    {
        auto& grid = LevelGrid::GetInstance();
        glm::vec2 targetWorld = grid.GridToWorld(m_gridPos.x, m_gridPos.y);
        glm::vec3 currentPos = GetOwner()->GetLocalPosition();

        CaptureSnoBeesInPath();

        glm::vec2 diff = targetWorld - glm::vec2(currentPos.x, currentPos.y);
        float dist = glm::length(diff);

        if (dist < 2.0f)
        {
            GetOwner()->SetLocalPosition(targetWorld.x, targetWorld.y, 0.0f);
            DragCarriedBees();

            glm::ivec2 next = m_gridPos + m_slideDir;
            if (!grid.IsPositionBlocked(next.x, next.y))
            {
                grid.MoveBlock(this, m_gridPos.x, m_gridPos.y, next.x, next.y);
                m_gridPos = next;
            }
            else
            {
                m_state = State::Idle;
                SquashCarriedBees();
                m_pPusher = nullptr;
                if (m_type == BlockType::Egg)
                    SetFrame(0);
            }
            return;
        }

        glm::vec2 dir = glm::normalize(diff);
        glm::vec3 newPos = currentPos;
        newPos.x += dir.x * SLIDE_SPEED * dt;
        newPos.y += dir.y * SLIDE_SPEED * dt;
        GetOwner()->SetLocalPosition(newPos);

        DragCarriedBees();
    }

    void IceBlock::CaptureSnoBeesInPath()
    {
        const glm::vec3 myPos = GetOwner()->GetLocalPosition();

        for (SnoBeeComponent* bee : SnoBeeRegistry::GetInstance().GetAll())
        {
            if (!bee->IsCapturable()) continue;

            const glm::vec3 beePos = bee->GetOwner()->GetLocalPosition();
            const glm::vec2 diff{ beePos.x - myPos.x, beePos.y - myPos.y };
            if (glm::length(diff) < CAPTURE_RADIUS)
            {
                bee->StartCarried(m_slideDir);
                m_carriedBees.push_back(bee);
            }
        }
    }

    void IceBlock::DragCarriedBees()
    {
        const glm::vec3 myPos = GetOwner()->GetLocalPosition();
        const float bx = myPos.x + static_cast<float>(m_slideDir.x) * static_cast<float>(FRAME_WIDTH);
        const float by = myPos.y + static_cast<float>(m_slideDir.y) * static_cast<float>(FRAME_HEIGHT);

        for (SnoBeeComponent* bee : m_carriedBees)
            bee->SetCarriedWorldPos(bx, by);
    }

    void IceBlock::SquashCarriedBees()
    {
        if (m_carriedBees.empty()) return;

        for (SnoBeeComponent* bee : m_carriedBees)
            bee->Crush(m_slideDir);

        if (m_pPusher)
        {
            if (auto* points = m_pPusher->GetComponent<PointsComponent>())
                points->AddPoints(CrushPoints(m_carriedBees.size()), GameEvent::EnemyDied);
        }

        m_carriedBees.clear();
    }

    void IceBlock::UpdateDying(float dt)
    {
        m_animTimer += dt;

        int frame = 1 + static_cast<int>(m_animTimer * DEATH_FPS);

        if (frame >= 1 + DEATH_FRAMES)
        {
            GetOwner()->MarkForDelete();
            return;
        }

        SetFrame(frame);
    }

    void IceBlock::SetFrame(int frame)
    {
        if (auto* render = GetOwner()->GetComponent<RenderComponent>())
        {
            if (m_type == BlockType::Diamond)
            {
                render->SetTexture(DIAMOND_TEXTURE);
                SDL_Rect src{ 0, 0, FRAME_WIDTH, FRAME_HEIGHT };
                render->SetSourceRect(src);
                return;
            }

            render->SetTexture(TEXTURE);

            SDL_Rect src{};
            src.x = frame * FRAME_WIDTH;
            src.y = 0;
            src.w = FRAME_WIDTH;
            src.h = FRAME_HEIGHT;
            render->SetSourceRect(src);
        }
    }

    void IceBlock::SetEggFrame(int row, int frame)
    {
        if (auto* render = GetOwner()->GetComponent<RenderComponent>())
        {
            render->SetTexture(EGG_TEXTURE);

            SDL_Rect src{};
            src.x = frame * FRAME_WIDTH;
            src.y = row * FRAME_HEIGHT;
            src.w = FRAME_WIDTH;
            src.h = FRAME_HEIGHT;
            render->SetSourceRect(src);
        }
    }
}