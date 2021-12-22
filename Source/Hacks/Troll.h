#pragma once

#include "../JsonForward.h"
#include "../SDK/GameEvent.h"

enum class ChatSpamEvents
{
    Off = 0,
    Timed,
    OnKill,
    OnDeath,
    OnKey,
    OnMVP,
    OnDMG
};

namespace Troll
{
	// Functions
	void blockbot(UserCmd* cmd) noexcept;
    void doorSpam(UserCmd* cmd) noexcept;
	void chatSpam(ChatSpamEvents chatSpamEvent) noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;
    
    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}