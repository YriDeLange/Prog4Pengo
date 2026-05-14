#include "StandingState.h"
#include "../Pengo.h"
#include "WalkingState.h"

void StandingState::OnEnter()
{
    m_pPengo->SetVelocity({ 0.0f, 0.0f });
    m_pPengo->StopMoving();
    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), 0);
}

std::unique_ptr<PengoState> StandingState::HandleInput(float dt)
{
    // Check if Pengo is receiving movement input (via commands)
    if (m_pPengo->GetVelocity().x != 0.0f || m_pPengo->GetVelocity().y != 0.0f)
    {
        return std::make_unique<WalkingState>(m_pPengo);
    }

    return nullptr;
}

void StandingState::Update(float dt)
{
    
}