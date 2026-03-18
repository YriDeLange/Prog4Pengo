#pragma once
#include "Component.h"
#include "Subject.h"
#include "GameEvents.h"

namespace dae
{
    class HealthComponent final : public Component
    {
        int m_Lives;
        Subject m_Subject;
    public:
        explicit HealthComponent(GameObject* pOwner, int lives = 3)
            : Component(pOwner), m_Lives(lives) {
        }

        int GetLives() const { return m_Lives; }

        void AddObserver(Subject::EventHandler handler)
        {
            m_Subject.AddObserver(std::move(handler));
        }

        void Die()
        {
            if (m_Lives <= 0) return;
            --m_Lives;
            m_Subject.Notify(GameEvent::PlayerDied);
        }
    };
}