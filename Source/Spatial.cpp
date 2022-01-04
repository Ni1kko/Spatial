#include <memory>
#include <clocale>
#include <Windows.h>

#include "AntiDetection.h"

AntiDetection antiDetect;

BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    return antiDetect.install(reason, reserved);
}