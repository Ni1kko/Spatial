#include <algorithm>
#include <Windows.h>


#include "VmtHook.h"
#include "../Helpers.h"

void VmtHook::init(void* base) noexcept
{
    this->base = base;
    auto vmt = *reinterpret_cast<std::uintptr_t**>(base);
    length = Helpers::calculateVmtLength(vmt);
    oldVmt = std::make_unique<std::uintptr_t[]>(length);
    std::copy(vmt, vmt + length, oldVmt.get());
}

void VmtHook::restore() const noexcept
{
    auto vmt = *reinterpret_cast<std::uintptr_t**>(base);

    if (DWORD oldProtection; VirtualProtect(vmt, length, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        std::copy(oldVmt.get(), oldVmt.get() + length, vmt);
        VirtualProtect(vmt, length, oldProtection, nullptr);
    }
}

void VmtHook::hookAt(std::size_t index, void* fun) const noexcept
{
    auto address = *reinterpret_cast<std::uintptr_t**>(base) + index;

    if (DWORD oldProtection; VirtualProtect(address, sizeof(address), PAGE_EXECUTE_READWRITE, &oldProtection)) {
        *address = std::uintptr_t(fun);
        VirtualProtect(address, sizeof(address), oldProtection, nullptr);
    }
}
