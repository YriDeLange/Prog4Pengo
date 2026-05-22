#pragma once
#include "PengoState.h"

class PushingState : public PengoState
{
public:
    explicit PushingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<PengoState> HandleInput(float dt) override;
    void Update(float dt) override;

private:
    float m_timer = 0.0f;
    static constexpr float PUSH_DURATION = 0.2f;
};