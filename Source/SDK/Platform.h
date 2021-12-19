#pragma once


#define __THISCALL __thiscall
#define __FASTCALL __fastcall
#define __CDECL __cdecl
#define __STDCALL __stdcall

#define RETURN_ADDRESS() std::uintptr_t(_ReturnAddress())
#define FRAME_ADDRESS() (std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t))

constexpr auto CLIENT_DLL = "client";
constexpr auto ENGINE_DLL = "engine";
constexpr auto FILESYSTEM_DLL = "filesystem_stdio";
constexpr auto INPUTSYSTEM_DLL = "inputsystem";
constexpr auto LOCALIZE_DLL = "localize";
constexpr auto MATERIALSYSTEM_DLL = "materialsystem";
constexpr auto PANORAMA_DLL = "panorama";
constexpr auto SOUNDEMITTERSYSTEM_DLL = "soundemittersystem";
constexpr auto STUDIORENDER_DLL = "studiorender";
constexpr auto TIER0_DLL = "tier0";
constexpr auto VGUI2_DLL = "vgui2";
constexpr auto VGUIMATSURFACE_DLL = "vguimatsurface";
constexpr auto VPHYSICS_DLL = "vphysics";
constexpr auto VSTDLIB_DLL = "vstdlib";