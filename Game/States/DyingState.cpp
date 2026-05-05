#include "DyingState.h"
#include "../Pengo.h"

void DyingState::OnEnter()
{
    _pengo->SetSpriteFrame(3);
}

std::unique_ptr<PengoState> DyingState::HandleInput(float dt)
{
    return nullptr;
}

void DyingState::Update(float dt) {}