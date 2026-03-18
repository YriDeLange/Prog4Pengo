#pragma once
#include "Command.h"
#include "HealthComponent.h"

namespace dae
{
    class DieCommand final : public GameObjectCommand
    {
    public:
        explicit DieCommand(GameObject* pGO) : GameObjectCommand(pGO) {}

        void Execute(float) override
        {
            auto* health = GetGameObject()->GetComponent<HealthComponent>();
            if (health) health->Die();
        }
    };
}