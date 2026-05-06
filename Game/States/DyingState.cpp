#include "DyingState.h"
#include "../Pengo.h"

void DyingState::OnEnter()
{
    m_pPengo->SetSpriteFrame(3);
}

std::unique_ptr<PengoState> DyingState::HandleInput(float dt)
{
    return nullptr;
}

void DyingState::Update(float dt) {}