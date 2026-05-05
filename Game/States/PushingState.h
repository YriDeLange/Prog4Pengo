#pragma once
#include "PengoState.h"

class PushingState : public PengoState
{
public:
    explicit PushingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    std::unique_ptr<PengoState> HandleInput(float dt) override;
    void Update(float dt) override;
};