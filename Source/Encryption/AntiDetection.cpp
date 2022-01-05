#include "AntiDetection.h"

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

void AntiDetection::cleanPEheader(SIZE_T dwSize) noexcept
{
	VirtualProtect((void*)moduleHandle, dwSize, PAGE_EXECUTE_READWRITE, &memoryProtect);
	memset((void*)moduleHandle, 0, dwSize);
	VirtualProtect((void*)moduleHandle, dwSize, memoryProtect, &memoryProtect);
	OutputDebugStringA(xorstr_("PEheader cleaned"));
}

void AntiDetection::HideModule() noexcept
{
	void* pPEB = NULL;

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
	
	OutputDebugStringA(xorstr_("PEB link scrambled."));
}

bool AntiDetection::install(DWORD reason, LPVOID reserved, bool cleanPE, bool useVPM, bool ScamblePEB) noexcept
{
	if(!cleanPE && useVPM)
		VMP_ULTRA(xorstr_("DllMain"));

	if (!_CRT_INIT(moduleHandle, reason, reserved))
		return FALSE;

	if (reason == DLL_PROCESS_ATTACH) {
		if (!cleanPE && useVPM) VMP_ULTRA(xorstr_("OnDllAttach"));
		std::setlocale(LC_CTYPE, xorstr_(".utf8"));
		if (cleanPE) cleanPEheader(0x2000);
		if (ScamblePEB) HideModule();
		hooks = std::make_unique<Hooks>(moduleHandle);
	}

	if (!cleanPE && useVPM) VMP_END;

	return TRUE;
}