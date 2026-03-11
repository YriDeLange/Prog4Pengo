#pragma once
#include "Command.h"
#include "glm/vec2.hpp"

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
            auto pos = GetGameObject()->GetLocalPosition();
            pos += glm::vec3(m_Direction * m_Speed * deltaTime, 0.f);
            GetGameObject()->SetLocalPosition(pos);
        }
    };
}