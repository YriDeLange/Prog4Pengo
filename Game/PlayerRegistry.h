#pragma once
#include "Singleton.h"
#include <vector>
#include <algorithm>

namespace dae
{
    class GameObject;
    class PlayerRegistry final : public Singleton<PlayerRegistry>
    {
    public:
        void Register(GameObject* player) { m_players.push_back(player); }
        void Unregister(GameObject* player) { std::erase(m_players, player); }

        const std::vector<GameObject*>& GetAll() const { return m_players; }
        void Clear() { m_players.clear(); }

    private:
        friend class Singleton<PlayerRegistry>;
        PlayerRegistry() = default;

        std::vector<GameObject*> m_players;
    };
}