#pragma once

#include "../JsonForward.h"
#include "../SDK/GameEvent.h"
#include "../InputUtil.h"


struct MovementConfig {
    bool fixMouseDelta{ false };
    bool fixMovement{ false };
    bool fixBoneMatrix{ false };


    bool autoStrafe{ false };
    bool fastStop{ false };
    bool moonwalk{ false };
    bool fastCrouch{ false };
    bool bunnyHop{ false };
    bool edgejump{ false }; KeyBind edgejumpkey;
    bool slowwalk{ false }; KeyBind slowwalkKey;
    bool setMaxAngleDelta{ false }; float maxAngleDelta{ 255.0f };
    bool fastPlant{ false };

    bool autoPeek{ false }; KeyBindToggle autoPeekKey;
} movementConfig;

namespace Movement
{
    // Vars
    float defaultMaxAngleDelta = 255.0f;
    Vector AutoPeekPosition {};

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
    void autoPeek(UserCmd* cmd, Vector currentViewAngles) noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;
    
    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}