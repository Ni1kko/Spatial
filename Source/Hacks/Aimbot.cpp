#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <memory>

#include "../Menu/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../Menu/imgui/imgui_internal.h"

#include "../Hacks/Aimbot.h"
#include "../Config.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h" 
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponId.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/PhysicsSurfaceProps.h"
#include "../SDK/WeaponData.h"
#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../GameData.h"
#include "../Hacks/Misc.h"
#include <Hacks/Movement.h>

struct Cvars {
    ConVar* accelerate;
    ConVar* maxSpeed;
};

static Cvars cvars;

Vector Aimbot::calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept
{
    return ((destination - source).toAngle() - viewAngles).normalize();
}

static bool traceToExit(const Trace& enterTrace, const Vector& start, const Vector& direction, Vector& end, Trace& exitTrace)
{
    bool result = false;
#if defined(_WIN32)
    const auto traceToExitFn = memory->traceToExit;
    __asm {
        push 0
        push 0
        push 0
        push exitTrace
        mov eax, direction
        push [eax]Vector.z
        push [eax]Vector.y
        push [eax]Vector.x
        mov eax, start
        push [eax]Vector.z
        push [eax]Vector.y
        push [eax]Vector.x
        mov edx, enterTrace
        mov ecx, end
        call traceToExitFn
        add esp, 40
        mov result, al
    }
#endif
    return result;
}

static float handleBulletPenetration(SurfaceData* enterSurfaceData, const Trace& enterTrace, const Vector& direction, Vector& result, float penetration, float damage) noexcept
{
    Vector end;
    Trace exitTrace;

    if (!traceToExit(enterTrace, enterTrace.endpos, direction, end, exitTrace))
        return -1.0f;

    SurfaceData* exitSurfaceData = interfaces->physicsSurfaceProps->getSurfaceData(exitTrace.surface.surfaceProps);

    float damageModifier = 0.16f;
    float penetrationModifier = (enterSurfaceData->penetrationmodifier + exitSurfaceData->penetrationmodifier) / 2.0f;

    if (enterSurfaceData->material == 71 || enterSurfaceData->material == 89) {
        damageModifier = 0.05f;
        penetrationModifier = 3.0f;
    } else if (enterTrace.contents >> 3 & 1 || enterTrace.surface.flags >> 7 & 1) {
        penetrationModifier = 1.0f;
    }

    if (enterSurfaceData->material == exitSurfaceData->material) {
        if (exitSurfaceData->material == 85 || exitSurfaceData->material == 87)
            penetrationModifier = 3.0f;
        else if (exitSurfaceData->material == 76)
            penetrationModifier = 2.0f;
    }

    damage -= 11.25f / penetration / penetrationModifier + damage * damageModifier + (exitTrace.endpos - enterTrace.endpos).squareLength() / 24.0f / penetrationModifier;

    result = exitTrace.endpos;
    return damage;
}

static bool canScan(Entity* entity, const Vector& destination, const WeaponInfo* weaponData, int minDamage, bool allowFriendlyFire) noexcept
{
    if (!localPlayer)
        return false;

    float damage{ static_cast<float>(weaponData->damage) };

    Vector start{ localPlayer->getEyePosition() };
    Vector direction{ destination - start };
    direction /= direction.length();

    int hitsLeft = 4;

    while (damage >= 1.0f && hitsLeft) {
        Trace trace;
        interfaces->engineTrace->traceRay({ start, destination }, 0x4600400B, localPlayer.get(), trace);

        if (!allowFriendlyFire && trace.entity && trace.entity->isPlayer() && !localPlayer->isOtherEnemy(trace.entity))
            return false;

        if (trace.fraction == 1.0f)
            break;

        if (trace.entity == entity && trace.hitgroup > HitGroup::Generic && trace.hitgroup <= HitGroup::RightLeg) {
            damage = HitGroup::getDamageMultiplier(trace.hitgroup) * damage * std::pow(weaponData->rangeModifier, trace.fraction * weaponData->range / 500.0f);

            if (float armorRatio{ weaponData->armorRatio / 2.0f }; HitGroup::isArmored(trace.hitgroup, trace.entity->hasHelmet()))
                damage -= (trace.entity->armor() < damage * armorRatio / 2.0f ? trace.entity->armor() * 4.0f : damage) * (1.0f - armorRatio);

            return damage >= minDamage;
        }
        const auto surfaceData = interfaces->physicsSurfaceProps->getSurfaceData(trace.surface.surfaceProps);

        if (surfaceData->penetrationmodifier < 0.1f)
            break;

        damage = handleBulletPenetration(surfaceData, trace, direction, start, weaponData->penetration, damage);
        hitsLeft--;
    }
    return false;
}

