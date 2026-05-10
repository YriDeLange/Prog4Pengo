#include "WalkingState.h"
#include "../Pengo.h"
#include "StandingState.h"

void WalkingState::OnEnter()
{
    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), 0);
}

std::unique_ptr<PengoState> WalkingState::HandleInput(float dt)
{
    // Check if Pengo stopped moving (no input from commands)
    if (m_pPengo->GetVelocity().x == 0.0f && m_pPengo->GetVelocity().y == 0.0f)
    {
        return std::make_unique<StandingState>(m_pPengo);
    }

    return nullptr; // Stay in walking state
}

void WalkingState::Update(float dt)
{
    // Animate walking sprite
    static float timer = 0.0f;
    timer += dt;
    int frame = (static_cast<int>(timer * 8.0f) % 2);

    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), frame);
}