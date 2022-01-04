#pragma once

#ifdef _WIN32

#define __THISCALL __thiscall
#define __FASTCALL __fastcall
#define __CDECL __cdecl
#define __STDCALL __stdcall

#else

#define __THISCALL
#define __FASTCALL
#define __CDECL
#define __STDCALL

#endif

#ifdef _WIN32

#define LINUX_ARGS(...)
#define RETURN_ADDRESS() std::uintptr_t(_ReturnAddress())
#define FRAME_ADDRESS() (std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t))
#define IS_WIN32() true

#else

#define LINUX_ARGS(...) __VA_ARGS__
#define RETURN_ADDRESS() std::uintptr_t(__builtin_return_address(0))
#define FRAME_ADDRESS() std::uintptr_t(__builtin_frame_address(0))
#define IS_WIN32() false

#endif

constexpr auto CLIENT_DLL = "client";
constexpr auto ENGINE_DLL = "engine";
constexpr auto FILESYSTEM_DLL ="filesystem_stdio";
constexpr auto INPUTSYSTEM_DLL = "inputsystem";
constexpr auto LOCALIZE_DLL = "localize";
constexpr auto MATERIALSYSTEM_DLL ="materialsystem";
constexpr auto PANORAMA_DLL = "panorama";
constexpr auto SOUNDEMITTERSYSTEM_DLL = "soundemittersystem";
constexpr auto STUDIORENDER_DLL = "studiorender";
constexpr auto TIER0_DLL = "tier0";
constexpr auto VGUI2_DLL = "vgui2";
constexpr auto VGUIMATSURFACE_DLL = "vguimatsurface";
constexpr auto VPHYSICS_DLL = "vphysics";
constexpr auto VSTDLIB_DLL = "vstdlib";
