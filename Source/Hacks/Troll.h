#pragma once

#include "../JsonForward.h"


namespace Troll
{
	// Functions
	void blockbot(UserCmd* cmd) noexcept;
    void doorSpam(UserCmd* cmd) noexcept;
	void chatSpam() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;
    
    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;

    long timestamp {0};
}