#include <algorithm>
#include <array>
#include <iomanip>
#include <mutex>
#include <numbers>
#include <numeric>
#include <sstream>
#include <vector>
#include <string>

#include <Encryption/xorstr.hpp>
#include <Encryption/cx_strenc.h>

#include "../Menu/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../Menu/imgui/imgui_internal.h"
#include "../Menu/imgui/imgui_stdlib.h"
#include "../Menu/imguiCustom.h"
#include "../Menu/Menu.h"

#include "../ConfigStructs.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../ProtobufReader.h"

#include "EnginePrediction.h"

#include "../SDK/ClassId.h"
#include "../SDK/Client.h"
#include "../SDK/ClientClass.h"
#include "../SDK/ClientMode.h"
#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/Localize.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/Panorama.h"
#include "../SDK/Platform.h"
#include "../SDK/UserCmd.h"
#include "../SDK/UtlVector.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponId.h"
#include "../SDK/WeaponSystem.h"

#include "../SDK/Steam.h"

#include "../Helpers.h"
#include "../Hooks.h"
#include "../GameData.h"

#include "Movement.h"


/////////////////////////////////////////////////////////////////
// Vars
/////////////////////////////////////////////////////////////////

static bool windowOpen = false;


/////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////
void Movement::fixMouseDelta(UserCmd* cmd) noexcept
{
    if (!cmd || !movementConfig.fixMouseDelta) return;

    static Vector delta_viewangles{ };
    Vector delta = cmd->viewangles - delta_viewangles;

    delta.x = std::clamp(delta.x, -89.0f, 89.0f);
    delta.y = std::clamp(delta.y, -180.0f, 180.0f);
    delta.z = 0.0f;
    static ConVar* sensitivity;
    if (!sensitivity)
        sensitivity = interfaces->cvar->findVar("sensitivity");;
    if (delta.x != 0.f) {
        static ConVar* m_pitch;

        if (!m_pitch)
            m_pitch = interfaces->cvar->findVar("m_pitch");

        int final_dy = static_cast<int>((delta.x / m_pitch->getFloat()) / sensitivity->getFloat());
        if (final_dy <= 32767) {
            if (final_dy >= -32768) {
                if (final_dy >= 1 || final_dy < 0) {
                    if (final_dy <= -1 || final_dy > 0)
                        final_dy = final_dy;
                    else
                        final_dy = -1;
                }
                else {
                    final_dy = 1;
                }
            }
            else {
                final_dy = 32768;
            }
        }
        else {
            final_dy = 32767;
        }

        cmd->mousedy = static_cast<short>(final_dy);
    }

    if (delta.y != 0.f) {
        static ConVar* m_yaw;

        if (!m_yaw)
            m_yaw = interfaces->cvar->findVar("m_yaw");

        int final_dx = static_cast<int>((delta.y / m_yaw->getFloat()) / sensitivity->getFloat());
        if (final_dx <= 32767) {
            if (final_dx >= -32768) {
                if (final_dx >= 1 || final_dx < 0) {
                    if (final_dx <= -1 || final_dx > 0)
                        final_dx = final_dx;
                    else
                        final_dx = -1;
                }
                else {
                    final_dx = 1;
                }
            }
            else {
                final_dx = 32768;
            }
        }
        else {
            final_dx = 32767;
        }

        cmd->mousedx = static_cast<short>(final_dx);
    }

    delta_viewangles = cmd->viewangles;
}

void Movement::fixMovement(UserCmd* cmd, float yaw) noexcept
{
    if (!cmd || !movementConfig.fixMouseDelta) return;
    float oldYaw = yaw + (yaw < 0.0f ? 360.0f : 0.0f);
    float newYaw = cmd->viewangles.y + (cmd->viewangles.y < 0.0f ? 360.0f : 0.0f);
    float yawDelta = newYaw < oldYaw ? fabsf(newYaw - oldYaw) : 360.0f - fabsf(newYaw - oldYaw);
    yawDelta = 360.0f - yawDelta;

    const float forwardmove = cmd->forwardmove;
    const float sidemove = cmd->sidemove;
    cmd->forwardmove = std::cos(Helpers::deg2rad(yawDelta)) * forwardmove + std::cos(Helpers::deg2rad(yawDelta + 90.0f)) * sidemove;
    cmd->sidemove = std::sin(Helpers::deg2rad(yawDelta)) * forwardmove + std::sin(Helpers::deg2rad(yawDelta + 90.0f)) * sidemove;
}

