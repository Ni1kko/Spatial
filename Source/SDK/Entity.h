#pragma once

#include <functional>
#include <string>

#include "AnimState.h"
#include "Inconstructible.h"
#include "Platform.h"
#include "Vector.h"
#include "VirtualMethod.h"
#include "WeaponData.h"
#include "WeaponId.h"

#include "../Netvars.h"

class EconItemView;

class matrix3x4;

struct AnimState;
struct ClientClass;
struct Model;
struct VarMap;

enum class MoveType { 
    NONE = 0,          /**< never moves */
    ISOMETRIC,         /**< For players */
    WALK,              /**< Player only - moving on the ground */
    STEP,              /**< gravity, special edge handling -- monsters use this */
    FLY,               /**< No gravity, but still collides with stuff */
    FLYGRAVITY,        /**< flies through the air + is affected by gravity */
    VPHYSICS,          /**< uses VPHYSICS for simulation */
    PUSH,              /**< no clip to world, push and crush */
    NOCLIP,            /**< No gravity, no collisions, still do velocity/avelocity */
    LADDER,            /**< Used by players only when going onto a ladder */
    OBSERVER,          /**< Observer movement, depends on player's observer mode */
    CUSTOM             /**< Allows the entity to describe its own physics */
};

enum class ObsMode {
    None = 0,
    Deathcam,
    Freezecam,
    Fixed,
    InEye,
    Chase,
    Roaming
};

enum class Team {
    None = 0,
    Spectators,
    TT,
    CT
};

class Collideable {
public:
    INCONSTRUCTIBLE(Collideable)

    VIRTUAL_METHOD(const Vector&, obbMins, 1, (), (this))
    VIRTUAL_METHOD(const Vector&, obbMaxs, 2, (), (this))
};

class Entity {
public:
    INCONSTRUCTIBLE(Entity)

