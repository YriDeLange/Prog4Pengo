#pragma once
#include "Command.h"
#include "glm/vec2.hpp"
#include "PengoComponent.h"
#include "Pengo.h"

namespace dae
{
    class GameObject;

    class MoveCommand : public GameObjectCommand
    {
        glm::vec2 m_Direction;
        float     m_Speed;
    public:
        MoveCommand(GameObject* pGO, glm::vec2 direction, float speed)
            : GameObjectCommand(pGO), m_Direction(direction), m_Speed(speed) {
        }

        void Execute(float deltaTime) override
        {
            auto* pengoComp = GetGameObject()->GetComponent<PengoComponent>();
            if (pengoComp)
            {
                auto* pengo = pengoComp->GetPengo();
                if (pengo)
                {
                    if (m_Direction.y < 0)      pengo->MoveUp();
                    else if (m_Direction.y > 0) pengo->MoveDown();
                    else if (m_Direction.x < 0) pengo->MoveLeft();
                    else if (m_Direction.x > 0) pengo->MoveRight();
                }
            }
            else
            {
                auto pos = GetGameObject()->GetLocalPosition();
                pos += glm::vec3(m_Direction * m_Speed * deltaTime, 0.f);
                GetGameObject()->SetLocalPosition(pos);
            }
        }
    };

    class StopMoveCommand : public GameObjectCommand
    {
    public:
        explicit StopMoveCommand(GameObject* pGO)
            : GameObjectCommand(pGO) {
        }

        void Execute(float /*deltaTime*/) override
        {
            auto* pengoComp = GetGameObject()->GetComponent<PengoComponent>();
            if (pengoComp)
            {
                auto* pengo = pengoComp->GetPengo();
                if (pengo)
                    pengo->StopMoving();
            }
        }
    };

    class PushCommand : public GameObjectCommand
    {
    public:
        explicit PushCommand(GameObject* pGO)
            : GameObjectCommand(pGO) {
        }

        void Execute(float /*deltaTime*/) override
        {
            auto* pengoComp = GetGameObject()->GetComponent<PengoComponent>();
            if (pengoComp)
            {
                auto* pengo = pengoComp->GetPengo();
                if (pengo)
                    pengo->TryPush();
            }
        }
    };
}