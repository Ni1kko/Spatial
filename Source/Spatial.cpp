#include <Encryption/AntiDetection.h>
 
BOOL APIENTRY DllEntryPoint(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{ 
    if (antiDetect.getModuleHandle() != moduleHandle)
        return antiDetect.install(reason, reserved, true, true, true);
    
    return FALSE;
}