#include "DyingState.h"
#include "../Pengo.h"

void DyingState::OnEnter()
{
    _pengo->SetSpriteFrame(3);
}

PengoState* DyingState::HandleInput(float dt)
{
    return nullptr;
}

void DyingState::Update(float dt) {}