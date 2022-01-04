#include <memory>
#include <clocale>
#include <Windows.h>

#include "VMP/def.h"
#include "xorstr.hpp"
#include "AntiDetection.h"
#include "Hooks.h"

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

static constexpr HMODULE getSelfModuleHandle() noexcept
{
	MEMORY_BASIC_INFORMATION mbi;
	return ((::VirtualQuery(getSelfModuleHandle, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
}

AntiDetection::AntiDetection()
{
	moduleHandle = getSelfModuleHandle();
}

HMODULE AntiDetection::getModuleHandle() noexcept
{ 
	return moduleHandle;
}

void AntiDetection::cleanPEheader() noexcept
{
	DWORD dwMemPro;

	VirtualProtect((void*)moduleHandle, 0x1000, PAGE_EXECUTE_READWRITE, &dwMemPro);
	memset((void*)moduleHandle, 0, 0x1000);
	VirtualProtect((void*)moduleHandle, 0x1000, dwMemPro, &dwMemPro);

	OutputDebugStringA(xorstr_("CleanUp PEheader Success."));
}

void AntiDetection::HideModule() noexcept
{
	void* pPEB = nullptr;

	_asm
	{
		push eax
		mov eax, fs: [0x30]
		mov pPEB, eax
		pop eax
	}

	void* pLDR = *((void**)((unsigned char*)pPEB + 0xc));
	void* pCurrent = *((void**)((unsigned char*)pLDR + 0x0c));
	void* pNext = pCurrent;

	do
	{
		void* pNextPoint = *((void**)((unsigned char*)pNext));
		void* pLastPoint = *((void**)((unsigned char*)pNext + 0x4));
		void* nBaseAddress = *((void**)((unsigned char*)pNext + 0x18));

		if (nBaseAddress == moduleHandle)
		{
			*((void**)((unsigned char*)pLastPoint)) = pNextPoint;
			*((void**)((unsigned char*)pNextPoint + 0x4)) = pLastPoint;
			pCurrent = pNextPoint;
		}

		pNext = *((void**)pNext);
	} while (pCurrent != pNext);
	
	OutputDebugStringA(xorstr_("Cutup PEB link success."));
}

bool AntiDetection::install(DWORD reason, LPVOID reserved) noexcept 
{
	VMP_ULTRA(xorstr_("DllMain"));
	if (!_CRT_INIT(moduleHandle, reason, reserved))
		return FALSE;

	if (reason == DLL_PROCESS_ATTACH) {
		VMP_ULTRA(xorstr_("OnDllAttach"));
		std::setlocale(LC_CTYPE, xorstr_(".utf8")); 
		cleanPEheader();
		HideModule();
		hooks = std::make_unique<Hooks>(moduleHandle);
	}
	return TRUE;
	VMP_END;
}