#pragma once
#include <memory>
#include "GameState.h"

namespace dae
{
    // Owns the active GameState and drives its lifecycle. Transitions are applied
    // between Update calls so a state can safely request its own replacement.
    class GameStateMachine final
    {
    public:
        GameStateMachine() = default;
        ~GameStateMachine()
        {
            if (m_pCurrent)
                m_pCurrent->OnExit();
        }

        GameStateMachine(const GameStateMachine&) = delete;
        GameStateMachine(GameStateMachine&&) = delete;
        GameStateMachine& operator=(const GameStateMachine&) = delete;
        GameStateMachine& operator=(GameStateMachine&&) = delete;

        // Set the initial state (calls its OnEnter).
        void SetState(std::unique_ptr<GameState> state)
        {
            if (m_pCurrent)
                m_pCurrent->OnExit();

            m_pCurrent = std::move(state);

            if (m_pCurrent)
                m_pCurrent->OnEnter();
        }

        void Update(float deltaTime)
        {
            if (!m_pCurrent)
                return;

            // Capture the request first; only swap once Update has fully returned.
            std::unique_ptr<GameState> next = m_pCurrent->Update(deltaTime);
            if (next)
            {
                m_pCurrent->OnExit();
                m_pCurrent = std::move(next);
                m_pCurrent->OnEnter();
            }
        }

        void Render() const
        {
            if (m_pCurrent)
                m_pCurrent->Render();
        }

        GameState* GetCurrent() const { return m_pCurrent.get(); }

    private:
        std::unique_ptr<GameState> m_pCurrent;
    };
}