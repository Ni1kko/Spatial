#pragma once

#include <forward_list>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <Encryption\xorstr.hpp>

#include "SDK/matrix3x4.h"
#include "SDK/Vector.h"

#include "Menu/Texture.h"

struct LocalPlayerData;

struct PlayerData;
struct ObserverData;
struct WeaponData;
struct EntityData;
struct LootCrateData;
struct ProjectileData;
struct BombData;
struct InfernoData;
struct SmokeData;

struct Matrix4x4;

namespace GameData
{
    void update() noexcept;
    void clearProjectileList() noexcept;
    void clearTextures() noexcept;
    void clearUnusedAvatars() noexcept;

    const std::string ranks[] = {
        xorstr_("Unranked"),
        xorstr_("Silver I"),
        xorstr_("Silver II"),
        xorstr_("Silver III"),
        xorstr_("Silver IV"),
        xorstr_("Silver Elite"),
        xorstr_("Silver Elite Master"),

        xorstr_("Gold Nova I"),
        xorstr_("Gold Nova II"),
        xorstr_("Gold Nova III"),
        xorstr_("Gold Nova Master"),
        xorstr_("Master Guardian I"),
        xorstr_("Master Guardian II"),
        xorstr_("Master Guardian Elite"),

        xorstr_("Distinguished Master Guardian"),
        xorstr_("Legendary Eagle"),
        xorstr_("Legendary Eagle Master"),
        xorstr_("Supreme Master First Class"),
        xorstr_("The Global Elite")
    };

    const std::string ranks_dz[] = {
        GameData::ranks[0],
        xorstr_("Lab Rat I"),
        xorstr_("Lab Rat II"),

        xorstr_("Sprinting Hare I"),
        xorstr_("Sprinting Hare II"),

        xorstr_("Wild Scout I"),
        xorstr_("Wild Scout II"),
        xorstr_("Wild Scout Elite"),

        xorstr_("Hunter Fox I"),
        xorstr_("Hunter Fox II"),
        xorstr_("Hunter Fox III"),
        xorstr_("Hunter Fox Elite"),

        xorstr_("Timber Wolf"),
        xorstr_("Ember Wolf"),
        xorstr_("Wildfire Wolf"),
        xorstr_("The Howling Alpha")
    };

    class Lock {
    public:
        Lock() noexcept : lock{ mutex } {}
    private:
        std::scoped_lock<std::mutex> lock;
        static inline std::mutex mutex;
    };
    
    // Lock-free
    int getNetOutgoingLatency() noexcept;

    // You have to acquire Lock before using these getters
    const Matrix4x4& toScreenMatrix() noexcept;
    const LocalPlayerData& local() noexcept;
    const std::vector<PlayerData>& players() noexcept;
    const PlayerData* playerByHandle(int handle) noexcept;
    const std::vector<ObserverData>& observers() noexcept;
    const std::vector<WeaponData>& weapons() noexcept;
    const std::vector<EntityData>& entities() noexcept;
    const std::vector<LootCrateData>& lootCrates() noexcept;
    const std::forward_list<ProjectileData>& projectiles() noexcept;
    const BombData& plantedC4() noexcept;
    const std::vector<InfernoData>& infernos() noexcept;
    const std::vector<SmokeData>& smokes() noexcept;
}

struct LocalPlayerData {
    void update() noexcept;

    bool exists = false;
    bool alive = false;
    bool inReload = false;
    bool shooting = false;
    bool noScope = false;
    float nextWeaponAttack = 0.0f;
    int fov;
    int handle;
    float flashDuration;
    Vector aimPunch;
    Vector origin;
};

class Entity;

struct BaseData {
    BaseData(Entity* entity) noexcept;

    float distanceToLocal;
    Vector obbMins, obbMaxs;
    matrix3x4 coordinateFrame;
};

struct EntityData final : BaseData {
    EntityData(Entity* entity) noexcept;
   
    const char* name;
};

struct ProjectileData : BaseData {
    ProjectileData(Entity* projectile) noexcept;

    void update(Entity* projectile) noexcept;

    constexpr auto operator==(int otherHandle) const noexcept
    {
        return handle == otherHandle;
    }

    bool exploded = false;
    bool thrownByLocalPlayer = false;
    bool thrownByEnemy = false;
    int handle;
    const char* name = nullptr;
    std::vector<std::pair<float, Vector>> trajectory;
};

enum class Team;

struct Commends {
    int Friendly;
    int Teacher;
    int Leader;
};

struct PlayerData : BaseData {
    PlayerData(Entity* entity) noexcept;
    PlayerData(const PlayerData&) = delete;
    PlayerData& operator=(const PlayerData&) = delete;
    PlayerData(PlayerData&&) = default;
    PlayerData& operator=(PlayerData&&) = default;

    void update(Entity* entity) noexcept;
    [[nodiscard]] ImTextureID getAvatarTexture() const noexcept;
    [[nodiscard]] float fadingAlpha() const noexcept;

    bool dormant;
    bool enemy = false;
    bool visible = false;
    bool audible;
    bool spotted;
    bool inViewFrustum;
    bool alive;
    bool immune = false;
    float flashDuration;
    float lastContactTime = 0.0f;
    int health;
    int handle;
    Team team;
    std::string name;
    std::string rank;
    int wins;
    int level;
    Commends commends;
    Vector headMins, headMaxs;
    Vector origin;
    std::string activeWeapon;
    std::vector<std::pair<Vector, Vector>> bones;
};

struct WeaponData : BaseData {
    WeaponData(Entity* entity) noexcept;

    int clip;
    int reserveAmmo;
    const char* group = "All";
    const char* name = "All";
    std::string displayName;
};

struct LootCrateData : BaseData {
    LootCrateData(Entity* entity) noexcept;

    const char* name = nullptr;
};

struct ObserverData {
    ObserverData(Entity* entity, Entity* obs, bool targetIsLocalPlayer) noexcept;

    int playerHandle;
    int targetHandle;
    bool targetIsLocalPlayer;
};

struct BombData {
    void update() noexcept;

    float blowTime;
    float timerLength;
    int defuserHandle;
    float defuseCountDown;
    float defuseLength;
    int bombsite;
};

struct InfernoData {
    InfernoData(Entity* inferno) noexcept;

    std::vector<Vector> points;
};


#define SMOKEGRENADE_LIFETIME 17.5f

struct SmokeData {
    SmokeData(const Vector& origin, int handle) noexcept;

    [[nodiscard]] float fadingAlpha() const noexcept;

    Vector origin;
    float explosionTime;
    int handle;
};