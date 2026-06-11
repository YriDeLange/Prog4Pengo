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

class MenuState final : public dae::GameState
{
public:
    MenuState() = default;

    void OnEnter() override;
    void OnExit() override;
    std::unique_ptr<dae::GameState> Update(float deltaTime) override;
    void Render() const override;

    void MoveSelection(int delta);
    void Confirm();

private:
    static constexpr int OPTION_COUNT = 3;

    int  m_selectedIndex{ 0 };
    bool m_confirmed{ false };

    dae::Scene* m_pScene{ nullptr };
    std::array<dae::TextComponent*, OPTION_COUNT> m_options{};

    static constexpr SDL_Color SELECTED_COLOR{ 255, 216, 0, 255 };
    static constexpr SDL_Color UNSELECTED_COLOR{ 120, 120, 120, 255 };

    GameMode SelectedMode() const;
    void UpdateHighlight();
    void BindInput();
    void UnbindInput();
};