bool Movement::fixBoneMatrix() noexcept
{
    return movementConfig.fixBoneMatrix;
}

void Movement::autoStrafe(UserCmd* cmd) noexcept
{
    if (localPlayer
        && movementConfig.autoStrafe
        && !(localPlayer->flags() & 1)
        && localPlayer->moveType() != MoveType::NOCLIP) {
        if (cmd->mousedx < 0)
            cmd->sidemove = -450.0f;
        else if (cmd->mousedx > 0)
            cmd->sidemove = 450.0f;
    }
}

void Movement::fastStop(UserCmd* cmd) noexcept
{
    if (!movementConfig.fastStop)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (localPlayer->moveType() == MoveType::NOCLIP || localPlayer->moveType() == MoveType::LADDER || !(localPlayer->flags() & 1) || cmd->buttons & UserCmd::IN_JUMP)
        return;

    if (cmd->buttons & (UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT | UserCmd::IN_FORWARD | UserCmd::IN_BACK))
        return;

    const auto velocity = localPlayer->velocity();
    const auto speed = velocity.length2D();
    if (speed < 15.0f)
        return;

    Vector direction = velocity.toAngle();
    direction.y = cmd->viewangles.y - direction.y;

    const auto negatedDirection = Vector::fromAngle(direction) * -speed;
    cmd->forwardmove = negatedDirection.x;
    cmd->sidemove = negatedDirection.y;
}

void Movement::moonwalk(UserCmd* cmd) noexcept
{
    if (movementConfig.moonwalk && localPlayer && localPlayer->moveType() != MoveType::LADDER)
        cmd->buttons ^= UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT;
}

void Movement::fastCrouch(UserCmd* cmd) noexcept
{
    if (movementConfig.fastCrouch)
        cmd->buttons |= UserCmd::IN_BULLRUSH;
}

void Movement::bunnyHop(UserCmd* cmd) noexcept
{
    if (!localPlayer) return;

    static auto wasLastTimeOnGround{ localPlayer->flags() & 1 };

    if (movementConfig.bunnyHop && !(localPlayer->flags() & 1) && localPlayer->moveType() != MoveType::LADDER && !wasLastTimeOnGround)
        cmd->buttons &= ~UserCmd::IN_JUMP;

    wasLastTimeOnGround = localPlayer->flags() & 1;
}

