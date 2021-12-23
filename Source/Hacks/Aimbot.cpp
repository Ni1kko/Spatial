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
#include <Menu/imguiCustom.h>

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
    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack()) return;

    // get weapon in use
    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon) return;
    
    // reloading or out of bullets
    if (!activeWeapon->clip()) return;

    // semi-auto, bolt-action, burst-fire or single fire
    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto()) return;

    // get weapon index
    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex) return;

    // get weapon class
    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());
     
    // aimbot config
    const auto& aCfg = config->aimbot;

    // weapon config
    auto wCfg = aCfg[weaponIndex];

    //enable toggle
    if (!wCfg.enabled) wCfg = aCfg[weaponClass]; 
    if (!wCfg.enabled) wCfg = aCfg[0];

    //between shots
    if (!wCfg.betweenShots && (activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime() || (activeWeapon->isFullAuto() && localPlayer->shotsFired() > 1))) return;

    //ignore flash
    if (!wCfg.ignoreFlash && localPlayer->isFlashed()) return;

    //on key
    if (config->aimbotOnKey && !keyPressed) return;

    //weapon config
    if (wCfg.enabled && (cmd->buttons & UserCmd::IN_ATTACK || wCfg.autoShot || wCfg.aimlock) && activeWeapon->getInaccuracy() <= wCfg.maxAimInaccuracy)
    {
        Vector bestTarget{ };
        auto bestFov = wCfg.fov;
        const auto localPlayerEyePosition = localPlayer->getEyePosition();
        const auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };

        //scoped only
        if (wCfg.scopedOnly && !wCfg.autoScope && activeWeapon->hasScope() && (!localPlayer->isScoped() || !activeWeapon->zoomLevel()))
            return;
        
        //each target
        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);

            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive() || !entity->isOtherEnemy(localPlayer.get()) && !wCfg.friendlyFire || entity->gunGameImmunity())
                continue;

            for (auto bone : { 8, 4, 3, 7, 6, 5 }) {
                const auto bonePosition = entity->getBonePosition(wCfg.bone > 1 ? 10 - wCfg.bone : bone);
                const auto angle = calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch);
                
                const auto fov = std::hypot(angle.x, angle.y);
                if (fov > bestFov)
                    continue;

                if (!wCfg.ignoreSmoke && memory->lineGoesThroughSmoke(localPlayerEyePosition, bonePosition, 1))
                    continue;

                const bool visible = entity->isVisible(bonePosition);

                if (!visible && (wCfg.visibleOnly || !canScan(entity, bonePosition, activeWeapon->getWeaponData(), wCfg.killshot ? entity->health() : wCfg.minDamage, wCfg.friendlyFire)))
                    continue;

                if (fov < bestFov) {
                    bestFov = fov;
                    bestTarget = bonePosition;

                    if (wCfg.autoStop && visible)
                        autoStop(cmd);
                }
                if (wCfg.bone)
                    break;
            }
        }

        //no target
        if (bestTarget.null()) 
            return;

        // auto Scope
        if (wCfg.autoScope)
            autoScope(cmd);

        static Vector lastAngles{ cmd->viewangles };
        static int lastCommand{ };
        bool clamped{ false };

        //Silent Aim
        if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull() && wCfg.silent)
            cmd->viewangles = lastAngles;
           
        //Current Calculated Angle
        auto angle = calculateRelativeAngle(localPlayerEyePosition, bestTarget, cmd->viewangles + aimPunch);
            
        //Max Angle Delta
        if (std::abs(angle.x) > movement->maxAngleDelta() || std::abs(angle.y) > movement->maxAngleDelta()) {
                angle.x = std::clamp(angle.x, -movement->maxAngleDelta(), movement->maxAngleDelta());
                angle.y = std::clamp(angle.y, -movement->maxAngleDelta(), movement->maxAngleDelta());
                clamped = true;
        }
            
        //Add smoth aim to view angle
        angle /= wCfg.smooth;

        //Adjust View angles
        cmd->viewangles += angle;
            
        //Set angles too aim at target
        if (!wCfg.silent)
            interfaces->engine->setViewAngles(cmd->viewangles);
            
        //Auto Attack
        if (wCfg.autoShot && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && !clamped && activeWeapon->getInaccuracy() <= wCfg.maxShotInaccuracy)
            cmd->buttons |= UserCmd::IN_ATTACK;

        //Cancel attack
        if (clamped)
            cmd->buttons &= ~UserCmd::IN_ATTACK;

        //Save Last Angles
        if (clamped || wCfg.smooth > 1.0f) lastAngles = cmd->viewangles;
        else lastAngles = Vector{ };

        //Save Command Number
        lastCommand = cmd->commandNumber;
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

void Aimbot::autoScope(UserCmd* cmd) noexcept
{
    if (!localPlayer) return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon) return;

    if (activeWeapon->hasScope() && (!localPlayer->isScoped() || !activeWeapon->zoomLevel()))
        cmd->buttons |= UserCmd::IN_ZOOM;
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

void Aimbot::drawGUI() noexcept
{
    ImGui::Checkbox("On key", &config->aimbotOnKey);
    ImGui::SameLine();
    ImGui::PushID("Aimbot Key");
    ImGui::hotkey("", config->aimbotKey);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushItemWidth(70.0f);
    ImGui::Combo("", &config->aimbotKeyMode, "Hold\0Toggle\0");
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::SameLine();
    ImGuiCustom::colorPicker("Draw AimBot FOV", config->drawaimbotFov);
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->aimbot[currentWeapon].enabled);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 220.0f);
    ImGui::Checkbox("Aimlock", &config->aimbot[currentWeapon].aimlock);
    ImGui::Checkbox("Silent", &config->aimbot[currentWeapon].silent);
    ImGui::Checkbox("Friendly fire", &config->aimbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Visible only", &config->aimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->aimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->aimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->aimbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Auto shot", &config->aimbot[currentWeapon].autoShot);
    ImGui::Checkbox("Auto scope", &config->aimbot[currentWeapon].autoScope);
    ImGui::Checkbox("Auto stop", &config->aimbot[currentWeapon].autoStop);
    ImGui::Combo("Bone", &config->aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(240.0f);
    ImGui::SliderFloat("Fov", &config->aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Smooth", &config->aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Max aim inaccuracy", &config->aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max shot inaccuracy", &config->aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::InputInt("Min damage", &config->aimbot[currentWeapon].minDamage);
    config->aimbot[currentWeapon].minDamage = std::clamp(config->aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Between shots", &config->aimbot[currentWeapon].betweenShots);
    ImGui::Columns(1);
}