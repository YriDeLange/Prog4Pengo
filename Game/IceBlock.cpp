#include "IceBlock.h"
#include "LevelGrid.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include <glm/geometric.hpp>
#include <SDL3/SDL.h>

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
        case State::Sliding: UpdateSlide(dt); break;
        case State::Dying:   UpdateDying(dt); break;
        default: break;
        }
    }

    bool IceBlock::TryPush(glm::ivec2 direction)
    {
        if (m_state != State::Idle) return false;

        // Diamond blocks are indestructible and immovable.
        if (m_type == BlockType::Diamond) return false;

        auto& grid = LevelGrid::GetInstance();
        glm::ivec2 next = m_gridPos + direction;

        if (grid.IsPositionBlocked(next.x, next.y))
        {
            grid.UnregisterBlock(m_gridPos.x, m_gridPos.y);
            m_state = State::Dying;
            m_animTimer = 0.0f;
            SetFrame(1);
            return true;
        }

        m_slideDir = direction;
        m_state = State::Sliding;

        grid.MoveBlock(this, m_gridPos.x, m_gridPos.y, next.x, next.y);
        m_gridPos = next;

        return true;
    }

    void IceBlock::UpdateSlide(float dt)
    {
        auto& grid = LevelGrid::GetInstance();
        glm::vec2 targetWorld = grid.GridToWorld(m_gridPos.x, m_gridPos.y);
        glm::vec3 currentPos = GetOwner()->GetLocalPosition();

        glm::vec2 diff = targetWorld - glm::vec2(currentPos.x, currentPos.y);
        float dist = glm::length(diff);

        if (dist < 2.0f)
        {
            GetOwner()->SetLocalPosition(targetWorld.x, targetWorld.y, 0.0f);

            glm::ivec2 next = m_gridPos + m_slideDir;
            if (!grid.IsPositionBlocked(next.x, next.y))
            {
                grid.MoveBlock(this, m_gridPos.x, m_gridPos.y, next.x, next.y);
                m_gridPos = next;
            }
            else
            {
                m_state = State::Idle;
            }
            return;
        }

        glm::vec2 dir = glm::normalize(diff);
        glm::vec3 newPos = currentPos;
        newPos.x += dir.x * SLIDE_SPEED * dt;
        newPos.y += dir.y * SLIDE_SPEED * dt;
        GetOwner()->SetLocalPosition(newPos);
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
            render->SetTexture(TEXTURE);

            SDL_Rect src{};
            src.x = frame * FRAME_WIDTH;
            src.y = 0;
            src.w = FRAME_WIDTH;
            src.h = FRAME_HEIGHT;

            render->SetSourceRect(src);
        }
    }
}