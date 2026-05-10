#include "StandingState.h"
#include "../Pengo.h"
#include "WalkingState.h"

void StandingState::OnEnter()
{
    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), 0);
}

std::unique_ptr<PengoState> StandingState::HandleInput(float dt)
{
    // Check if Pengo is receiving movement input (via commands)
    if (m_pPengo->GetVelocity().x != 0.0f || m_pPengo->GetVelocity().y != 0.0f)
    {
        return std::make_unique<WalkingState>(m_pPengo);
    }

    return nullptr; // Stay in standing state
}

void StandingState::Update(float dt)
{
    // Nothing to do while standing
}