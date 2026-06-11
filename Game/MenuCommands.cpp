#include "MenuCommands.h"
#include "States/MenuState.h"

namespace dae
{
    void MenuNavigateCommand::Execute(float /*deltaTime*/)
    {
        if (m_pMenu)
            m_pMenu->MoveSelection(m_delta);
    }

    void MenuConfirmCommand::Execute(float /*deltaTime*/)
    {
        if (m_pMenu)
            m_pMenu->Confirm();
    }
}