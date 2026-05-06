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
    PengoDirection GetDirection() const { return m_direction; }
    void SetSpriteFrame(int frameIndex);
    void SetSpriteFrame(PengoDirection dir, int frame = 0);
    int GetFrameOffsetForDirection(PengoDirection dir) const;
    void SetVelocity(const glm::vec2& velocity);
    const glm::vec2& GetVelocity() const { return m_velocity; }

    dae::GameObject* GetOwner() const { return m_owner; }

private:
    dae::GameObject* m_owner = nullptr;
    std::unique_ptr<PengoState> m_currentState;
    PengoDirection m_direction = PengoDirection::Down;
    int m_currentFrame = 0;
    static constexpr int FRAME_WIDTH = 16;
    static constexpr int FRAME_HEIGHT = 16;
    static constexpr const char* SPRITESHEET = "Pengo_Spritesheet.png";
    glm::vec2 m_velocity{ 0.0f, 0.0f };
};