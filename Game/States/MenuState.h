#pragma once
#include "GameState.h"
#include "GameMode.h"
#include <memory>
#include <array>
#include <SDL3/SDL.h>

namespace dae
{
    class Scene;
    class TextComponent;
}

// Front door of the game. Shows a logo and three options (Single-Player /
// Co-op / Versus), lets the player navigate with keyboard or gamepad, and
// transitions to PlayingState with the chosen mode. Owns its own scene.
class MenuState final : public dae::GameState
{
public:
    MenuState() = default;

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<dae::GameState> Update(float deltaTime) override;
    void Render() const override;

    // Called by the menu input commands.
    void MoveSelection(int delta);
    void Confirm();

private:
    static constexpr int OPTION_COUNT = 3; // Single / Coop / Versus

    int  m_selectedIndex{ 0 };
    bool m_confirmed{ false };

    dae::Scene* m_pScene{ nullptr };
    std::array<dae::TextComponent*, OPTION_COUNT> m_options{};

    static constexpr SDL_Color SELECTED_COLOR{ 255, 216, 0, 255 };   // bright yellow
    static constexpr SDL_Color UNSELECTED_COLOR{ 120, 120, 120, 255 }; // dim grey

    GameMode SelectedMode() const;
    void UpdateHighlight();
    void BindInput();
    void UnbindInput();
};