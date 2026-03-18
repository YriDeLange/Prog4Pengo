#pragma once
#include "Command.h"
#include "PointsComponent.h"

namespace dae
{
    class AddPointsCommand final : public GameObjectCommand
    {
        int m_Amount;
        float m_Cooldown{ 0.1f };
        float m_TimeSinceLastFire{ m_Cooldown };

    public:
        AddPointsCommand(GameObject* pGO, int amount)
            : GameObjectCommand(pGO), m_Amount(amount) {
        }

        void Execute(float deltaTime) override
        {
            m_TimeSinceLastFire += deltaTime;
            if (m_TimeSinceLastFire < m_Cooldown) return;
            m_TimeSinceLastFire = 0.f;

            auto* points = GetGameObject()->GetComponent<PointsComponent>();
            if (points) points->AddPoints(m_Amount);
        }
    };
}