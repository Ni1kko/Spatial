#include <memory>
#include <clocale>
#include <Windows.h>

#include "Encryption/AntiDetection.h"

AntiDetection antiDetect;

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    if(antiDetect.getModuleHandle() != moduleHandle)
        return antiDetect.install(reason, reserved);

    return FALSE;
}