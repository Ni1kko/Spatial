#pragma once

#include <cstdint>

#include "Inconstructible.h"
#include "Platform.h"

template <typename T> class UtlVector;

class EconItemView {
public:
    INCONSTRUCTIBLE(EconItemView)

    std::uintptr_t getAttributeList() noexcept
    {
        return std::uintptr_t(this) + 0x244;
    }

    UtlVector<void*>& customMaterials() noexcept
    {
        return *reinterpret_cast<UtlVector<void*>*>(std::uintptr_t(this) + 0x14);
    }

    UtlVector<void*>& visualDataProcessors() noexcept
    {
        return *reinterpret_cast<UtlVector<void*>*>(std::uintptr_t(this) + 0x230);
    }

    void clearInventoryImageRGBA() noexcept;
};
