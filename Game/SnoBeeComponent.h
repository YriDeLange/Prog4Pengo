#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace dae
{
    // Direction order matches the spritesheet column layout (and Pengo's).
    enum class SnoBeeDirection
    {
        Down = 0,
        Left = 1,
        Up = 2,
        Right = 3
    };

    class SnoBeeComponent final : public Component
    {
    public:
        enum class State { Spawning, Wandering, Eating, Stunned, Carried, Crushed };

        SnoBeeComponent(GameObject* pOwner, int gridX, int gridY, bool playSpawnAnim = true);
        ~SnoBeeComponent() override;

        void Update(float deltaTime) override;

        glm::ivec2 GetGridPos() const { return m_gridPos; }
        State GetState() const { return m_state; }
        bool IsWandering() const { return m_state == State::Wandering; }
        bool IsStunned() const { return m_state == State::Stunned; }
        bool IsCapturable() const
        {
            return m_state == State::Wandering || m_state == State::Stunned
                || m_state == State::Eating;
        }

        void Stun(float duration);

        void SetPlayerControlled(bool controlled);
        bool IsPlayerControlled() const { return m_playerControlled; }
        void TryMove(SnoBeeDirection dir);

        void ResetToSpawn();

        void StartCarried(glm::ivec2 slideDir);
        void SetCarriedWorldPos(float x, float y);
        void Crush(glm::ivec2 slideDir);

    private:
        glm::ivec2 m_spawnGridPos;
        glm::ivec2 m_gridPos;
        glm::ivec2 m_targetGridPos;
        SnoBeeDirection m_direction{ SnoBeeDirection::Down };
        State m_state{ State::Wandering };
        bool  m_isMoving{ false };
        float m_animTimer{ 0.0f };
        float m_decisionCooldown{ 0.0f };
        float m_stunTimer{ 0.0f };
        float m_eatTimer{ 0.0f };
        float m_eatCooldown{ 0.0f };
        glm::ivec2 m_eatTargetCell{ 0, 0 };
        bool  m_playerControlled{ false };

        static constexpr float MOVE_SPEED = 60.0f;
        static constexpr float CHASE_CHANCE = 0.6f;
        static constexpr float EAT_CHANCE = 0.8f;
        static constexpr float EAT_DURATION = 0.8f;
        static constexpr float EAT_COOLDOWN = 2.5f;
        static constexpr float CONTACT_RADIUS = 12.0f;
        static constexpr float CRUSH_DURATION = 0.6f;
        static constexpr int   FRAME_W = 16;
        static constexpr int   FRAME_H = 16;
        static constexpr int   SHEET_Y_OFFSET = 0;
        static constexpr int   SPAWN_ROW = 0;
        static constexpr int   SPAWN_FRAMES = 6;
        static constexpr int   STUN_FRAME0 = 6;
        static constexpr float STUN_ANIM_FPS = 4.0f;
        static constexpr float SPAWN_DURATION = 0.9f;
        static constexpr int   WALK_ROW = 1;
        static constexpr int   CRUSH_ROW = 4;
        static constexpr const char* TEXTURE = "Sno-Bee_Spritesheet.png";
        static constexpr const char* PLAYER_TEXTURE = "Sno-BeePlayer_Spritesheet.png";
        const char* CurrentTexture() const { return m_playerControlled ? PLAYER_TEXTURE : TEXTURE; }

        void UpdateSpawning(float dt);
        void UpdateStunned(float dt);
        void UpdateEating(float dt);
        bool FindChaseTarget(glm::ivec2& outGridPos) const;
        void CheckPengoContact();
        void ChooseNextMove();
        void UpdateGridMovement(float dt);
        void UpdateCrushed(float dt);
        void SetRowFrame(int row, int frame);
        void SetAbsFrame(int row, int column);
        static SnoBeeDirection DeltaToDir(glm::ivec2 delta);
        static glm::ivec2 DirToDelta(SnoBeeDirection dir);
        static int FrameOffsetForDirection(SnoBeeDirection dir);
    };
}