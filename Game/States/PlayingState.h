#pragma once
#include "GameState.h"
#include "GameMode.h"
#include "InputManager.h"   // for dae::KeyState
#include "Gamepad.h"
#include <memory>
#include <vector>
#include <utility>
#include <tuple>
#include <SDL3/SDL.h>

namespace dae
{
    class Scene;
    class GameObject;
}

// The actual gameplay state. Built with the mode chosen in the menu; OnEnter
// loads the level file, builds the HUD, spawns players according to the mode,
// and binds gameplay input. OnExit unbinds everything it bound and destroys
// its scene, so no command outlives the GameObject it points at.
class PlayingState final : public dae::GameState
{
public:
    explicit PlayingState(GameMode mode) : m_mode(mode) {}

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<dae::GameState> Update(float deltaTime) override;
    void Render() const override;

private:
    GameMode    m_mode;
    dae::Scene* m_pScene{ nullptr };

    // Spawns a fully-wired Pengo (health, points, render) plus its HUD label,
    // lives display and score display. isPlayer1 selects the sprite sheet.
    // Returns the raw GameObject* for input binding.
    dae::GameObject* SpawnPlayer(int gridX, int gridY, bool isPlayer1,
        const char* labelText, float labelX, float livesX, float scoreX);

    // Input binding helpers. Each records what it bound so OnExit can undo it.
    void BindKeyboardForPlayer(dae::GameObject* player);
    void BindGamepadForPlayer(unsigned int controllerIdx, dae::GameObject* player);
    void UnbindAll();

    // Track exactly what we bound, to unbind precisely in OnExit.
    std::vector<std::pair<SDL_Scancode, dae::KeyState>> m_boundKeys;
    std::vector<std::tuple<unsigned int, dae::Gamepad::Button, dae::KeyState>> m_boundButtons;
};