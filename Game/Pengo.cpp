#include "Pengo.h"
#include "States/StandingState.h"
#include "RenderComponent.h"

Pengo::Pengo(dae::GameObject* owner)
    : m_owner(owner)
{
    m_currentState = std::make_unique<StandingState>(this);
    m_currentState->OnEnter();
}

Pengo::~Pengo()
{
    if (m_currentState)
    {
        m_currentState->OnExit();
    }
}

void Pengo::SetState(std::unique_ptr<PengoState> state)
{
    if (m_currentState)
    {
        m_currentState->OnExit();
    }

    m_currentState = std::move(state);

    if (m_currentState)
    {
        m_currentState->OnEnter();
    }
}

void Pengo::HandleInput(float dt)
{
    if (m_currentState)
    {
        auto newState = m_currentState->HandleInput(dt);
        if (newState) SetState(std::move(newState));
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

    m_velocity = { 0.0f, 0.0f };
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
        render->SetTexture(SPRITESHEET);

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
