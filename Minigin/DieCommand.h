#pragma once
#include "Command.h"
#include "HealthComponent.h"

namespace dae
{
    class DieCommand final : public GameObjectCommand
    {
        float m_Cooldown{ 0.1f };
        float m_TimeSinceLastFire{ m_Cooldown };

    public:
        explicit DieCommand(GameObject* pGO) : GameObjectCommand(pGO) {}

        void Execute(float deltaTime) override
        {
            m_TimeSinceLastFire += deltaTime;
            if (m_TimeSinceLastFire < m_Cooldown) return;
            m_TimeSinceLastFire = 0.f;

            auto* health = GetGameObject()->GetComponent<HealthComponent>();
            if (health) health->Die();
        }
    };
}