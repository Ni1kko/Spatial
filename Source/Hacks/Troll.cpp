#include <algorithm>
#include <array>
#include <iomanip>
#include <mutex>
#include <numbers>
#include <numeric>
#include <sstream>
#include <vector>

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../ConfigStructs.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../ProtobufReader.h"

#include "EnginePrediction.h"
#include "Troll.h"

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

#include "../GUI.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../GameData.h"

#include "../imguiCustom.h"


/////////////////////////////////////////////////////////////////
// Vars
/////////////////////////////////////////////////////////////////

static bool windowOpen = false;

/////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////

struct TrollConfig {

    bool blockbot { false };
    KeyBind blockbotKey{ KeyBind::V };
    bool doorSpam { false }; 
    float doorSpamRange { 0.f };
} trollConfig;


/////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////

void Troll::blockbot(UserCmd* cmd) noexcept
{
    if (!trollConfig.blockbot || !trollConfig.blockbotKey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    float bestDistance = 200.0f;
    int plyIndex = -1;

    for (int i = 1; i < interfaces->engine->getMaxClients(); i++)
    {
        Entity* ply = interfaces->entityList->getEntity(i);

        if (!ply)
            continue;

        if (!ply->isAlive() || ply->isDormant() || ply == localPlayer.get())
            continue;

        float distance = localPlayer->origin().distTo(ply->origin());

        if (distance < bestDistance)
        {
            bestDistance = distance;
            plyIndex = i;
        }
    }

    if (plyIndex == -1)
        return;

    Entity* target = interfaces->entityList->getEntity(plyIndex);

    if (!target)
        return;

    if (localPlayer->origin().z - target->origin().z > 20)
    {
        Vector vecForward = target->origin() - localPlayer->origin();

        cmd->forwardmove = ((sin(Helpers::deg2rad(cmd->viewangles.y)) * vecForward.y) + (cos(Helpers::deg2rad(cmd->viewangles.y)) * vecForward.x)) * 450.0f;
        cmd->sidemove = ((cos(Helpers::deg2rad(cmd->viewangles.y)) * -vecForward.y) + (sin(Helpers::deg2rad(cmd->viewangles.y)) * vecForward.x)) * 450.0f;
    }
    else {
        Vector angles = Helpers::calculateRelativeAngle(localPlayer->origin(), target->origin());

        angles.y = angles.y - localPlayer->eyeAngles().y;
        angles.normalize();

        if (angles.y < 0.0f)
            cmd->sidemove = 450.0f;
        else if (angles.y > 0.0f)
            cmd->sidemove = -450.0f;
    }
}

void Troll::doorSpam(UserCmd* cmd) noexcept
{
    if (!trollConfig.doorSpam || !localPlayer || localPlayer->isDefusing()) {
        return;
    }

    Trace trace;
    TraceFilter traceFilter{ localPlayer.get() };
    traceFilter.skip = localPlayer.get();

    const auto startPos = localPlayer->getEyePosition();
    const auto endPos = startPos + Vector::fromAngle(cmd->viewangles) * trollConfig.doorSpamRange;
    interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, traceFilter, trace);

    if (trace.entity && trace.entity->getClientClass()->classId == ClassId::PropDoorRotating) {
        if (cmd->buttons & UserCmd::IN_USE && cmd->tickCount & 1) {
            cmd->buttons &= ~UserCmd::IN_USE;
        }
    }
}

/////////////////////////////////////////////////////////////////
// GUI Functions
/////////////////////////////////////////////////////////////////

void Troll::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Troll")) {
        windowOpen = true;
        ImGui::SetWindowFocus("Troll");
        ImGui::SetWindowPos("Troll", { 100.0f, 100.0f });
    }
}

void Troll::tabItem() noexcept
{
    if (ImGui::BeginTabItem("Troll")) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Troll::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 580.0f, 0.0f });
        ImGui::Begin("Troll", &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    //col 1
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 230.0f); 
    ImGui::Checkbox("Block Bot", &trollConfig.blockbot);
    ImGui::SameLine();
    ImGui::PushID("Block Bot Key");
    ImGui::hotkey("", trollConfig.blockbotKey);
    ImGui::PopID();

    //col 2
    ImGui::NextColumn();
    ImGui::Checkbox("Door spam", &trollConfig.doorSpam);
    ImGui::SameLine();
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderFloat("Range", &trollConfig.doorSpamRange, 0, 500, "%.0f meters");
    ImGui::PopItemWidth();
    if (!contentOnly)
        ImGui::End();
}

/////////////////////////////////////////////////////////////////
// Config Functions
/////////////////////////////////////////////////////////////////

static void from_json(const json& j, TrollConfig& m)
{
    read(j, "Block Bot", m.blockbot);
    read(j, "Slowwalk key", m.blockbotKey);
    read(j, "Door spam", m.doorSpam);
    read(j, "Door spam range", m.doorSpamRange);
}

static void to_json(json& j, const TrollConfig& o)
{
    const TrollConfig dummy;
    WRITE("Block Bot", blockbot);
    WRITE("Block Bot Key", blockbotKey);
    WRITE("Door spam", doorSpam);
    WRITE("Door spam range", doorSpamRange);
}

json Troll::toJson() noexcept
{
    json j;
    to_json(j, trollConfig);
    return j;
}

void Troll::fromJson(const json& j) noexcept
{
    from_json(j, trollConfig);
}

void Troll::resetConfig() noexcept
{
    trollConfig = {};
}