void Movement::edgejump(UserCmd* cmd) noexcept
{
    if (!movementConfig.edgejump || !movementConfig.edgejumpkey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    if ((EnginePrediction::getFlags() & 1) && !(localPlayer->flags() & 1))
        cmd->buttons |= UserCmd::IN_JUMP;
}

void Movement::slowwalk(UserCmd* cmd) noexcept
{
    if (!movementConfig.slowwalk || !movementConfig.slowwalkKey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    const float maxSpeed = (localPlayer->isScoped() ? weaponData->maxSpeedAlt : weaponData->maxSpeed) / 3;

    if (cmd->forwardmove && cmd->sidemove) {
        const float maxSpeedRoot = maxSpeed * static_cast<float>(M_SQRT1_2);
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
    }
    else if (cmd->forwardmove) {
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeed : maxSpeed;
    }
    else if (cmd->sidemove) {
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeed : maxSpeed;
    }
}

float Movement::maxAngleDelta() noexcept
{
    return movementConfig.setMaxAngleDelta ? movementConfig.maxAngleDelta : Movement::defaultMaxAngleDelta;
}

void Movement::fastPlant(UserCmd* cmd) noexcept
{
    if (!movementConfig.fastPlant)
        return;

    if (static auto plantAnywhere = interfaces->cvar->findVar("mp_plant_c4_anywhere"); plantAnywhere->getInt())
        return;

    if (!localPlayer || !localPlayer->isAlive() || (localPlayer->inBombZone() && localPlayer->flags() & 1))
        return;

    if (const auto activeWeapon = localPlayer->getActiveWeapon(); !activeWeapon || activeWeapon->getClientClass()->classId != ClassId::CC4)
        return;

    cmd->buttons &= ~UserCmd::IN_ATTACK;

    constexpr auto doorRange = 200.0f;

    Trace trace;
    const auto startPos = localPlayer->getEyePosition();
    const auto endPos = startPos + Vector::fromAngle(cmd->viewangles) * doorRange;
    interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, localPlayer.get(), trace);

    if (!trace.entity || trace.entity->getClientClass()->classId != ClassId::CPropDoorRotating)
        cmd->buttons &= ~UserCmd::IN_USE;
}

void Movement::autoPeek(UserCmd* cmd, Vector currentViewAngles) noexcept
{
    
    if (!movementConfig.autoPeek || !localPlayer || !localPlayer->isAlive() || !movementConfig.autoPeekKey.isSet() || !movementConfig.autoPeekKey.isDown())
    {

        Movement::AutoPeekHasShot = false;
        Movement::AutoPeekPosition = Vector{};
        return;
    }

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP || !(localPlayer->flags() & 1))
        return;

    if (movementConfig.autoPeekKey.isDown())
    {
        if (Movement::AutoPeekPosition.null())
            Movement::AutoPeekPosition = localPlayer->getRenderOrigin();

        if (cmd->buttons & UserCmd::IN_ATTACK)
            Movement::AutoPeekHasShot = true;

        if (Movement::AutoPeekHasShot)
        {
            const float yaw = currentViewAngles.y;
            const auto difference = localPlayer->getRenderOrigin() - Movement::AutoPeekPosition;

            if (difference.length2D() > 5.0f)
            {
                const auto velocity = Vector{
                    difference.x * std::cos(yaw / 180.0f * 3.141592654f) + difference.y * std::sin(yaw / 180.0f * 3.141592654f),
                    difference.y * std::cos(yaw / 180.0f * 3.141592654f) - difference.x * std::sin(yaw / 180.0f * 3.141592654f),
                    difference.z };

                cmd->forwardmove = -velocity.x * 20.f;
                cmd->sidemove = velocity.y * 20.f;
            }
            else
            {
                Movement::AutoPeekHasShot = false;
                Movement::AutoPeekPosition = Vector{};
                movementConfig.autoPeekKey.setToggle(false);
            }
        }
    }
    else
    {
        Movement::AutoPeekHasShot = false;
        Movement::AutoPeekPosition = Vector{};
    }
}

/////////////////////////////////////////////////////////////////
// GUI Functions
/////////////////////////////////////////////////////////////////

void Movement::menuBarItem() noexcept
{
    if (ImGui::MenuItem(xorstr_("Troll"))) {
        windowOpen = true;
        ImGui::SetWindowFocus(xorstr_("Troll"));
        ImGui::SetWindowPos(xorstr_("Troll"), { 100.0f, 100.0f });
    }
}

