#pragma once
#include <vector>
#include <functional>

namespace dae
{
    class Subject
    {
    public:
        using EventHandler = std::function<void(unsigned int eventId)>;
        using ObserverId = size_t;

        ObserverId AddObserver(EventHandler handler)
        {
            m_Handlers.emplace_back(m_NextId, std::move(handler));
            return m_NextId++;
        }

        void RemoveObserver(ObserverId id)
        {
            std::erase_if(m_Handlers, [id](const auto& pair) { return pair.first == id; });
        }

        void Notify(unsigned int eventId)
        {
            for (auto& [id, handler] : m_Handlers)
                handler(eventId);
        }

    private:
        std::vector<std::pair<ObserverId, EventHandler>> m_Handlers;
        ObserverId m_NextId{ 0 };
    };
}