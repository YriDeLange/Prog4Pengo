#pragma once
#include "GameState.h"
#include "GameMode.h"
#include "InputManager.h"
#include "Gamepad.h"
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <utility>
#include <tuple>
#include <SDL3/SDL.h>

namespace dae
{
    class Scene;
    class GameObject;
    class SnoBeeCounterComponent;
    class SnoBeeComponent;
}

#include "GameOverState.h"

struct PlayerCarry
{
    int lives{ 4 };
    int points{ 0 };
};

class PlayingState final : public dae::GameState
{
public:
    static constexpr int FIRST_LEVEL = 1;
    static constexpr int LAST_LEVEL = 3;

    explicit PlayingState(GameMode mode,
        int levelIndex = FIRST_LEVEL,
        std::array<PlayerCarry, 2> carry = {})
        : m_mode(mode), m_levelIndex(levelIndex), m_carry(carry) {}

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<dae::GameState> Update(float deltaTime) override;
    void Render() const override;

private:
    GameMode    m_mode;
    int         m_levelIndex;
    std::array<PlayerCarry, 2> m_carry;

    dae::Scene* m_pScene{ nullptr };

    dae::SnoBeeCounterComponent* m_pBeeCounter{ nullptr };
    int   m_initialBeeCount{ 0 };
    float m_hatchCooldown{ 0.f };
    struct PendingHatch { glm::ivec2 gridPos; float timer; };
    std::vector<PendingHatch> m_pendingHatches;

    dae::SnoBeeComponent* m_pControlledBee{ nullptr };
    glm::vec2 m_lastControlledPos{ 0.f, 0.f };
    void UpdatePossession();
    void BindSnoBeeGamepad(unsigned int controllerIdx);

    bool  m_beesSpawned{ false };
    bool  m_skipRequested{ false };
    float m_roundTimer{ 0.f };
    float m_winTimer{ -1.f };
    float m_loseTimer{ -1.f };

    static constexpr float WIN_DELAY = 1.5f;
    static constexpr float LOSE_DELAY = 3.f;

    dae::GameObject* SpawnPlayer(int gridX, int gridY, bool isPlayer1,
        const PlayerCarry& carry,
        const char* labelText, float labelX, float livesX, float scoreX);

    std::unique_ptr<dae::GameState> NextLevel();
    std::array<PlayerCarry, 2> HarvestCarry() const;
    std::vector<PlayerResult> BuildResults() const;

    int  CountIdleEggs() const;
    void UpdateEggs(float deltaTime);

    bool m_diamondBonusAwarded{ false };
    void CheckDiamondAlignment();

    void BindKeyboardForPlayer(dae::GameObject* player);
    void BindGamepadForPlayer(unsigned int controllerIdx, dae::GameObject* player);
    void BindLevelHotkeys();
    void UnbindAll();

    std::vector<std::pair<SDL_Scancode, dae::KeyState>> m_boundKeys;
    std::vector<std::tuple<unsigned int, dae::Gamepad::Button, dae::KeyState>> m_boundButtons;
};