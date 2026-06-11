#pragma once
#include "Command.h"

class MenuState;

namespace dae
{
    class MenuNavigateCommand final : public Command
    {
        MenuState* m_pMenu;
        int        m_delta;
    public:
        MenuNavigateCommand(MenuState* menu, int delta)
            : m_pMenu(menu), m_delta(delta) {}

        void Execute(float deltaTime) override;
    };

    class MenuConfirmCommand final : public Command
    {
        MenuState* m_pMenu;
    public:
        explicit MenuConfirmCommand(MenuState* menu)
            : m_pMenu(menu) {}

        void Execute(float deltaTime) override;
    };
}