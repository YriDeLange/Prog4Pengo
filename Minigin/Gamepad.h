#pragma once
namespace dae
{
    class Gamepad
    {
    public:
        enum class Button : unsigned int
        {
            DpadUp = 0x0001,
            DpadDown = 0x0002,
            DpadLeft = 0x0004,
            DpadRight = 0x0008,
        };

        explicit Gamepad(unsigned int controllerIndex);
        ~Gamepad();

        void Update();

        bool IsDownThisFrame(Button button) const;
        bool IsUpThisFrame(Button button) const;
        bool IsPressed(Button button) const;

    private:
        class GamepadImpl;
        GamepadImpl* m_pImpl;
        unsigned int m_ControllerIndex;
    };
}