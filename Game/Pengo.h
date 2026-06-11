#pragma once
#include "States/PengoState.h"
#include "GameObject.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

namespace dae
{
    class RenderComponent;
}

namespace dae { class IceBlock; }

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
    explicit Pengo(dae::GameObject* owner, bool IsPlayer1);
    ~Pengo();

    void HandleInput(float dt);
    void Update(float dt);

    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();
    void StopMoving();
    void TryPush();

    void SetDirection(PengoDirection dir);
    PengoDirection GetDirection() const { return m_direction; }
    void SetSpriteFrame(int frameIndex);
    void SetSpriteFrame(PengoDirection dir, int frame = 0);
    void SetDeathFrame(int frame);
    void SetPushFrame(int frame);
    int GetFrameOffsetForDirection(PengoDirection dir) const;
    void SetVelocity(const glm::vec2& velocity);
    const glm::vec2& GetVelocity() const { return m_velocity; }
    const glm::vec2& GetInputDirection() const { return m_inputDirection; }

    bool IsPushing() const { return m_isPushing; }
    void ClearPushState() { m_isPushing = false; m_pushedBlock = nullptr; }

    void LockMovement();
    void UnlockMovement() { m_movementLocked = false; }

    bool IsDying() const { return m_movementLocked; }
    void ResetToSpawn();

    dae::GameObject* GetOwner() const { return m_owner; }

private:
    dae::GameObject* m_owner = nullptr;
    std::unique_ptr<PengoState> m_currentState;
    PengoDirection m_direction = PengoDirection::Down;
    static constexpr int FRAME_WIDTH = 16;
    static constexpr int FRAME_HEIGHT = 16;
    static constexpr const char* SPRITESHEET = "Pengo_Spritesheet.png";
    static constexpr const char* SPRITESHEET2 = "Pengo2_Spritesheet.png";
    glm::vec2 m_velocity{ 0.0f, 0.0f };
    glm::vec2 m_inputDirection{ 0.0f, 0.0f };
    static constexpr float MOVE_SPEED = 150.0f;
    static constexpr float STUN_DURATION = 4.5f;
    bool IsPlayer1{};

    glm::ivec2 m_currentGridPos{ 0, 0 };
    glm::ivec2 m_targetGridPos{ 0, 0 };
    glm::ivec2 m_spawnGridPos{ 0, 0 };
    bool m_isMovingToTarget = false;

    bool m_isPushing = false;
    dae::IceBlock* m_pushedBlock = nullptr;
    bool m_movementLocked = false;

    void UpdateGridMovement(float dt);
    void StartMovingToGrid(int gridX, int gridY);
};