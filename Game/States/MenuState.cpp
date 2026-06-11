#include "MenuState.h"
#include "../MenuCommands.h"
#include "PlayingState.h"

#include "SceneManager.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "InputManager.h"
#include "Gamepad.h"

#include <glm/glm.hpp>

namespace
{
    constexpr float LOGO_X = 32.f;
    constexpr float LOGO_Y = 40.f;

    constexpr float OPTION_X = 32.f;
    constexpr float OPTION_Y0 = 160.f;
    constexpr float OPTION_STEP = 28.f;

    const char* OPTION_LABELS[3] = { "SINGLE PLAYER", "CO-OP", "VERSUS" };
}

void MenuState::OnEnter()
{
    m_selectedIndex = 0;
    m_confirmed = false;

    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    m_pScene = &scene;
    auto& rm = dae::ResourceManager::GetInstance();

    // --- Logo ---
    {
        auto logo = std::make_unique<dae::GameObject>();
        logo->SetLocalPosition(LOGO_X, LOGO_Y);
        auto* r = logo->AddComponent<dae::RenderComponent>();
        r->SetTexture("Logo.png");
        scene.Add(std::move(logo));
    }

    // --- Options ---
    auto font = rm.LoadFont("PressStart2P-Regular.ttf", 10);
    for (int i = 0; i < OPTION_COUNT; ++i)
    {
        auto opt = std::make_unique<dae::GameObject>();
        opt->SetLocalPosition(OPTION_X, OPTION_Y0 + static_cast<float>(i) * OPTION_STEP);
        opt->AddComponent<dae::RenderComponent>();
        auto* text = opt->AddComponent<dae::TextComponent>(OPTION_LABELS[i], font);
        m_options[i] = text;
        scene.Add(std::move(opt));
    }

    UpdateHighlight();
    BindInput();
}

void MenuState::OnExit()
{
    UnbindInput();

    if (m_pScene)
    {
        dae::SceneManager::GetInstance().RemoveScene(*m_pScene);
        m_pScene = nullptr;
    }
    m_options = {};
}

std::unique_ptr<dae::GameState> MenuState::Update(float /*deltaTime*/)
{
    if (m_confirmed)
        return std::make_unique<PlayingState>(SelectedMode());

    return nullptr;
}

void MenuState::Render() const
{
    // Scene rendering is driven by SceneManager via the Renderer.
}

void MenuState::MoveSelection(int delta)
{
    m_selectedIndex = (m_selectedIndex + delta + OPTION_COUNT) % OPTION_COUNT;
    UpdateHighlight();
}

void MenuState::Confirm()
{
    m_confirmed = true;
}

void MenuState::UpdateHighlight()
{
    for (int i = 0; i < OPTION_COUNT; ++i)
    {
        if (!m_options[i]) continue;
        m_options[i]->SetColor(i == m_selectedIndex ? SELECTED_COLOR : UNSELECTED_COLOR);
    }
}

GameMode MenuState::SelectedMode() const
{
    switch (m_selectedIndex)
    {
    case 1:  return GameMode::Coop;
    case 2:  return GameMode::Versus;
    default: return GameMode::SinglePlayer;
    }
}

void MenuState::BindInput()
{
    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Down,
        std::make_unique<dae::MenuNavigateCommand>(this, -1));
    input.BindKeyboardCommand(SDL_SCANCODE_UP, dae::KeyState::Down,
        std::make_unique<dae::MenuNavigateCommand>(this, -1));
    input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Down,
        std::make_unique<dae::MenuNavigateCommand>(this, +1));
    input.BindKeyboardCommand(SDL_SCANCODE_DOWN, dae::KeyState::Down,
        std::make_unique<dae::MenuNavigateCommand>(this, +1));
    input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::KeyState::Down,
        std::make_unique<dae::MenuConfirmCommand>(this));
    input.BindKeyboardCommand(SDL_SCANCODE_SPACE, dae::KeyState::Down,
        std::make_unique<dae::MenuConfirmCommand>(this));

    input.BindControllerCommand(0, dae::Gamepad::Button::DpadUp, dae::KeyState::Down,
        std::make_unique<dae::MenuNavigateCommand>(this, -1));
    input.BindControllerCommand(0, dae::Gamepad::Button::DpadDown, dae::KeyState::Down,
        std::make_unique<dae::MenuNavigateCommand>(this, +1));
    input.BindControllerCommand(0, dae::Gamepad::Button::A, dae::KeyState::Down,
        std::make_unique<dae::MenuConfirmCommand>(this));
}

void MenuState::UnbindInput()
{
    auto& input = dae::InputManager::GetInstance();

    input.UnbindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_UP, dae::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_DOWN, dae::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_RETURN, dae::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_SPACE, dae::KeyState::Down);

    input.UnbindControllerCommand(0, dae::Gamepad::Button::DpadUp, dae::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Gamepad::Button::DpadDown, dae::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Gamepad::Button::A, dae::KeyState::Down);
}