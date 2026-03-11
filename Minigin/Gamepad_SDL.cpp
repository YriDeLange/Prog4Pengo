#include "Gamepad.h"
#include <SDL3/SDL.h>

class dae::Gamepad::GamepadImpl
{
    SDL_Gamepad* m_pGamepad{ nullptr };
    bool m_CurrentButtons[SDL_GAMEPAD_BUTTON_COUNT]{};
    bool m_PreviousButtons[SDL_GAMEPAD_BUTTON_COUNT]{};

public:
    void Open(unsigned int index)
    {
        m_pGamepad = SDL_OpenGamepad(index);
    }

    void Update()
    {
        memcpy(m_PreviousButtons, m_CurrentButtons, sizeof(m_CurrentButtons));
        for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i)
            m_CurrentButtons[i] = SDL_GetGamepadButton(m_pGamepad,
                static_cast<SDL_GamepadButton>(i));
    }

    bool IsDownThisFrame(unsigned int button) const
    {
        return m_ButtonsPressedThisFrame & button;
    }

    bool IsUpThisFrame(unsigned int button) const
    {
        return m_ButtonsReleasedThisFrame & button;
    }

    bool IsPressed(unsigned int button) const
    {
        return m_CurrentState.Gamepad.wButtons & button;
    }
};

dae::Gamepad::Gamepad(unsigned int controllerIndex)
    : m_ControllerIndex(controllerIndex)
    , m_pImpl(new GamepadImpl{}) {
}

dae::Gamepad::~Gamepad() { delete m_pImpl; }

void dae::Gamepad::Update() { m_pImpl->Update(m_ControllerIndex); }

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
};