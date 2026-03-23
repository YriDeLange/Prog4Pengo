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

        Subject::ObserverId AddObserver(Subject::EventHandler handler)
        {
            return m_Subject.AddObserver(std::move(handler));
        }

        void RemoveObserver(Subject::ObserverId id)
        {
            m_Subject.RemoveObserver(id);
        }

        void AddPoints(int amount, unsigned int eventId = GameEvent::EnemyDied)
        {
            m_Points += amount;
            m_Subject.Notify(eventId);

        #if USE_STEAMWORKS
            if (!m_WinnerAchievementGranted && m_Points >= 500 && g_SteamAchievements)
            {
                g_SteamAchievements->SetAchievement("ACH_WIN_ONE_GAME");
                m_WinnerAchievementGranted = true;
            }
        #endif
        }
    };
}