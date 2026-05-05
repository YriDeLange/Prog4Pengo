#pragma once
#include "PengoState.h"

class WalkingState : public PengoState
{
public:
    explicit WalkingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    PengoState* HandleInput(float dt) override;
    void Update(float dt) override;
};