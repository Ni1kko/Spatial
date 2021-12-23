#pragma once

#include "../JsonForward.h"

struct UserCmd;

namespace Tickbase
{
	// Functions
	void chokePackets(bool& sendPacket) noexcept;
	bool DTKeyDown() noexcept;
	bool CPKeyDown() noexcept;
	void run(UserCmd*) noexcept;
	void drawGUI() noexcept;

	// Config
	json toJson() noexcept;
	void fromJson(const json& j) noexcept;
	void resetConfig() noexcept;
} 
