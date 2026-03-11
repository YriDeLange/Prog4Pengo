#pragma once
#include <memory>

namespace dae
{
    class Command
    {
    public:
        virtual ~Command() = default;
        virtual void Execute(float deltaTime) = 0;
    };

    class GameObject;

    class GameObjectCommand : public Command
    {
        GameObject* m_pGameObject;
    protected:
        GameObject* GetGameObject() const { return m_pGameObject; }
    public:
        explicit GameObjectCommand(GameObject* pGameObject) : m_pGameObject(pGameObject) {}
        virtual ~GameObjectCommand() = default;
    };
}