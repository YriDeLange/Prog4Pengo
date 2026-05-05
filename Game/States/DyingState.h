#pragma once
#include "PengoState.h"

class DyingState : public PengoState
{
public:
    explicit DyingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    std::unique_ptr<PengoState> HandleInput(float dt) override;
    void Update(float dt) override;
};