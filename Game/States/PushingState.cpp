#include "PushingState.h"
#include "../Pengo.h"
#include "StandingState.h"

void PushingState::OnEnter()
{
    m_timer = 0.0f;
    m_pPengo->SetPushFrame(0);
}

void PushingState::OnExit()
{
    m_pPengo->ClearPushState();
}

std::unique_ptr<PengoState> PushingState::HandleInput(float dt)
{
    if (m_timer >= PUSH_DURATION)
        return std::make_unique<StandingState>(m_pPengo);

    return nullptr;
}

void PushingState::Update(float dt)
{
    m_timer += dt;
    int frame = static_cast<int>(m_timer * 8.0f) % 2;
    m_pPengo->SetPushFrame(frame);
}