#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "../SDK/Platform.h"

class VmtSwap {
public:
    void init(void* base) noexcept;
    void restore() noexcept
    {
        *reinterpret_cast<std::uintptr_t**>(base) = oldVmt;
    }

    template<typename T>
    void hookAt(std::size_t index, T fun) const noexcept
    {
        newVmt[index + dynamicCastInfoLength] = reinterpret_cast<std::uintptr_t>(fun);
    }

    template<typename T, std::size_t Idx, typename ...Args>
    constexpr auto getOriginal(Args...) const noexcept
    {
        return reinterpret_cast<T(__THISCALL*)(void*, Args...)>(oldVmt[Idx]);
    }

    template<typename T, std::size_t Idx, typename ...Args>
    constexpr auto callOriginal(Args... args) const noexcept
    {
        return getOriginal<T, Idx>(args...)(base, args...);
    }

private:
    static constexpr auto dynamicCastInfoLength = 1;

    void* base = nullptr;
    std::uintptr_t* oldVmt = nullptr;
    std::unique_ptr<std::uintptr_t[]> newVmt;
    std::size_t length = 0;
};
