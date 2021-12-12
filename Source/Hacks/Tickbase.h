#pragma once

#include "../JsonForward.h"

struct UserCmd;

namespace Tickbase
{
	// Functions
	void shiftTicks(int, UserCmd*, bool = false) noexcept; 
	void chokePackets(bool& sendPacket) noexcept;
	bool DTKeyDown() noexcept;
	bool CPKeyDown() noexcept;
	void run(UserCmd*) noexcept;
	 
	// GUI
	void menuBarItem() noexcept;
	void tabItem() noexcept;
	void drawGUI(bool contentOnly) noexcept;

	// Config
	json toJson() noexcept;
	void fromJson(const json& j) noexcept;
	void resetConfig() noexcept;
} 
