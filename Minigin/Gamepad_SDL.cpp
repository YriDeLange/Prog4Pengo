#include "Gamepad.h"
#include <SDL3/SDL.h>
#include <cstring>

class dae::Gamepad::GamepadImpl
{
    unsigned int m_ControllerIndex;
    SDL_Gamepad* m_pGamepad{ nullptr };
    bool m_CurrentButtons[SDL_GAMEPAD_BUTTON_COUNT]{};
    bool m_PreviousButtons[SDL_GAMEPAD_BUTTON_COUNT]{};

    static SDL_GamepadButton ToSDLButton(unsigned int button)
    {
        switch (button)
        {
        case 0x0001: return SDL_GAMEPAD_BUTTON_DPAD_UP;
        case 0x0002: return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
        case 0x0004: return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
        case 0x0008: return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
        default:     return SDL_GAMEPAD_BUTTON_INVALID;
        }
    }

public:
    explicit GamepadImpl(unsigned int controllerIndex)
        : m_ControllerIndex(controllerIndex) {
    }

    void Open()
    {
        int count{};
        SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
        if (gamepads && static_cast<int>(m_ControllerIndex) < count)
            m_pGamepad = SDL_OpenGamepad(gamepads[m_ControllerIndex]);
        SDL_free(gamepads);
    }

    void Update()
    {
        if (!m_pGamepad)
            Open();

        memcpy(m_PreviousButtons, m_CurrentButtons, sizeof(m_CurrentButtons));
        if (m_pGamepad)
        {
            for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i)
                m_CurrentButtons[i] = SDL_GetGamepadButton(m_pGamepad,
                    static_cast<SDL_GamepadButton>(i));
        }
    }

    bool IsDownThisFrame(unsigned int button) const
    {
        auto sdlBtn = ToSDLButton(button);
        if (sdlBtn == SDL_GAMEPAD_BUTTON_INVALID) return false;
        return m_CurrentButtons[sdlBtn] && !m_PreviousButtons[sdlBtn];
    }

    bool IsUpThisFrame(unsigned int button) const
    {
        auto sdlBtn = ToSDLButton(button);
        if (sdlBtn == SDL_GAMEPAD_BUTTON_INVALID) return false;
        return !m_CurrentButtons[sdlBtn] && m_PreviousButtons[sdlBtn];
    }

    bool IsPressed(unsigned int button) const
    {
        auto sdlBtn = ToSDLButton(button);
        if (sdlBtn == SDL_GAMEPAD_BUTTON_INVALID) return false;
        return m_CurrentButtons[sdlBtn];
    }
};

dae::Gamepad::Gamepad(unsigned int controllerIndex)
    : m_pImpl(new GamepadImpl{ controllerIndex })
{
    m_pImpl->Open();
}

dae::Gamepad::~Gamepad() { delete m_pImpl; }

void dae::Gamepad::Update() { m_pImpl->Update(); }

bool dae::Gamepad::IsDownThisFrame(Button b) const
{
    return m_pImpl->IsDownThisFrame(static_cast<unsigned int>(b));
}

bool dae::Gamepad::IsUpThisFrame(Button b) const
{
    return m_pImpl->IsUpThisFrame(static_cast<unsigned int>(b));
}

bool dae::Gamepad::IsPressed(Button b) const
{
    return m_pImpl->IsPressed(static_cast<unsigned int>(b));
}

void dae::Gamepad::OnGamepadAdded()
{
    if (!m_pImpl->m_pGamepad)
        m_pImpl->Open();
}