#pragma once
#include "States/PengoState.h"
#include "GameObject.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

namespace dae
{
    class RenderComponent;
}

enum class PengoDirection
{
    Down = 0,
    Left = 1,
    Up = 2,
    Right = 3
};

class Pengo
{
public:
    explicit Pengo(dae::GameObject* owner);
    ~Pengo();

    void SetState(std::unique_ptr<PengoState> state);

    void HandleInput(float dt);
    void Update(float dt);

    void SetDirection(PengoDirection dir);
    void SetSpriteFrame(int frameIndex);
    void SetSpriteFrame(PengoDirection dir, int frame = 0);

    dae::GameObject* GetOwner() const { return _owner; }

private:
    dae::GameObject* _owner = nullptr;
    std::unique_ptr<PengoState> _currentState;
    PengoDirection _direction = PengoDirection::Down;
    int _currentFrame = 0;
    static constexpr int FRAME_WIDTH = 16;
    static constexpr int FRAME_HEIGHT = 16;
    static constexpr const char* SPRITESHEET = "Pengo_Spritesheet.png";
};