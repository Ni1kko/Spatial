#include <memory>
#include <clocale>
#include <Windows.h>

#include "VMP/def.h"
#include "Encryption/xorstr.hpp"


#ifndef _DEBUG
#include "AntiDetection.h"
    AntiDetection antiDetection;
#endif

#include "Hooks.h"

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    VMP_ULTRA(xorstr_("DllMain"));
    if (!_CRT_INIT(moduleHandle, reason, reserved))
        return FALSE;

    if (reason == DLL_PROCESS_ATTACH) {
        VMP_ULTRA(xorstr_("OnDllAttach"));
        std::setlocale(LC_CTYPE, xorstr_(".utf8"));
        hooks = std::make_unique<Hooks>(moduleHandle);
    }
    return TRUE;
    VMP_END;
}