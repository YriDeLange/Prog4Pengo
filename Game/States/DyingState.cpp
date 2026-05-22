#include "DyingState.h"
#include "../Pengo.h"
#include "StandingState.h"

void DyingState::OnEnter()
{
    m_pPengo->LockMovement();
    m_timer = 0.0f;
    m_pPengo->SetDeathFrame(0);
}

void DyingState::OnExit()
{
    m_pPengo->UnlockMovement();
}

std::unique_ptr<PengoState> DyingState::HandleInput(float dt)
{
    if (m_timer >= 3.0f)
        return std::make_unique<StandingState>(m_pPengo);

    return nullptr;
}

void DyingState::Update(float dt)
{
    m_timer += dt;

    int frame = (static_cast<int>(m_timer * 4.0f) % 2);

    m_pPengo->SetDeathFrame(frame);
}