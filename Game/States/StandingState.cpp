#include "StandingState.h"
#include "../Pengo.h"
#include "WalkingState.h"
#include "PushingState.h"

void StandingState::OnEnter()
{
    m_pPengo->SetVelocity({ 0.0f, 0.0f });
    m_pPengo->StopMoving();
    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), 0);
}

std::unique_ptr<PengoState> StandingState::HandleInput(float dt)
{
    if (m_pPengo->IsPushing())
        return std::make_unique<PushingState>(m_pPengo);

    const auto& dir = m_pPengo->GetInputDirection();
    if (dir.x != 0.0f || dir.y != 0.0f)
        return std::make_unique<WalkingState>(m_pPengo);

    return nullptr;
}

void StandingState::Update(float dt)
{
    
}