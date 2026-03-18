#pragma once
#include "Command.h"
#include "PointsComponent.h"

namespace dae
{
    class AddPointsCommand final : public GameObjectCommand
    {
        int m_Amount;
    public:
        AddPointsCommand(GameObject* pGO, int amount)
            : GameObjectCommand(pGO), m_Amount(amount) {
        }

        void Execute(float) override
        {
            auto* points = GetGameObject()->GetComponent<PointsComponent>();
            if (points) points->AddPoints(m_Amount);
        }
    };
}