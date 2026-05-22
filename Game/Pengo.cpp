#include "Pengo.h"
#include "States/StandingState.h"
#include "RenderComponent.h"
#include "HealthComponent.h"
#include "GameEvents.h"
#include "States/DyingState.h"
#include "LevelGrid.h"
#include "IceBlock.h"

Pengo::Pengo(dae::GameObject* owner, bool IsPlayer1)
    : m_owner(owner)
    , IsPlayer1(IsPlayer1)
{
    // Sync grid position from the world position set before this component was added
    glm::vec3 worldPos = m_owner->GetLocalPosition();
    m_currentGridPos = dae::LevelGrid::GetInstance().WorldToGrid(worldPos.x, worldPos.y);
    m_targetGridPos = m_currentGridPos;

    m_currentState = std::make_unique<StandingState>(this);
    m_currentState->OnEnter();

    if (auto* health = m_owner->GetComponent<dae::HealthComponent>())
    {
        health->AddObserver([this](unsigned int eventId)
            {
                if (eventId == GameEvent::PlayerDied)
                {
                    if (m_currentState)
                    {
                        m_currentState->OnExit();
                    }
                    m_currentState = std::make_unique<DyingState>(this);
                    m_currentState->OnEnter();
                }
            });
    }
}

Pengo::~Pengo()
{
    if (m_currentState)
    {
        m_currentState->OnExit();
    }
}

void Pengo::HandleInput(float dt)
{
    if (m_currentState)
    {
        auto newState = m_currentState->HandleInput(dt);
        if (newState)
        {
            m_currentState->OnExit();
            m_currentState = std::move(newState);
            m_currentState->OnEnter();
        }
    }
}

void Pengo::Update(float dt)
{
    if (m_currentState)
        m_currentState->Update(dt);

    UpdateGridMovement(dt);
}

// ====================== INPUT METHODS (for Command pattern) ======================
void Pengo::MoveUp()
{
    if (m_isMovingToTarget || m_isPushing || m_movementLocked) return;
    SetDirection(PengoDirection::Up);
    m_inputDirection = { 0.0f, -1.0f };
    StartMovingToGrid(m_currentGridPos.x, m_currentGridPos.y - 1);
}

void Pengo::MoveDown()
{
    if (m_isMovingToTarget || m_isPushing || m_movementLocked) return;
    SetDirection(PengoDirection::Down);
    m_inputDirection = { 0.0f, 1.0f };
    StartMovingToGrid(m_currentGridPos.x, m_currentGridPos.y + 1);
}

void Pengo::MoveLeft()
{
    if (m_isMovingToTarget || m_isPushing || m_movementLocked) return;
    SetDirection(PengoDirection::Left);
    m_inputDirection = { -1.0f, 0.0f };
    StartMovingToGrid(m_currentGridPos.x - 1, m_currentGridPos.y);
}

void Pengo::MoveRight()
{
    if (m_isMovingToTarget || m_isPushing || m_movementLocked) return;
    SetDirection(PengoDirection::Right);
    m_inputDirection = { 1.0f, 0.0f };
    StartMovingToGrid(m_currentGridPos.x + 1, m_currentGridPos.y);
}

void Pengo::TryPush()
{
    if (m_isPushing || m_isMovingToTarget || m_movementLocked) return;

    glm::ivec2 front = m_currentGridPos;
    switch (m_direction)
    {
    case PengoDirection::Up:    front.y -= 1; break;
    case PengoDirection::Down:  front.y += 1; break;
    case PengoDirection::Left:  front.x -= 1; break;
    case PengoDirection::Right: front.x += 1; break;
    }

    auto* block = dae::LevelGrid::GetInstance().GetBlockAt(front.x, front.y);
    if (!block) return;

    glm::ivec2 pushDir = front - m_currentGridPos;
    m_isPushing = block->TryPush(pushDir);
    m_pushedBlock = m_isPushing ? block : nullptr;
}

void Pengo::StopMoving()
{
    m_inputDirection = { 0.0f, 0.0f };
    m_velocity = { 0.0f, 0.0f };
}

void Pengo::LockMovement()
{
    m_movementLocked = true;
    m_isMovingToTarget = false;
    m_inputDirection = { 0.0f, 0.0f };
    m_velocity = { 0.0f, 0.0f };
    // Snap to the committed grid position to avoid being stuck mid-cell
    glm::vec2 snapped = dae::LevelGrid::GetInstance().GridToWorld(m_currentGridPos.x, m_currentGridPos.y);
    m_owner->SetLocalPosition(snapped.x, snapped.y, 0.0f);
}