static bool keyPressed = false;

void Aimbot::updateInput() noexcept
{
    if (config->aimbotKeyMode == 0)
        keyPressed = config->aimbotKey.isDown();
    if (config->aimbotKeyMode == 1 && config->aimbotKey.isPressed())
        keyPressed = !keyPressed;
}

void Aimbot::run(UserCmd* cmd) noexcept
{
    Vector bestTarget{ };

    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());
    if (!config->aimbot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    //enabled
    if (!config->aimbot[weaponIndex].enabled)
        weaponIndex = 0;

    //between shots
    if (!config->aimbot[weaponIndex].betweenShots && (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime() || (activeWeapon->isFullAuto() && localPlayer->shotsFired() > 1)))
        return;

    //ignore flash
    if (!config->aimbot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->aimbotOnKey && !keyPressed)
        return;

    //weapon config
    if (config->aimbot[weaponIndex].enabled && (cmd->buttons & UserCmd::IN_ATTACK || config->aimbot[weaponIndex].autoShot || config->aimbot[weaponIndex].aimlock) && activeWeapon->getInaccuracy() <= config->aimbot[weaponIndex].maxAimInaccuracy) {

        //scoped only
        if (config->aimbot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
            return;

        auto bestFov = config->aimbot[weaponIndex].fov; 
        const auto localPlayerEyePosition = localPlayer->getEyePosition(); 
        const auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };

        //each target
        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);

            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive() || !entity->isOtherEnemy(localPlayer.get()) && !config->aimbot[weaponIndex].friendlyFire || entity->gunGameImmunity())
                continue;

            for (auto bone : { 8, 4, 3, 7, 6, 5 }) {
                const auto bonePosition = entity->getBonePosition(config->aimbot[weaponIndex].bone > 1 ? 10 - config->aimbot[weaponIndex].bone : bone);
                const auto angle = calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch);
                
                const auto fov = std::hypot(angle.x, angle.y);
                if (fov > bestFov)
                    continue;

                if (!config->aimbot[weaponIndex].ignoreSmoke && memory->lineGoesThroughSmoke(localPlayerEyePosition, bonePosition, 1))
                    continue;

                if (!entity->isVisible(bonePosition) && (config->aimbot[weaponIndex].visibleOnly || !canScan(entity, bonePosition, activeWeapon->getWeaponData(), config->aimbot[weaponIndex].killshot ? entity->health() : config->aimbot[weaponIndex].minDamage, config->aimbot[weaponIndex].friendlyFire)))
                    continue;

                if (fov < bestFov) {
                    bestFov = fov;
                    bestTarget = bonePosition;
                }
                if (config->aimbot[weaponIndex].bone)
                    break;
            }
        }

        if (bestTarget.notNull()) {
            static Vector lastAngles{ cmd->viewangles };
            static int lastCommand{ };

            if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull() && config->aimbot[weaponIndex].silent)
                cmd->viewangles = lastAngles;

            auto angle = calculateRelativeAngle(localPlayerEyePosition, bestTarget, cmd->viewangles + aimPunch);
            bool clamped{ false };

            if (std::abs(angle.x) > Movement::maxAngleDelta() || std::abs(angle.y) > Movement::maxAngleDelta()) {
                    angle.x = std::clamp(angle.x, -Movement::maxAngleDelta(), Movement::maxAngleDelta());
                    angle.y = std::clamp(angle.y, -Movement::maxAngleDelta(), Movement::maxAngleDelta());
                    clamped = true;
            }
            
            angle /= config->aimbot[weaponIndex].smooth;
            cmd->viewangles += angle;
            if (!config->aimbot[weaponIndex].silent)
                interfaces->engine->setViewAngles(cmd->viewangles);

            if (config->aimbot[weaponIndex].autoStop)
                autoStop(cmd);

            if (config->aimbot[weaponIndex].autoScope && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
                cmd->buttons |= UserCmd::IN_ATTACK2;

            if (config->aimbot[weaponIndex].autoShot && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && !clamped && activeWeapon->getInaccuracy() <= config->aimbot[weaponIndex].maxShotInaccuracy)
                cmd->buttons |= UserCmd::IN_ATTACK;

            if (clamped)
                cmd->buttons &= ~UserCmd::IN_ATTACK;

            if (clamped || config->aimbot[weaponIndex].smooth > 1.0f) lastAngles = cmd->viewangles;
            else lastAngles = Vector{ };

            lastCommand = cmd->commandNumber;
        }
    }
}

