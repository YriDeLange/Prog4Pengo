#pragma once
#include "Singleton.h"
#include "Gamepad.h"
#include "Command.h"
#include <map>
#include <memory>
#include <SDL3/SDL.h>

namespace dae
{
    enum class KeyState { Down, Up, Pressed };

    class InputManager final : public Singleton<InputManager>
    {
    public:
        bool ProcessInput(float deltaTime);

        void BindKeyboardCommand(SDL_Scancode key, KeyState state,
            std::unique_ptr<Command> command);
        void UnbindKeyboardCommand(SDL_Scancode key, KeyState state);

        void BindControllerCommand(unsigned int controllerIdx, Gamepad::Button button,
            KeyState state, std::unique_ptr<Command> command);
        void UnbindControllerCommand(unsigned int controllerIdx, Gamepad::Button button,
            KeyState state);

    private:
        using KeyboardBinding = std::pair<SDL_Scancode, KeyState>;
        std::map<KeyboardBinding, std::unique_ptr<Command>> m_KeyboardCommands;

        using ControllerBinding = std::tuple<unsigned int, Gamepad::Button, KeyState>;
        std::map<ControllerBinding, std::unique_ptr<Command>> m_ControllerCommands;

        static constexpr int MAX_CONTROLLERS = 4;
        Gamepad m_Gamepads[MAX_CONTROLLERS] = {
            Gamepad{0}, Gamepad{1}, Gamepad{2}, Gamepad{3}
        };

        const bool* m_pKeyboardState{ nullptr };
        bool m_PreviousKeyboardState[SDL_SCANCODE_COUNT]{};
    };
}
