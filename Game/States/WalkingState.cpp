#include "WalkingState.h"
#include "../Pengo.h"
#include "StandingState.h"
#include "PushingState.h"

void WalkingState::OnEnter()
{
    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), 0);
}

std::unique_ptr<PengoState> WalkingState::HandleInput(float dt)
{
    if (m_pPengo->IsPushing())
        return std::make_unique<PushingState>(m_pPengo);

    const auto& dir = m_pPengo->GetInputDirection();
    if (dir.x == 0.0f && dir.y == 0.0f)
        return std::make_unique<StandingState>(m_pPengo);

    return nullptr;
}

void WalkingState::Update(float dt)
{
    // Animate walking sprite
    static float timer = 0.0f;
    timer += dt;
    int frame = (static_cast<int>(timer * 8.0f) % 2);

    m_pPengo->SetSpriteFrame(m_pPengo->GetDirection(), frame);
}