void Movement::tabItem() noexcept
{
    if (ImGui::BeginTabItem(xorstr_("Troll"))) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Movement::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen) return;
        ImGui::SetNextWindowSize({ 580.0f, 0.0f });
        ImGui::Begin(xorstr_("Movement"), &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    //col 1
    ImGui::Checkbox(xorstr_("Fix Mouse Delta"), &movementConfig.fixMouseDelta);
    ImGui::Checkbox(xorstr_("Fix Bone Matrix"), &movementConfig.fixBoneMatrix);
    ImGui::Checkbox(xorstr_("Fix Movement"), &movementConfig.fixMovement);
    ImGui::Checkbox(xorstr_("Custom Angle Delta"), &movementConfig.setMaxAngleDelta);
    if (movementConfig.setMaxAngleDelta)
    {
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderFloat(xorstr_("Max Angle Delta"), &movementConfig.maxAngleDelta, 0.0f, 255.0f, "%.2f");
    }
    
    ImGui::Checkbox(xorstr_("Auto Strafe"), &movementConfig.autoStrafe);
    ImGui::Checkbox(xorstr_("Auto Peek"), &movementConfig.autoPeek);
    if (movementConfig.autoPeek) 
    {
        ImGui::SameLine();
        ImGui::PushID(xorstr_("Auto Peek Key"));
        ImGui::hotkey("", movementConfig.autoPeekKey);
        ImGui::PopID();
    }

    ImGui::Checkbox(xorstr_("Fast Stop"), &movementConfig.fastStop);
    ImGui::Checkbox(xorstr_("Fast plant"), &movementConfig.fastPlant);
    ImGui::Checkbox(xorstr_("Fast Crouch"), &movementConfig.fastCrouch);
    ImGui::Checkbox(xorstr_("Moon Walk"), &movementConfig.moonwalk);
    ImGui::Checkbox(xorstr_("Bunny hop"), &movementConfig.bunnyHop);
    
    ImGui::Checkbox(xorstr_("Edge Jump"), &movementConfig.edgejump);
    if (movementConfig.edgejump) {
        ImGui::SameLine();
        ImGui::PushID(xorstr_("Edge Jump Key"));
        ImGui::hotkey("", movementConfig.edgejumpkey);
        ImGui::PopID();
    }
    
    ImGui::Checkbox(xorstr_("Slowwalk"), &movementConfig.slowwalk);
    if (movementConfig.slowwalk) {
        ImGui::SameLine();
        ImGui::PushID(xorstr_("Slowwalk Key"));
        ImGui::hotkey("", movementConfig.slowwalkKey);
        ImGui::PopID();
    }
    

    if (!contentOnly)
        ImGui::End();
}

/////////////////////////////////////////////////////////////////
// Config Functions
/////////////////////////////////////////////////////////////////

static void from_json(const json& j, MovementConfig& m)
{
    read(j, "Fix MouseDelta", m.fixMouseDelta);
    read(j, "Fix bone Matrix", m.fixBoneMatrix);
    read(j, "Fix Movement", m.fixMovement);
    read(j, "Auto Strafe", m.autoStrafe);
    read(j, "Fast Stop", m.fastStop);
    read(j, "Moon Walk", m.moonwalk);
    read(j, "Fast Crouch", m.fastCrouch);
    read(j, "Bunny Hop", m.bunnyHop);
    read(j, "Edge Jump", m.edgejump);
    read(j, "Edge Jump Key", m.edgejumpkey);
    read(j, "Slow Walk", m.slowwalk);
    read(j, "Slow Walk Key", m.slowwalkKey);
    read(j, "Set Max angle delta", m.setMaxAngleDelta);
    read(j, "Max angle delta", m.maxAngleDelta); 
    read(j, "Auto Peek Key", m.autoPeek);
    read(j, "Auto Peek Key", m.autoPeekKey);
}

static void to_json(json& j, const MovementConfig& o)
{
    const MovementConfig dummy;
    WRITE("Fix MouseDelta", fixMouseDelta);
    WRITE("Fix Bone Matrix", fixBoneMatrix);
    WRITE("Fix Movement", fixMovement);
    WRITE("Auto Strafe", autoStrafe);
    WRITE("Fast Stop", fastStop);
    WRITE("Moon Walk", moonwalk);
    WRITE("Fast Crouch", fastCrouch);
    WRITE("Bunny Hop", bunnyHop);
    WRITE("Edge Jump", edgejump);
    WRITE("Edge Jump Key", edgejumpkey);
    WRITE("Slow Walk", slowwalk);
    WRITE("Slow Walk Key", slowwalkKey);
    WRITE("Set Max angle delta", setMaxAngleDelta);
    WRITE("Max angle delta", maxAngleDelta);
    WRITE("Fast plant", fastPlant);
    WRITE("Auto Peek Key", autoPeek);
    WRITE("Auto Peek Key", autoPeekKey);
}

json Movement::toJson() noexcept
{
    json j;
    to_json(j, movementConfig);
    return j;
}

void Movement::fromJson(const json& j) noexcept
{
    from_json(j, movementConfig);
}

void Movement::resetConfig() noexcept
{
    movementConfig = {};
}