    VIRTUAL_METHOD(void, release, 1, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(ClientClass*, getClientClass, 2, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(void, onDataChanged, 5, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
    VIRTUAL_METHOD(void, preDataUpdate, 6, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
    VIRTUAL_METHOD(void, postDataUpdate, 7, (int updateType), (this + sizeof(uintptr_t) * 2, updateType))
    VIRTUAL_METHOD(bool, isDormant, 9, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(int, index, 10, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(void, setDestroyedOnRecreateEntities, 13, (), (this + sizeof(uintptr_t) * 2))
    VIRTUAL_METHOD(Vector&, getRenderOrigin, 1, (), (this + sizeof(uintptr_t)))
    VIRTUAL_METHOD(bool, shouldDraw, 3, (), (this + sizeof(uintptr_t)))
    VIRTUAL_METHOD(const Model*, getModel, 8, (), (this + sizeof(uintptr_t)))
    VIRTUAL_METHOD(const matrix3x4&, toWorldTransform, 32, (), (this + sizeof(uintptr_t)))

    VIRTUAL_METHOD_V(int&, handle, 2, (), (this))
    VIRTUAL_METHOD_V(Collideable*, getCollideable, 3, (), (this))

    VIRTUAL_METHOD(const Vector&, getAbsOrigin, 10, (), (this))
    VIRTUAL_METHOD(void, setModelIndex, 75, (int index), (this, index))
    VIRTUAL_METHOD(bool, getAttachment, 84, (int index, Vector& origin), (this, index, std::ref(origin)))
    VIRTUAL_METHOD(Team, getTeamNumber, 88, (), (this))
    VIRTUAL_METHOD(int, health, 122, (), (this))
    VIRTUAL_METHOD(bool, isAlive, 156, (), (this))
    VIRTUAL_METHOD(bool, isPlayer, 158, (), (this))
    VIRTUAL_METHOD(bool, isWeapon, 166, (), (this))
    VIRTUAL_METHOD(Entity*, getActiveWeapon, 268, (), (this))
    VIRTUAL_METHOD(int, getWeaponSubType, 282, (), (this))
    VIRTUAL_METHOD(ObsMode, getObserverMode, 294, (), (this))
    VIRTUAL_METHOD(Entity*, getObserverTarget, 295, (), (this))
    VIRTUAL_METHOD(WeaponType, getWeaponType, 455, (), (this))
    VIRTUAL_METHOD(WeaponInfo*, getWeaponData, 461, (), (this))
    VIRTUAL_METHOD(int, getMuzzleAttachmentIndex1stPerson, 468, (Entity* viewModel), (this, viewModel))
    VIRTUAL_METHOD(int, getMuzzleAttachmentIndex3rdPerson, 469, (), (this))
    VIRTUAL_METHOD(float, getInaccuracy, 483, (), (this))

    auto getEyePosition() noexcept
    {
        Vector v;
        VirtualMethod::call<void, 285>(this, std::ref(v));
        return v;
    }

    auto getAimPunch() noexcept
    {
        Vector v;
        VirtualMethod::call<void, 346>(this, std::ref(v));
        return v;
    }

    auto isPistol() noexcept { return getWeaponType() == WeaponType::Pistol; }
    auto isSniperRifle() noexcept { return getWeaponType() == WeaponType::SniperRifle; }
    auto isScopedRifle() noexcept { return ID() == WeaponId::Aug || ID() == WeaponId::Sg553; }
    auto hasScope() noexcept { return isSniperRifle() || isScopedRifle(); } 
    auto isGrenade() noexcept { return getWeaponType() == WeaponType::Grenade; }
    auto isKnife() noexcept { return getWeaponType() == WeaponType::Knife; }

    auto isFullAuto() noexcept
    {
        const auto weaponData = getWeaponData();
        if (weaponData)
            return weaponData->fullAuto;
        return false;
    }

    auto requiresRecoilControl() noexcept
    {
        const auto weaponData = getWeaponData();
        if (weaponData)
            return weaponData->recoilMagnitude < 35.0f && weaponData->recoveryTimeStand > weaponData->cycletime;
        return false;
    }

    bool setupBones(matrix3x4* out, int maxBones, int boneMask, float currentTime) noexcept;
    Vector getBonePosition(int bone) noexcept;

    bool isVisible(const Vector& position = { }) noexcept;
    bool isOtherEnemy(Entity* other) noexcept;

    VarMap& getVarMap() noexcept
    {
        return *reinterpret_cast<VarMap*>(std::uintptr_t(this) + 0x24);
    }
   
    AnimState* getAnimstate() noexcept
    {
        return *reinterpret_cast<AnimState**>(this + 0x3914);
    }

    float getMaxDesyncAngle() noexcept;

    bool isInReload() noexcept
    {
        return *reinterpret_cast<bool*>(uintptr_t(&clip()) + 0x41);
    }

    int getUserId() noexcept;
    std::uint64_t getSteamId() noexcept;

    void getPlayerName(char(&out)[128]) noexcept;
    [[nodiscard]] std::string getPlayerName() noexcept
    {
        char name[128];
        getPlayerName(name);
        return name;
    }

    bool canSee(Entity* other, const Vector& pos) noexcept;
    bool visibleTo(Entity* other) noexcept;
  
    NETVAR(body, "CBaseAnimating", "m_nBody", int)
    NETVAR(hitboxSet, "CBaseAnimating", "m_nHitboxSet", int)

    NETVAR(modelIndex, "CBaseEntity", "m_nModelIndex", unsigned)
    NETVAR(origin, "CBaseEntity", "m_vecOrigin", Vector)
    NETVAR_OFFSET(moveType, "CBaseEntity", "m_nRenderMode", 1, MoveType)
    NETVAR(simulationTime, "CBaseEntity", "m_flSimulationTime", float)
    NETVAR(ownerEntity, "CBaseEntity", "m_hOwnerEntity", int)
    NETVAR(spotted, "CBaseEntity", "m_bSpotted", bool)

    NETVAR(weapons, "CBaseCombatCharacter", "m_hMyWeapons", int[64])
    PNETVAR(wearables, "CBaseCombatCharacter", "m_hMyWearables", int)

    NETVAR(viewModel, "CBasePlayer", "m_hViewModel[0]", int)
    NETVAR(fov, "CBasePlayer", "m_iFOV", int)
    NETVAR(fovStart, "CBasePlayer", "m_iFOVStart", int)
    NETVAR(defaultFov, "CBasePlayer", "m_iDefaultFOV", int)
    NETVAR(flags, "CBasePlayer", "m_fFlags", int)
    NETVAR(tickBase, "CBasePlayer", "m_nTickBase", int)
    NETVAR(aimPunchAngle, "CBasePlayer", "m_aimPunchAngle", Vector)
    NETVAR(viewPunchAngle, "CBasePlayer", "m_viewPunchAngle", Vector)
    NETVAR(velocity, "CBasePlayer", "m_vecVelocity[0]", Vector)
    NETVAR(lastPlaceName, "CBasePlayer", "m_szLastPlaceName", char[18])

    NETVAR(armor, "CCSPlayer", "m_ArmorValue", int)
    NETVAR(eyeAngles, "CCSPlayer", "m_angEyeAngles", Vector)
    NETVAR(isScoped, "CCSPlayer", "m_bIsScoped", bool)
    NETVAR(isDefusing, "CCSPlayer", "m_bIsDefusing", bool)
    NETVAR_OFFSET(flashDuration, "CCSPlayer", "m_flFlashMaxAlpha", -8, float)
    NETVAR(flashMaxAlpha, "CCSPlayer", "m_flFlashMaxAlpha", float)
    NETVAR(gunGameImmunity, "CCSPlayer", "m_bGunGameImmunity", bool)
    NETVAR(account, "CCSPlayer", "m_iAccount", int)
    NETVAR(inBombZone, "CCSPlayer", "m_bInBombZone", bool)
    NETVAR(hasDefuser, "CCSPlayer", "m_bHasDefuser", bool)
    NETVAR(hasHelmet, "CCSPlayer", "m_bHasHelmet", bool)
    NETVAR(lby, "CCSPlayer", "m_flLowerBodyYawTarget", float)
    NETVAR(ragdoll, "CCSPlayer", "m_hRagdoll", int)
    NETVAR(shotsFired, "CCSPlayer", "m_iShotsFired", int)
    NETVAR(waitForNoAttack, "CCSPlayer", "m_bWaitForNoAttack", bool)
    NETVAR(playerPatchIndices, "CCSPlayer", "m_vecPlayerPatchEconIndices", int[5])

    NETVAR(viewModelIndex, "CBaseCombatWeapon", "m_iViewModelIndex", int)
    NETVAR(worldModelIndex, "CBaseCombatWeapon", "m_iWorldModelIndex", int)
    NETVAR(worldDroppedModelIndex, "CBaseCombatWeapon", "m_iWorldDroppedModelIndex", int)
    NETVAR(weaponWorldModel, "CBaseCombatWeapon", "m_hWeaponWorldModel", int)
    NETVAR(clip, "CBaseCombatWeapon", "m_iClip1", int)
    NETVAR(reserveAmmoCount, "CBaseCombatWeapon", "m_iPrimaryReserveAmmoCount", int)
    NETVAR(nextPrimaryAttack, "CBaseCombatWeapon", "m_flNextPrimaryAttack", float)

    NETVAR(nextAttack, "CBaseCombatCharacter", "m_flNextAttack", float)

    NETVAR(accountID, "CBaseAttributableItem", "m_iAccountID", int)
    NETVAR(itemDefinitionIndex, "CBaseAttributableItem", "m_iItemDefinitionIndex", WeaponId)
    NETVAR(itemIDHigh, "CBaseAttributableItem", "m_iItemIDHigh", std::uint32_t)
    NETVAR(itemIDLow, "CBaseAttributableItem", "m_iItemIDLow", std::uint32_t)
    NETVAR(entityQuality, "CBaseAttributableItem", "m_iEntityQuality", int)
    NETVAR(initialized, "CBaseAttributableItem", "m_bInitialized", bool)
    NETVAR(econItemView, "CBaseAttributableItem", "m_Item", EconItemView)
    NETVAR(originalOwnerXuidLow, "CBaseAttributableItem", "m_OriginalOwnerXuidLow", std::uint32_t)
    NETVAR(originalOwnerXuidHigh, "CBaseAttributableItem", "m_OriginalOwnerXuidHigh", std::uint32_t)

    NETVAR(owner, "CBaseViewModel", "m_hOwner", int)
    NETVAR(weapon, "CBaseViewModel", "m_hWeapon", int)

    NETVAR(c4StartedArming, "CC4", "m_bStartedArming", bool)

    NETVAR(tabletReceptionIsBlocked, "CTablet", "m_bTabletReceptionIsBlocked", bool)
    
    NETVAR(droneTarget, "CDrone", "m_hMoveToThisEntity", int)

    NETVAR(thrower, "CBaseGrenade", "m_hThrower", int)
        
    NETVAR(mapHasBombTarget, "CCSGameRulesProxy", "m_bMapHasBombTarget", bool)
    NETVAR(isValveDS, "CCSGameRulesProxy", "m_bIsValveDS", bool)

    NETVAR(fireXDelta, "CInferno", "m_fireXDelta", int[100])
    NETVAR(fireYDelta, "CInferno", "m_fireYDelta", int[100])
    NETVAR(fireZDelta, "CInferno", "m_fireZDelta", int[100])
    NETVAR(fireIsBurning, "CInferno", "m_bFireIsBurning", bool[100])
    NETVAR(fireCount, "CInferno", "m_fireCount", int)
        
    NETVAR(zoomLevel, "CWeaponCSBaseGun", "m_zoomLevel", int)

    bool isFlashed() noexcept
    {
        return flashDuration() > 75.0f;
    }

    WeaponId ID() noexcept
    {
        return itemDefinitionIndex();
    }

    std::uint64_t originalOwnerXuid() noexcept
    {
        return (std::uint64_t(originalOwnerXuidHigh()) << 32) | originalOwnerXuidLow();
    }

    std::uint64_t itemID() noexcept
    {
        return (std::uint64_t(itemIDHigh()) << 32) | itemIDLow();
    }
};

static_assert(sizeof(Entity) == 1);
