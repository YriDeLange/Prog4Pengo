#pragma once
#include "PengoState.h"
#include "RenderComponent.h"

class DyingState : public PengoState
{
public:
    explicit DyingState(Pengo* pengo) : PengoState(pengo) {}

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<PengoState> HandleInput(float dt) override;
    void Update(float dt) override;
private:
	float m_timer = 0.0f;
};