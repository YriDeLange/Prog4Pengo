#include "Pengo.h"
#include "States/StandingState.h"
#include "RenderComponent.h"
#include "HealthComponent.h"
#include "GameEvents.h"
#include "States/DyingState.h"

Pengo::Pengo(dae::GameObject* owner, bool IsPlayer1)
    : m_owner(owner)
    , IsPlayer1(IsPlayer1)
{
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

    if (m_velocity.x != 0.0f || m_velocity.y != 0.0f)
    {
        glm::vec3 pos = m_owner->GetLocalPosition();
        pos.x += m_velocity.x * dt;
        pos.y += m_velocity.y * dt;
        m_owner->SetLocalPosition(pos);
    }

    m_velocity = m_inputDirection * MOVE_SPEED;
}

// ====================== INPUT METHODS (for Command pattern) ======================
void Pengo::MoveUp()
{
    m_inputDirection = { 0.0f, -1.0f };
    SetDirection(PengoDirection::Up);
}

void Pengo::MoveDown()
{
    m_inputDirection = { 0.0f, 1.0f };
    SetDirection(PengoDirection::Down);
}

void Pengo::MoveLeft()
{
    m_inputDirection = { -1.0f, 0.0f };
    SetDirection(PengoDirection::Left);
}

void Pengo::MoveRight()
{
    m_inputDirection = { 1.0f, 0.0f };
    SetDirection(PengoDirection::Right);
}

void Pengo::StopMoving()
{
    m_inputDirection = { 0.0f, 0.0f };
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