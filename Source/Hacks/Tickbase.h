#pragma once

#include "../JsonForward.h"

struct UserCmd;

namespace Tickbase
{
	struct Tick
	{
		int	maxUsercmdProcessticks{ 17 }; //on valve servers this is 8 ticks, always do +1 command
		int ticksAllowedForProcessing{ maxUsercmdProcessticks };
		int chokedPackets{ 0 };
		int fakeLag{ 0 };
		int tickshift{ 0 };
		int tickbase{ 0 };
		int commandNumber{ 0 };
		int ticks{ 0 };
	};

	inline std::unique_ptr<Tick> tick;

	// Functions
	void shiftTicks(int, UserCmd*, bool = false) noexcept; 
	void chokePackets(bool& sendPacket) noexcept;
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
