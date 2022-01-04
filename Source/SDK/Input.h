#pragma once

#include "Pad.h"
#include "Vector.h"

template<typename FuncType>
__forceinline static FuncType qCallVFunction(void* ppClass, int index)
{
    int* pVTable = *(int**)ppClass;
    int dwAddress = pVTable[index];
    return (FuncType)(dwAddress);
}

class Input {
public:
    PAD(12)
    bool isTrackIRAvailable;
    bool isMouseInitialized;
    bool isMouseActive;
    PAD(178)
    bool isCameraInThirdPerson;
    bool cameraMovingWithMouse;
    Vector cameraOffset;
};