// ====================== SPRITESHEET ======================
void Pengo::SetDirection(PengoDirection dir)
{
    m_direction = dir;
}

void Pengo::SetSpriteFrame(int frameIndex)
{
    SetSpriteFrame(m_direction, frameIndex);
}

void Pengo::SetSpriteFrame(PengoDirection dir, int frame)
{
    if (auto* render = m_owner->GetComponent<dae::RenderComponent>())
    {
        
        if (IsPlayer1 == true)
        {
            render->SetTexture(SPRITESHEET);
        }
        else
        {
            render->SetTexture(SPRITESHEET2);
        }

        int baseFrame = GetFrameOffsetForDirection(dir);
        int finalFrame = baseFrame + (frame % 2);

        SDL_Rect src{};
        src.x = finalFrame * FRAME_WIDTH;
        src.y = 0;
        src.w = FRAME_WIDTH;
        src.h = FRAME_HEIGHT;

        render->SetSourceRect(src);
    }
}

void Pengo::SetDeathFrame(int frame)
{
    if (auto* render = m_owner->GetComponent<dae::RenderComponent>())
    {
        if (IsPlayer1)
            render->SetTexture(SPRITESHEET);
        else
            render->SetTexture(SPRITESHEET2);

        SDL_Rect src{};
        src.x = (frame % 2) * FRAME_WIDTH;
        src.y = 32;
        src.w = FRAME_WIDTH;
        src.h = FRAME_HEIGHT;

        render->SetSourceRect(src);
    }
}

void Pengo::SetPushFrame(int frame)
{
    if (auto* render = m_owner->GetComponent<dae::RenderComponent>())
    {
        if (IsPlayer1)
            render->SetTexture(SPRITESHEET);
        else
            render->SetTexture(SPRITESHEET2);

        int baseFrame = GetFrameOffsetForDirection(m_direction);

        SDL_Rect src{};
        src.x = (baseFrame + (frame % 2)) * FRAME_WIDTH;
        src.y = 16;
        src.w = FRAME_WIDTH;
        src.h = FRAME_HEIGHT;

        render->SetSourceRect(src);
    }
}

int Pengo::GetFrameOffsetForDirection(PengoDirection dir) const
{
    switch (dir)
    {
    case PengoDirection::Down:  return 0;
    case PengoDirection::Left:  return 2;
    case PengoDirection::Up:    return 4;
    case PengoDirection::Right: return 6;
    default:                    return 0;
    }
}

void Pengo::SetVelocity(const glm::vec2& velocity)
{
    m_velocity = velocity;
}

void Pengo::UpdateGridMovement(float dt)
{
    if (!m_isMovingToTarget)
        return;

    auto& grid = dae::LevelGrid::GetInstance();
    glm::vec3 currentPos = m_owner->GetLocalPosition();
    glm::vec2 targetWorldPos = grid.GridToWorld(m_targetGridPos.x, m_targetGridPos.y);

    glm::vec2 direction = targetWorldPos - glm::vec2(currentPos.x, currentPos.y);
    float distance = glm::length(direction);

    // Close enough? Snap to exact position
    if (distance < 2.0f)
    {
        m_owner->SetLocalPosition(targetWorldPos.x, targetWorldPos.y, 0.0f);
        m_currentGridPos = m_targetGridPos;
        m_isMovingToTarget = false;
        m_velocity = { 0.0f, 0.0f };
        return;
    }

    // Move towards target
    glm::vec2 normalizedDir = glm::normalize(direction);
    m_velocity = normalizedDir * MOVE_SPEED;

    glm::vec3 pos = currentPos;
    pos.x += m_velocity.x * dt;
    pos.y += m_velocity.y * dt;
    m_owner->SetLocalPosition(pos);
}

void Pengo::StartMovingToGrid(int gridX, int gridY)
{
    auto& grid = dae::LevelGrid::GetInstance();

    if (!grid.IsInBounds(gridX, gridY))
        return;

    if (grid.IsPositionBlocked(gridX, gridY))
        return;

    m_targetGridPos = { gridX, gridY };
    m_isMovingToTarget = true;
}
