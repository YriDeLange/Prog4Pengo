#include "SteamAchievements.h"
#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if USE_STEAMWORKS

CSteamAchievements::CSteamAchievements(Achievement_t* Achievements, int NumAchievements)
    : m_iAppID(0)
    , m_bInitialized(false)
    , m_CallbackUserStatsStored(this, &CSteamAchievements::OnUserStatsStored)
    , m_CallbackAchievementStored(this, &CSteamAchievements::OnAchievementStored)
{
    m_iAppID = SteamUtils()->GetAppID();
    m_pAchievements = Achievements;
    m_iNumAchievements = NumAchievements;
    m_bInitialized = Initialize();
}

bool CSteamAchievements::Initialize()
{
    if (NULL == SteamUserStats() || NULL == SteamUser())
        return false;
    if (!SteamUser()->BLoggedOn())
        return false;
    return true;
}

bool CSteamAchievements::SetAchievement(const char* ID)
{
    if (m_bInitialized)
    {
        SteamUserStats()->SetAchievement(ID);
        return SteamUserStats()->StoreStats();
    }
    return false;
}

void CSteamAchievements::OnUserStatsStored(UserStatsStored_t* pCallback)
{
    if (static_cast<uint64>(m_iAppID) == pCallback->m_nGameID)
    {
        if (k_EResultOK == pCallback->m_eResult)
            OutputDebugString("Stored stats for Steam\n");
        else
        {
            char buffer[128];
            _snprintf_s(buffer, 128, _TRUNCATE, "StatsStored - failed, %d\n", pCallback->m_eResult);
            OutputDebugString(buffer);
        }
    }
}

void CSteamAchievements::OnAchievementStored(UserAchievementStored_t* pCallback)
{
    if (static_cast<uint64>(m_iAppID) == pCallback->m_nGameID)
        OutputDebugString("Stored Achievement for Steam\n");
}

#endif