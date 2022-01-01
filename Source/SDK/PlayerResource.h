#pragma once

#include "Inconstructible.h"
#include "../Netvars.h"
#include "VirtualMethod.h"

struct Vector;

class IPlayerResource {
public:
    INCONSTRUCTIBLE(IPlayerResource)

    VIRTUAL_METHOD_V(bool, isAlive, 5, (int index), (this, index))
    VIRTUAL_METHOD_V(const char*, getPlayerName, 8, (int index), (this, index))
    VIRTUAL_METHOD_V(int, getPlayerHealth, 14, (int index), (this, index))
};

class PlayerResource {
public:
    INCONSTRUCTIBLE(PlayerResource)

    auto getIPlayerResource() noexcept
    {
        return reinterpret_cast<IPlayerResource*>(std::uintptr_t(this) + WIN32_LINUX(0x9D8, 0xF68));
    }

    NETVAR(bombsiteCenterA, "CCSPlayerResource", "m_bombsiteCenterA", Vector)
    NETVAR(bombsiteCenterB, "CCSPlayerResource", "m_bombsiteCenterB", Vector)
    NETVAR(musicID, "CCSPlayerResource", "m_nMusicID", int[65])
    NETVAR(rank, "CCSPlayerResource", "m_iCompetitiveRanking", int[65]) // 0x1A84 is netvar offset
    NETVAR(wins, "CCSPlayerResource", "m_iCompetitiveWins", int[65])
    NETVAR(mvps, "CCSPlayerResource", "m_iMVPs", int[65])
    NETVAR(vip, "CCSPlayerResource", "m_iPlayerVIP", int[65])
    NETVAR(score, "CCSPlayerResource", "m_iScore", int[65])
    NETVAR(level, "CCSPlayerResource", "m_nPersonaDataPublicLevel", int[65])
    NETVAR(commendsLeader, "CCSPlayerResource", "m_nPersonaDataPublicCommendsLeader", int[65])
    NETVAR(commendsTeacher, "CCSPlayerResource", "m_nPersonaDataPublicCommendsTeacher", int[65])
    NETVAR(commendsFriendly, "CCSPlayerResource", "m_nPersonaDataPublicCommendsFriendly", int[65])
    NETVAR(activeCoinRank, "CCSPlayerResource", "m_nActiveCoinRank", int[65])
    NETVAR(clanTag, "CCSPlayerResource", "m_szClan", char)
};