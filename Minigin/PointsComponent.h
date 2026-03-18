#pragma once
#include "Component.h"
#include "Subject.h"
#include "GameEvents.h"
#include "SteamGlobals.h"

namespace dae
{
    class PointsComponent final : public Component
    {
        int m_Points{};
        Subject m_Subject;
    public:
        explicit PointsComponent(GameObject* pOwner) : Component(pOwner) {}

        int GetPoints() const { return m_Points; }

        void AddObserver(Subject::EventHandler handler)
        {
            m_Subject.AddObserver(std::move(handler));
        }

        void AddPoints(int amount)
        {
            m_Points += amount;
            m_Subject.Notify(GameEvent::PointsScored);

        #if USE_STEAMWORKS
            if (m_Points >= 500 && g_SteamAchievements)
                g_SteamAchievements->SetAchievement("ACH_WIN_ONE_GAME");
        #endif
        }
    };
}