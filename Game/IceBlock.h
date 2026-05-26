#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace dae
{
    enum class BlockType
    {
        Normal,
        Diamond,
        Egg
    };

    class IceBlock : public Component
    {
    public:
        IceBlock(GameObject* owner, int gridX, int gridY, BlockType type = BlockType::Normal);
        ~IceBlock() override;

        void Update(float dt) override;
        void Cleanup(); // call before unloading a level to unregister from the grid

        bool TryPush(glm::ivec2 direction);

        bool IsSliding() const { return m_state == State::Sliding; }
        glm::ivec2 GetGridPos() const { return m_gridPos; }

        BlockType GetBlockType() const { return m_type; }
        bool IsDiamond() const { return m_type == BlockType::Diamond; }

    private:
        enum class State { Idle, Sliding, Dying };

        glm::ivec2 m_gridPos;
        glm::ivec2 m_slideDir{ 0, 0 };
        State m_state = State::Idle;
        BlockType m_type = BlockType::Normal;
        float m_animTimer = 0.0f;

        static constexpr float SLIDE_SPEED = 200.0f;
        static constexpr float DEATH_FPS = 10.0f;
        static constexpr int   DEATH_FRAMES = 9;
        static constexpr int   FRAME_WIDTH = 16;
        static constexpr int   FRAME_HEIGHT = 16;
        static constexpr const char* TEXTURE = "IceBlock_Spritesheet.png";

        void UpdateSlide(float dt);
        void UpdateDying(float dt);
        void SetFrame(int frame);
    };
}