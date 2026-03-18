#pragma once
#include <vector>
#include <functional>

namespace dae
{
    class Subject
    {
    public:
        using EventHandler = std::function<void(unsigned int eventId)>;

        void AddObserver(EventHandler handler)
        {
            m_Handlers.push_back(std::move(handler));
        }
        void Notify(unsigned int eventId)
        {
            for (auto& handler : m_Handlers)
                handler(eventId);
        }

    private:
        std::vector<EventHandler> m_Handlers;
    };
}