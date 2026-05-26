#include "MenuCommands.h"
#include "MenuState.h"

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