void Aimbot::autoStop(UserCmd* cmd) noexcept
{
    if (!localPlayer)
        return;

    if (localPlayer->moveType() != MoveType::WALK)
        return;

    Vector velocity = localPlayer->velocity();
    velocity.z = 0;

    float speed = velocity.length2D();

    if (speed < 1.f) {
        cmd->forwardmove = 0.f;
        cmd->sidemove = 0.f;
        return;
    }

    float accel = cvars.accelerate->getFloat();
    float maxSpeed = cvars.maxSpeed->getFloat();

    float playerSurfaceFriction = 1.0f;
    float maxAccelSpeed = accel * memory->globalVars->intervalPerTick * maxSpeed * playerSurfaceFriction;

    float wishSpeed{};

    if (speed - maxAccelSpeed <= -1.f) wishSpeed = maxAccelSpeed / (speed / (accel * memory->globalVars->intervalPerTick));
    else wishSpeed = maxAccelSpeed;

    Vector ndir = (velocity * -1.f).toAngle();

    ndir.y = cmd->viewangles.y - ndir.y;
    ndir = ndir.fromAngle(ndir);

    cmd->forwardmove = ndir.x * wishSpeed;
    cmd->sidemove = ndir.y * wishSpeed;
}

void Aimbot::init() noexcept
{
    cvars.accelerate = interfaces->cvar->findVar("sv_accelerate");
    cvars.maxSpeed = interfaces->cvar->findVar("sv_maxspeed");
}

static bool worldToScreen(const Vector& in, ImVec2& out) noexcept
{
    const auto& matrix = GameData::toScreenMatrix();

    const auto w = matrix._41 * in.x + matrix._42 * in.y + matrix._43 * in.z + matrix._44;
    if (w < 0.001f)
        return false;

    out = ImGui::GetIO().DisplaySize / 2.0f;
    out.x *= 1.0f + (matrix._11 * in.x + matrix._12 * in.y + matrix._13 * in.z + matrix._14) / w;
    out.y *= 1.0f - (matrix._21 * in.x + matrix._22 * in.y + matrix._23 * in.z + matrix._24) / w;
    out = ImFloor(out);
    return true;
}

void Aimbot::drawFov(ImDrawList* drawList) noexcept
{
    if (!config->drawaimbotFov.enabled)
        return;

    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());
    if (!config->aimbot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!config->aimbot[weaponIndex].enabled)
        weaponIndex = 0;

    if (!config->aimbot[weaponIndex].enabled)
        return;

    if (!config->aimbot[weaponIndex].betweenShots && (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime() || (activeWeapon->isFullAuto() && localPlayer->shotsFired() > 1)))
        return;

    if (!config->aimbot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->aimbot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
        return;

    const auto& screensize = ImGui::GetIO().DisplaySize;
    float radius = std::tan(Helpers::deg2rad(config->aimbot[weaponIndex].fov) / 2.f) / std::tan(Helpers::deg2rad(localPlayer->isScoped() ? localPlayer->fov() : config->totalFov) / 2.f) * screensize.x;
    const ImVec2 screen_mid = { screensize.x / 2.0f, screensize.y / 2.0f };

    const auto aimPunchAngle = localPlayer->getEyePosition() + Vector::fromAngle(interfaces->engine->getViewAngles() + localPlayer->getAimPunch()) * 1000.0f;

    if (ImVec2 pos; worldToScreen(aimPunchAngle, pos))
        drawList->AddCircle(localPlayer->shotsFired() > 1 ? pos : screen_mid, radius, Helpers::calculateColor(config->drawaimbotFov.asColor4()), 360);
}