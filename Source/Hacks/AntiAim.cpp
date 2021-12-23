#include <cmath>

#include "AntiAim.h"

#include "../Menu/imgui/imgui.h"

#include "../ConfigStructs.h"
#include "../SDK/Entity.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"

#if Spatial_ANTIAIM()

/////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////

struct AntiAimConfig {
    bool enabled = false;
    bool pitch = false;
    bool yaw = false;
    float pitchAngle = 0.0f;
} antiAimConfig;

/////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////

void AntiAim::run(UserCmd* cmd, const Vector& previousViewAngles, const Vector& currentViewAngles, bool& sendPacket) noexcept
{
    if (antiAimConfig.enabled) {
        if (!localPlayer)
            return;

        if (antiAimConfig.pitch && cmd->viewangles.x == currentViewAngles.x)
            cmd->viewangles.x = antiAimConfig.pitchAngle;

        if (antiAimConfig.yaw && !sendPacket && cmd->viewangles.y == currentViewAngles.y) {
            cmd->viewangles.y += localPlayer->getMaxDesyncAngle();
            if (std::abs(cmd->sidemove) < 5.0f) {
                if (cmd->buttons & UserCmd::IN_DUCK)
                    cmd->sidemove = cmd->tickCount & 1 ? 3.25f : -3.25f;
                else
                    cmd->sidemove = cmd->tickCount & 1 ? 1.1f : -1.1f;
            }
        }
    }
}


/////////////////////////////////////////////////////////////////
// GUI Functions
/////////////////////////////////////////////////////////////////

void AntiAim::drawGUI() noexcept
{
    ImGui::Checkbox("Enabled", &antiAimConfig.enabled);
    ImGui::Checkbox("##pitch", &antiAimConfig.pitch);
    ImGui::SameLine();
    ImGui::SliderFloat("Pitch", &antiAimConfig.pitchAngle, -89.0f, 89.0f, "%.2f");
    ImGui::Checkbox("Yaw", &antiAimConfig.yaw);
}

/////////////////////////////////////////////////////////////////
// Config Functions
/////////////////////////////////////////////////////////////////

static void to_json(json& j, const AntiAimConfig& o, const AntiAimConfig& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Pitch", pitch);
    WRITE("Pitch angle", pitchAngle);
    WRITE("Yaw", yaw);
}

static void from_json(const json& j, AntiAimConfig& a)
{
    read(j, "Enabled", a.enabled);
    read(j, "Pitch", a.pitch);
    read(j, "Yaw", a.yaw);
    read(j, "Pitch angle", a.pitchAngle);
}

json AntiAim::toJson() noexcept
{
    json j;
    to_json(j, antiAimConfig);
    return j;
}

void AntiAim::fromJson(const json& j) noexcept
{
    from_json(j, antiAimConfig);
}

void AntiAim::resetConfig() noexcept
{
    antiAimConfig = { };
}

#else

namespace AntiAim
{
    void run(UserCmd*, const Vector&, const Vector&, bool&) noexcept {}

    // GUI
    void menuBarItem() noexcept {}
    void tabItem() noexcept {}
    void drawGUI() noexcept {}

    // Config
    json toJson() noexcept { return {}; }
    void fromJson(const json& j) noexcept {}
    void resetConfig() noexcept {}
}

#endif
