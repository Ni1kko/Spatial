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
        return reinterpret_cast<IPlayerResource*>(std::uintptr_t(this) + 0x9D8);
    }

    NETVAR(bombsiteCenterA, "CCSPlayerResource", "m_bombsiteCenterA", Vector)
    NETVAR(bombsiteCenterB, "CCSPlayerResource", "m_bombsiteCenterB", Vector)
    NETVAR(musicID, "CCSPlayerResource", "m_nMusicID", int[65])
    NETVAR(rank, "CCSPlayerResource", "m_iCompetitiveRanking", int[65]) // 0x1A84 is netvar offset
    NETVAR(wins, "CCSPlayerResource", "m_iCompetitiveWins", int)
    NETVAR(mvps, "CCSPlayerResource", "m_iMVPs", int)
    NETVAR(vip, "CCSPlayerResource", "m_iPlayerVIP", int)
    NETVAR(score, "CCSPlayerResource", "m_iScore", int)
    NETVAR(level, "CCSPlayerResource", "m_nPersonaDataPublicLevel", int)
    NETVAR(commendsLeader, "CCSPlayerResource", "m_nPersonaDataPublicCommendsLeader", int)
    NETVAR(commendsTeacher, "CCSPlayerResource", "m_nPersonaDataPublicCommendsTeacher", int)
    NETVAR(commendsFriendly, "CCSPlayerResource", "m_nPersonaDataPublicCommendsFriendly", int)
    NETVAR(activeCoinRank, "CCSPlayerResource", "m_nActiveCoinRank", int[65])
    NETVAR(clanTag, "CCSPlayerResource", "m_szClan", char);
};