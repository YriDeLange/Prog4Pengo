#pragma once
#include "States/PengoState.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "HealthComponent.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Pengo
{
private:
    dae::GameObject* _owner = nullptr;
    PengoState* _currentState = nullptr;

public:
    explicit Pengo(dae::GameObject* owner);
    ~Pengo();

    void SetState(PengoState* newState);

    void HandleInput(float dt);
    void Update(float dt);

    void SetSpriteFrame(int frameIndex);

    dae::GameObject* GetOwner() const { return _owner; }

private:
    static constexpr int FRAME_WIDTH = 32;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr const char* SPRITESHEET = "Pengo_Spritesheet.png";
};