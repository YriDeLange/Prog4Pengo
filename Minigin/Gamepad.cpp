#ifndef __EMSCRIPTEN__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <XInput.h>
#endif

#include "Gamepad.h"

#ifndef __EMSCRIPTEN__

class dae::Gamepad::GamepadImpl
{
    XINPUT_STATE m_PreviousState{};
    XINPUT_STATE m_CurrentState{};
    WORD m_ButtonsPressedThisFrame{};
    WORD m_ButtonsReleasedThisFrame{};

public:
    void Update(unsigned int controllerIndex)
    {
        CopyMemory(&m_PreviousState, &m_CurrentState, sizeof(XINPUT_STATE));
        ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
        XInputGetState(controllerIndex, &m_CurrentState);

        auto buttonChanges = m_CurrentState.Gamepad.wButtons ^ m_PreviousState.Gamepad.wButtons;
        m_ButtonsPressedThisFrame = buttonChanges & m_CurrentState.Gamepad.wButtons;
        m_ButtonsReleasedThisFrame = buttonChanges & (~m_CurrentState.Gamepad.wButtons);
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

#else
class dae::Gamepad::GamepadImpl
{
public:
    void Update(unsigned int) {}
    bool IsDownThisFrame(unsigned int) const { return false; }
    bool IsUpThisFrame(unsigned int) const { return false; }
    bool IsPressed(unsigned int) const { return false; }
};

#endif

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