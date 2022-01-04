#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include <Windows.h>

#include "Helpers.h"
#include "SDK/Platform.h"

class BaseFileSystem;
class Client;
class Cvar;
class Engine;
class EngineSound;
class EngineTrace;
class EntityList;
class GameEventManager;
class GameMovement;
class GameUI;
class InputSystem;
class Localize;
class MaterialSystem;
class ModelInfo;
class ModelRender;
class NetworkStringTableContainer;
class PanoramaUIEngine;
class PhysicsSurfaceProps;
class Prediction;
class RenderView;
class Surface;
class SoundEmitter;
class StudioRender;

class Interfaces {
public:
#define GAME_INTERFACE(type, name, moduleName, version) \
type* name = reinterpret_cast<type*>(find(moduleName, version));

    GAME_INTERFACE(BaseFileSystem, baseFileSystem, FILESYSTEM_DLL, "VBaseFileSystem011")
    GAME_INTERFACE(Client, client, CLIENT_DLL, "VClient018")
    GAME_INTERFACE(Cvar, cvar, VSTDLIB_DLL, "VEngineCvar007")
    GAME_INTERFACE(Engine, engine, ENGINE_DLL, "VEngineClient014")
    GAME_INTERFACE(EngineTrace, engineTrace, ENGINE_DLL, "EngineTraceClient004")
    GAME_INTERFACE(EntityList, entityList, CLIENT_DLL, "VClientEntityList003")
    GAME_INTERFACE(GameEventManager, gameEventManager, ENGINE_DLL, "GAMEEVENTSMANAGER002")
    GAME_INTERFACE(GameMovement, gameMovement, CLIENT_DLL, "GameMovement001")
    GAME_INTERFACE(GameUI, gameUI, CLIENT_DLL, "GameUI011")
    GAME_INTERFACE(InputSystem, inputSystem, INPUTSYSTEM_DLL, "InputSystemVersion001")
    GAME_INTERFACE(Localize, localize, LOCALIZE_DLL, "Localize_001")
    GAME_INTERFACE(MaterialSystem, materialSystem, MATERIALSYSTEM_DLL, "VMaterialSystem080")
    GAME_INTERFACE(ModelInfo, modelInfo, ENGINE_DLL, "VModelInfoClient004")
    GAME_INTERFACE(ModelRender, modelRender, ENGINE_DLL, "VEngineModel016")
    GAME_INTERFACE(NetworkStringTableContainer, networkStringTableContainer, ENGINE_DLL, "VEngineClientStringTable001")
    GAME_INTERFACE(PanoramaUIEngine, panoramaUIEngine, PANORAMA_DLL, "PanoramaUIEngine001")
    GAME_INTERFACE(PhysicsSurfaceProps, physicsSurfaceProps, VPHYSICS_DLL, "VPhysicsSurfaceProps001")
    GAME_INTERFACE(Prediction, prediction, CLIENT_DLL, "VClientPrediction001")
    GAME_INTERFACE(RenderView, renderView, ENGINE_DLL, "VEngineRenderView014")
    GAME_INTERFACE(Surface, surface, VGUIMATSURFACE_DLL, "VGUI_Surface031")
    GAME_INTERFACE(EngineSound, sound, ENGINE_DLL, "IEngineSoundClient003")
    GAME_INTERFACE(SoundEmitter, soundEmitter, SOUNDEMITTERSYSTEM_DLL, "VSoundEmitter003")
    GAME_INTERFACE(StudioRender, studioRender, STUDIORENDER_DLL, "VStudioRender026")

#undef GAME_INTERFACE
private:
    static std::string getInterfaceDllName(const char* interfacename)  noexcept
    {
        if (interfacename == "VBaseFileSystem011")
            return std::string{ FILESYSTEM_DLL }.append(".dll");
        else if (interfacename == "VEngineCvar007")
            return std::string{ VSTDLIB_DLL }.append(".dll");
        else if (interfacename == "InputSystemVersion001")
            return std::string{ INPUTSYSTEM_DLL }.append(".dll");
        else if (interfacename == "Localize_001")
            return std::string{ LOCALIZE_DLL }.append(".dll");
        else if (interfacename == "VMaterialSystem080")
            return std::string{ MATERIALSYSTEM_DLL }.append(".dll");
        else if (interfacename == "PanoramaUIEngine001")
            return std::string{ PANORAMA_DLL }.append(".dll");
        else if (interfacename == "VPhysicsSurfaceProps001")
            return std::string{ VPHYSICS_DLL }.append(".dll");
        else if (interfacename == "VGUI_Surface031")
            return std::string{ VGUIMATSURFACE_DLL }.append(".dll");
        else if (interfacename == "VSoundEmitter003")
            return std::string{ SOUNDEMITTERSYSTEM_DLL }.append(".dll");
        else if (interfacename == "VStudioRender026")
            return std::string{ STUDIORENDER_DLL }.append(".dll");
        else if (interfacename == "VClient018" || interfacename == "VClientEntityList003" || interfacename == "GameMovement001" || interfacename == "GameUI011" || interfacename == "VClientPrediction001")
            return std::string{ CLIENT_DLL }.append(".dll");
        else if (interfacename == "VEngineClient014" || interfacename == "EngineTraceClient004" || interfacename == "GAMEEVENTSMANAGER002" || interfacename == "VModelInfoClient004" || interfacename == "VEngineModel016" || interfacename == "VEngineClientStringTable001" || interfacename == "VEngineRenderView014" || interfacename == "IEngineSoundClient003")
            return std::string{ ENGINE_DLL }.append(".dll");
        else
            return std::string{ "Undefined DLL" };
    }

    static void* find(const char* moduleName, const char* name) noexcept
    {
        if (const auto createInterface = reinterpret_cast<std::add_pointer_t<void* __CDECL(const char* name, int* returnCode)>>(GetProcAddress(GetModuleHandleA(moduleName), "CreateInterface"))) {
            if (void* foundInterface = createInterface(name, nullptr))
                return foundInterface;
        }
        
        auto title = Helpers::getDllNameVersion();
        auto message = std::string{ "Failed to find interface(" }.append(name).append(") [").append(getInterfaceDllName(name)).append("]");

        Helpers::messageBox(title.c_str(), message.c_str());
        std::exit(EXIT_FAILURE);
    }
};

inline std::unique_ptr<const Interfaces> interfaces;
