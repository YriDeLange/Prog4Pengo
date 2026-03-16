#include "InputManager.h"
#include <backends/imgui_impl_sdl3.h>
#include <cstring>

bool dae::InputManager::ProcessInput(float deltaTime)
{
    if (m_pKeyboardState)
        memcpy(m_PreviousKeyboardState, m_pKeyboardState, SDL_SCANCODE_COUNT * sizeof(bool));

    m_pKeyboardState = SDL_GetKeyboardState(nullptr);

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_EVENT_QUIT) return false;
        if (e.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            for (int i = 0; i < MAX_CONTROLLERS; ++i)
                m_Gamepads[i].OnGamepadAdded();
        }
        ImGui_ImplSDL3_ProcessEvent(&e);
    }

    for (auto& [binding, command] : m_KeyboardCommands)
    {
        auto [scancode, state] = binding;
        bool current = m_pKeyboardState[scancode];
        bool previous = m_PreviousKeyboardState[scancode];

        bool trigger = false;
        switch (state)
        {
        case KeyState::Down:    trigger = current && !previous;  break;
        case KeyState::Up:      trigger = !current && previous;  break;
        case KeyState::Pressed: trigger = current;               break;
        }
        if (trigger) command->Execute(deltaTime);
    }

    for (int i = 0; i < MAX_CONTROLLERS; ++i)
        m_Gamepads[i].Update();

    for (auto& [binding, command] : m_ControllerCommands)
    {
        auto [idx, button, state] = binding;
        bool trigger = false;
        switch (state)
        {
        case KeyState::Down:    trigger = m_Gamepads[idx].IsDownThisFrame(button); break;
        case KeyState::Up:      trigger = m_Gamepads[idx].IsUpThisFrame(button);   break;
        case KeyState::Pressed: trigger = m_Gamepads[idx].IsPressed(button);       break;
        }
        if (trigger) command->Execute(deltaTime);
    }

    return true;
}

void dae::InputManager::BindKeyboardCommand(SDL_Scancode key, KeyState state,
    std::unique_ptr<Command> command)
{
    m_KeyboardCommands[{key, state}] = std::move(command);
}
void dae::InputManager::UnbindKeyboardCommand(SDL_Scancode key, KeyState state)
{
    m_KeyboardCommands.erase({ key, state });
}
void dae::InputManager::BindControllerCommand(unsigned int idx, Gamepad::Button button,
    KeyState state, std::unique_ptr<Command> command)
{
    m_ControllerCommands[{idx, button, state}] = std::move(command);
}
void dae::InputManager::UnbindControllerCommand(unsigned int idx, Gamepad::Button button, KeyState state)
{
    m_ControllerCommands.erase({ idx, button, state });
}