#pragma once

#include <memory>
#include <clocale>
#include <Windows.h>

#define ENABLE_VMP
#include <Encryption/VMP/def.h>
#include <Encryption/xorstr.hpp>

#include <Hooks.h>
#include <Helpers.h>

class AntiDetection
{
public:
	AntiDetection();
	bool install(DWORD reason, LPVOID reserved, bool cleanPEheader, bool useVPM, bool ScamblePEBlink = true) noexcept;
	void uninstall() noexcept;
	HMODULE getModuleHandle() noexcept;
private:
	HMODULE moduleHandle{ nullptr };
	DWORD memoryProtect{ NULL };
	void cleanPEheader(SIZE_T dwSize = 0x1000) noexcept;
	void HideModule() noexcept;
};

static AntiDetection antiDetect;