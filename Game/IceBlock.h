#pragma once
#include "Component.h"
#include <glm/glm.hpp>
#include <vector>

namespace dae
{
    class SnoBeeComponent;

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
        void Cleanup();

        bool TryPush(glm::ivec2 direction, GameObject* pusher = nullptr);

        void StartHatching();

        void Disintegrate();
        static constexpr float HATCH_DURATION = 0.9f;

        bool IsSliding() const { return m_state == State::Sliding; }
        bool IsIdleEgg() const { return m_type == BlockType::Egg && m_state == State::Idle; }
        glm::ivec2 GetGridPos() const { return m_gridPos; }

        BlockType GetBlockType() const { return m_type; }
        bool IsDiamond() const { return m_type == BlockType::Diamond; }

    private:
        enum class State { Idle, Sliding, Dying, EggDying, Hatching };

        glm::ivec2 m_gridPos;
        glm::ivec2 m_slideDir{ 0, 0 };
        State m_state = State::Idle;
        BlockType m_type = BlockType::Normal;
        float m_animTimer = 0.0f;
        float m_flashTimer = 0.0f;

        GameObject* m_pPusher{ nullptr };
        std::vector<SnoBeeComponent*> m_carriedBees;

        static constexpr float SLIDE_SPEED = 200.0f;
        static constexpr float CAPTURE_RADIUS = 14.0f;
        static constexpr float DEATH_FPS = 10.0f;
        static constexpr int   DEATH_FRAMES = 9;
        static constexpr float FLASH_DURATION = 3.0f;
        static constexpr float FLASH_PERIOD = 0.25f;
        static constexpr float EGG_ANIM_FPS = 3.34f;
        static constexpr int   EGG_FRAMES = 3;
        static constexpr int   EGG_HATCH_ROW = 0;
        static constexpr int   EGG_DEATH_ROW = 1;
        static constexpr int   FRAME_WIDTH = 16;
        static constexpr int   FRAME_HEIGHT = 16;
        static constexpr const char* TEXTURE = "IceBlock_Spritesheet.png";
        static constexpr const char* DIAMOND_TEXTURE = "DiamondBlock.png";
        static constexpr const char* EGG_TEXTURE = "Egg_Spritesheet.png";

        void UpdateSlide(float dt);
        void UpdateDying(float dt);
        void UpdateEggAnim(float dt);
        void UpdateFlash(float dt);
        void CaptureSnoBeesInPath();
        void DragCarriedBees();
        void SquashCarriedBees();
        void SetFrame(int frame);
        void SetEggFrame(int row, int frame);
    };
}