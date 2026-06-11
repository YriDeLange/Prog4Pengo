#pragma once
#include "Singleton.h"
#include <vector>
#include <algorithm>

namespace dae
{
    class SnoBeeComponent;

    class SnoBeeRegistry final : public Singleton<SnoBeeRegistry>
    {
    public:
        void Register(SnoBeeComponent* snoBee)
        {
            m_snoBees.push_back(snoBee);
        }

        void Unregister(SnoBeeComponent* snoBee)
        {
            std::erase(m_snoBees, snoBee);
        }

        const std::vector<SnoBeeComponent*>& GetAll() const { return m_snoBees; }
        size_t Count() const { return m_snoBees.size(); }

        void Clear() { m_snoBees.clear(); }

    private:
        friend class Singleton<SnoBeeRegistry>;
        SnoBeeRegistry() = default;

        std::vector<SnoBeeComponent*> m_snoBees;
    };
}