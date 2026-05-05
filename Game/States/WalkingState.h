#pragma once
#include "PengoState.h"

class WalkingState : public PengoState
{
public:
    explicit WalkingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    std::unique_ptr<PengoState> HandleInput(float dt) override;
    void Update(float dt) override;
};