#pragma once

#if USE_STEAMWORKS
#include "SteamAchievements.h"
extern CSteamAchievements* g_SteamAchievements;
bool m_WinnerAchievementGranted = false;
#endif