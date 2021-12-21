#pragma once

#include "../JsonForward.h"
#include "../SDK/GameEvent.h"
#include "../InputUtil.h"



namespace Movement
{
    // Vars
    float defaultMaxAngleDelta = 255.0f;

	// Functions
    void fixMouseDelta(UserCmd* cmd) noexcept;
    void fixMovement(UserCmd* cmd, float yaw) noexcept;
    bool fixBoneMatrix() noexcept;
    
    void autoStrafe(UserCmd* cmd) noexcept;
    void fastStop(UserCmd*) noexcept;
    void moonwalk(UserCmd* cmd) noexcept;
    void fastCrouch(UserCmd* cmd) noexcept;
    void bunnyHop(UserCmd*) noexcept;
    void edgejump(UserCmd* cmd) noexcept;
    void slowwalk(UserCmd* cmd) noexcept;
    float maxAngleDelta() noexcept;
    void fastPlant(UserCmd*) noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;
    
    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}