#pragma once
#include "PengoState.h"

class StandingState : public PengoState
{
public:
    explicit StandingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    std::unique_ptr<PengoState> HandleInput(float dt) override;
    void Update(float dt) override;
};