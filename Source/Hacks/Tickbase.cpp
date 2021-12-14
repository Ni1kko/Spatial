#include "Backtrack.h"
#include "Tickbase.h"

#include "../imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../Menu/imguiCustom.h"

#include <Encryption/xorstr.hpp>

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
#include "../SDK/NetworkChannel.h"
#include "../GameData.h"
#include "../Hacks/Misc.h"

/////////////////////////////////////////////////////////////////
// Vars
/////////////////////////////////////////////////////////////////

static bool windowOpen = false;
static bool isDoubleTapping = false; 

/////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////

struct NetConfig {
	bool enabled_dt{ false };
	bool onkey_dt{ false };
    int speed_dt{ 0 };
    KeyBind key_dt;
    bool enabled_cp{ false };
    int cp { 0 };
    bool onkey_cp{ false };
    KeyBind key_cp;
} netConfig;


/////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////


bool canShift(int ticks)
{ 
    if (ticks <= 0 || localPlayer->nextAttack() > memory->globalVars->serverTime())
        return false;

    float nextAttack = (localPlayer->nextAttack() + (ticks * memory->globalVars->intervalPerTick));
    if (nextAttack >= memory->globalVars->serverTime())
        return false;

    auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip() || activeWeapon->isGrenade())
        return false;

    if (activeWeapon->isKnife() || activeWeapon->isGrenade()
        || activeWeapon->ID() == WeaponId::Revolver
        || activeWeapon->ID() == WeaponId::Awp
        || activeWeapon->ID() == WeaponId::Ssg08
        || activeWeapon->ID() == WeaponId::Taser
        || activeWeapon->ID() == WeaponId::Revolver)
        return false;

    float shiftTime = (localPlayer->tickBase() - ticks) * memory->globalVars->intervalPerTick;

    if (shiftTime < activeWeapon->nextPrimaryAttack())
        return false;

    return true;
}
  
bool Tickbase::DTKeyDown() noexcept
{
    return (netConfig.key_dt.isSet() && netConfig.key_dt.isDown());
}

bool Tickbase::CPKeyDown() noexcept
{
    return (netConfig.onkey_cp && netConfig.key_cp.isSet() && netConfig.key_cp.isDown());
}

void Tickbase::run(UserCmd* cmd) noexcept
{
    auto enabled = netConfig.enabled_dt || DTKeyDown();
    auto alive = localPlayer && localPlayer->isAlive();
    auto alwaysRun = !netConfig.onkey_dt;
    auto ticks = 0;

    if (alive && (enabled && alwaysRun || enabled && !alwaysRun && DTKeyDown()))
    {
        switch (netConfig.speed_dt) 
        {
            case 0: //Instant
                ticks = 17;
                break;
            case 1: //Fast
                ticks = 14;
                break;
            case 2: //Accurate
                ticks = 13;
                break;
        }
        
        if (ticks > 0 && (cmd->buttons == UserCmd::IN_ATTACK || DTKeyDown()))
        { 
            isDoubleTapping = true;
            cmd->tickCount = ticks; 
            isDoubleTapping = false;
        }
        
        cmd->tickCount = 0;
    }
}

void Tickbase::chokePackets(bool& sendPacket) noexcept
{
    if (localPlayer && (netConfig.enabled_cp && !netConfig.onkey_cp || netConfig.enabled_cp && netConfig.onkey_cp && CPKeyDown()) || isDoubleTapping)
        sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= netConfig.cp;
}

/////////////////////////////////////////////////////////////////
// GUI Functions
/////////////////////////////////////////////////////////////////

void Tickbase::menuBarItem() noexcept
{
    if (ImGui::MenuItem(xorstr_("Network"))) {
        windowOpen = true;
        ImGui::SetWindowFocus(xorstr_("Network"));
        ImGui::SetWindowPos(xorstr_("Network"), { 100.0f, 100.0f });
    }
}

void Tickbase::tabItem() noexcept
{
    if (ImGui::BeginTabItem(xorstr_("Network"))) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Tickbase::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 580.0f, 0.0f });
        ImGui::Begin(xorstr_("Network"), &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    //col 1
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 300.0f);
    ImGui::Checkbox("Enable Double TAP", &netConfig.enabled_dt); 
    ImGui::Checkbox("Double TAP Key", &netConfig.onkey_dt);
    ImGui::SameLine();
    ImGui::PushID("DTKey");
    ImGui::hotkey("", netConfig.key_dt);
    ImGui::PopID(); 
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID("DTSpeed");
    ImGui::Combo("", &netConfig.speed_dt, "Instant\0Fast\0Accurate\0");
    ImGui::PopID();
    
    //col 2
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(90.0f);
    ImGui::Checkbox("Enable Choked Packets", &netConfig.enabled_cp);
    ImGui::Checkbox("Choked Packets Key", &netConfig.onkey_cp);
    ImGui::SameLine();
    ImGui::PushID("CPKey");
    ImGui::hotkey("", netConfig.key_cp);
    ImGui::PopID();
    ImGui::SetNextItemWidth(90.0f);
    ImGui::InputInt("Choked packets", &netConfig.cp, 1, 5);
    netConfig.cp = std::clamp(netConfig.cp, 0, 64);
    ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}

/////////////////////////////////////////////////////////////////
// Config Functions
/////////////////////////////////////////////////////////////////

static void from_json(const json& j, NetConfig& m)
{
    read(j, xorstr_("Double tap"), m.enabled_dt);
    read(j, xorstr_("Double tap onkey"), m.onkey_dt);
    read(j, xorstr_("Double tap speed"), m.speed_dt);
    read(j, xorstr_("Double tap key"), m.key_dt);
    read(j, xorstr_("Choke packets"), m.enabled_cp);
    read(j, xorstr_("Choked packets"), m.cp);
    read(j, xorstr_("Choke packets onkey"), m.onkey_cp);
    read(j, xorstr_("Choke packets key"), m.key_cp);
}

static void to_json(json& j, const NetConfig& o)
{
    const NetConfig dummy;
    WRITE(xorstr_("Double tap"), enabled_dt);
    WRITE(xorstr_("Double tap onkey"), onkey_dt);
    WRITE(xorstr_("Double tap speed"), speed_dt);
    WRITE(xorstr_("Double tap key"), key_dt);
    WRITE(xorstr_("Choke packets"), enabled_cp);
    WRITE(xorstr_("Choked packets"), cp);
    WRITE(xorstr_("Choke packets onkey"), onkey_cp);
    WRITE(xorstr_("Choke packets key"), key_cp);
}

json Tickbase::toJson() noexcept
{
    json j;
    to_json(j, netConfig);
    return j;
}

void Tickbase::fromJson(const json& j) noexcept
{
    from_json(j, netConfig);
}

void Tickbase::resetConfig() noexcept
{
    netConfig = {};
}