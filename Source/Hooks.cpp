#include <charconv>
#include <functional>
#include <string>

#include "Menu/imgui/imgui.h"


#include <intrin.h>
#include <Windows.h>
#include <Psapi.h>

#include "Menu/imgui/imgui_impl_dx9.h"
#include "Menu/imgui/imgui_impl_win32.h"

#include "MinHook/MinHook.h"

#include "Config.h"
#include "EventListener.h"
#include "GameData.h"
#include "Menu/Menu.h"
#include "Hooks.h"
#include "Interfaces.h"
#include "Memory.h"

#include "Hacks/Aimbot.h"
#include "Hacks/AntiAim.h"
#include "Hacks/Chams.h"
#include "Hacks/EnginePrediction.h"
#include "Hacks/StreamProofESP.h"
#include "Hacks/Glow.h"
#include "Hacks/Misc.h"
#include "Hacks/Troll.h"
#include "Hacks/Sound.h"
#include "Hacks/Triggerbot.h"
#include "Hacks/Visuals.h"
#include "Hacks/Tickbase.h"

#include "InventoryChanger/InventoryChanger.h"

#include "SDK/ClientClass.h"
#include "SDK/Cvar.h"
#include "SDK/Engine.h"
#include "SDK/Entity.h"
#include "SDK/EntityList.h"
#include "SDK/FrameStage.h"
#include "SDK/GameEvent.h"
#include "SDK/GameUI.h"
#include "SDK/GlobalVars.h"
#include "SDK/InputSystem.h"
#include "SDK/ItemSchema.h"
#include "SDK/LocalPlayer.h"
#include "SDK/MaterialSystem.h"
#include "SDK/ModelRender.h"
#include "SDK/Platform.h"
#include "SDK/RenderContext.h"
#include "SDK/SoundInfo.h"
#include "SDK/SoundEmitter.h"
#include "SDK/StudioRender.h"
#include "SDK/Surface.h"
#include "SDK/UserCmd.h"
#include "SDK/UserMessages.h"
#include <Hacks/Movement.h>
#include <Encryption/AntiDetection.h>

/*
    Helpful keybinds (VS 2019)
    CTRL + M + O will collapse all
    CTRL + M + L will expand all

    Helpful keybinds (VS Code)
    CTRL + K + 0 will collapse all
    CTRL + K + J will expand all
*/

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT __stdcall wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    [[maybe_unused]] static const auto once = [](HWND window) noexcept {
        Netvars::init();
        EventListener::init();

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(window);
        config = std::make_unique<Config>();
        gui = std::make_unique<Menu>();
        chams = std::make_unique<Chams>();
        movement = std::make_unique<Movement>();
        
        hooks->install();
        return true;
    }(window);

    ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);
    interfaces->inputSystem->enableInput(!gui->isOpen());

    return CallWindowProcW(hooks->originalWndProc, window, msg, wParam, lParam);
}

static HRESULT __stdcall reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    InventoryChanger::clearItemIconTextures();
    GameData::clearTextures();
    return hooks->originalReset(device, params);
}

static HRESULT __stdcall present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{
    [[maybe_unused]] static bool imguiInit{ ImGui_ImplDX9_Init(device) };

    if (config->loadScheduledFonts())
        ImGui_ImplDX9_DestroyFontsTexture();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    if (const auto& displaySize = ImGui::GetIO().DisplaySize; displaySize.x > 0.0f && displaySize.y > 0.0f) {
        StreamProofESP::render();
        Misc::purchaseList();
        Misc::noscopeCrosshair(ImGui::GetBackgroundDrawList());
        Misc::recoilCrosshair(ImGui::GetBackgroundDrawList());
        Misc::drawOffscreenEnemies(ImGui::GetBackgroundDrawList());
        Misc::drawBombTimer();
        Misc::spectatorList();
        Visuals::hitMarker(nullptr, ImGui::GetBackgroundDrawList());
        Visuals::drawMolotovHull(ImGui::GetBackgroundDrawList());
        Visuals::drawSmokeTimer(ImGui::GetBackgroundDrawList());
        Visuals::drawAutoPeek(ImGui::GetBackgroundDrawList());
        Aimbot::drawFov(ImGui::GetBackgroundDrawList());
        Aimbot::updateInput();
        Visuals::updateInput();
        StreamProofESP::updateInput();
        Misc::updateInput();
        Triggerbot::updateInput();
        chams->updateInput();
        Glow::updateInput();
        Troll::chatSpam(ChatSpamEvents::Timed);
        gui->render(ImGui::GetBackgroundDrawList(), displaySize);
    }

    ImGui::EndFrame();
    ImGui::Render();

    if (device->BeginScene() == D3D_OK) {
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        device->EndScene();
    }

    GameData::clearUnusedAvatars();
    InventoryChanger::clearUnusedItemIconTextures();

    return hooks->originalPresent(device, src, dest, windowOverride, dirtyRegion);
}

static bool __STDCALL createMove(float inputSampleTime, UserCmd* cmd) noexcept
{
    auto result = hooks->clientMode.callOriginal<bool, 24>(inputSampleTime, cmd);

    if (!cmd->commandNumber)
        return result;

    bool& sendPacket = *reinterpret_cast<bool*>(*reinterpret_cast<std::uintptr_t*>(FRAME_ADDRESS()) - 0x1C);

    static auto previousViewAngles{ cmd->viewangles };
    const auto currentViewAngles{ cmd->viewangles };

    memory->globalVars->serverTime(cmd);
    Misc::nadePredict();
    Misc::antiAfkKick(cmd);
    movement->fastStop(cmd);
    Misc::prepareRevolver(cmd);
    Visuals::removeShadows();
    Misc::runReportbot();
    movement->bunnyHop(cmd);
    movement->autoStrafe(cmd);
    movement->fastCrouch(cmd);
    Misc::autoPistol(cmd);
    Misc::autoReload(cmd);
    Misc::updateClanTag();
    Misc::fakeMessage();
    Misc::stealNames();
    Misc::revealRanks(cmd);
    Misc::quickReload(cmd);
    Misc::fixTabletSignal();
    movement->slowwalk(cmd);
    Troll::blockbot(cmd);
    Visuals::viewModel();

    EnginePrediction::run(cmd);

    Aimbot::run(cmd);
    Triggerbot::run(cmd);
    movement->autoPeek(cmd, currentViewAngles);
    movement->edgejump(cmd);
    movement->moonwalk(cmd);
    movement->fastPlant(cmd);
    Troll::doorSpam(cmd);

    if (!(cmd->buttons & (UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2))) {
        Tickbase::chokePackets(sendPacket);
        AntiAim::run(cmd, previousViewAngles, currentViewAngles, sendPacket);
    }

    auto viewAnglesDelta{ cmd->viewangles - previousViewAngles };
    viewAnglesDelta.normalize();
    viewAnglesDelta.x = std::clamp(viewAnglesDelta.x, -movement->maxAngleDelta(), movement->maxAngleDelta());
    viewAnglesDelta.y = std::clamp(viewAnglesDelta.y, -movement->maxAngleDelta(), movement->maxAngleDelta());

    cmd->viewangles = previousViewAngles + viewAnglesDelta;

    cmd->viewangles.normalize();
    movement->fixMovement(cmd, currentViewAngles.y);

    cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f);
    cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
    cmd->viewangles.z = 0.0f;
    cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
    cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);

    movement->fixMouseDelta(cmd);

    previousViewAngles = cmd->viewangles;

    Tickbase::run(cmd);

    return false;
}

static void __STDCALL doPostScreenEffects(void* param) noexcept
{
    if (interfaces->engine->isInGame()) {
        Visuals::thirdperson();
        Visuals::inverseRagdollGravity();
        Visuals::reduceFlashEffect();
        Visuals::updateBrightness();
        Visuals::remove3dSky();
        Glow::render();
    }
    hooks->clientMode.callOriginal<void, 44>(param);
}

static float __STDCALL getViewModelFov() noexcept
{
    float additionalFov = Visuals::viewModelFov();
    if (localPlayer) {
        if (const auto activeWeapon = localPlayer->getActiveWeapon(); activeWeapon && activeWeapon->getClientClass()->classId == ClassId::CTablet)
            additionalFov = 0.0f;
    }

    return hooks->clientMode.callOriginal<float, 35>() + additionalFov;
}

static void __STDCALL drawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    if (interfaces->studioRender->isForcedMaterialOverride())
        return hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);

    if (Visuals::removeHands(info.model->name) || Visuals::removeSleeves(info.model->name) || Visuals::removeWeapons(info.model->name))
        return;

    if (!chams->render(ctx, state, info, customBoneToWorld))
        hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);

    interfaces->studioRender->forcedMaterialOverride(nullptr);
}

static bool __FASTCALL svCheatsGetBool(void* _this) noexcept
{
    if (RETURN_ADDRESS() == memory->cameraThink && Visuals::isThirdpersonOn())
        return true;

    return hooks->svCheats.getOriginal<bool, 13>()(_this);
}

static void __STDCALL frameStageNotify(FrameStage stage) noexcept
{
    [[maybe_unused]] static auto aimbotInit = (Aimbot::init(), false);

    if (interfaces->engine->isConnected() && !interfaces->engine->isInGame())
        Misc::changeName(true, nullptr, 0.0f);

    if (stage == FrameStage::START)
        GameData::update();

    if (stage == FrameStage::RENDER_START) {
        Misc::fakePrime();
        Misc::preserveKillfeed();
        Misc::disablePanoramablur();
        Visuals::colorWorld();
        Misc::updateEventListeners();
        Visuals::updateEventListeners();
        Misc::forceRelayCluster();
    }
    if (interfaces->engine->isInGame()) {
        Visuals::skybox(stage);
        Visuals::removeBlur(stage);
        Misc::oppositeHandKnife(stage);
        Visuals::removeGrass(stage);
        Visuals::modifySmoke(stage);
        Visuals::disablePostProcessing(stage);
        Visuals::removeVisualRecoil(stage);
        Visuals::applyZoom(stage);
        Misc::fixAnimationLOD(stage);
        Troll::chatSpam(ChatSpamEvents::OnKey);
    }
    InventoryChanger::run(stage);

    hooks->client.callOriginal<void, 37>(stage);
}

static int __STDCALL emitSound(void* filter, int entityIndex, int channel, const char* soundEntry, unsigned int soundEntryHash, const char* sample, float volume, int seed, int soundLevel, int flags, int pitch, const Vector& origin, const Vector& direction, void* utlVecOrigins, bool updatePositions, float soundtime, int speakerentity, void* soundParams) noexcept
{
    Sound::modulateSound(soundEntry, entityIndex, volume);
    Misc::autoAccept(soundEntry);

    volume = std::clamp(volume, 0.0f, 1.0f);
    return hooks->sound.callOriginal<int, 5>(filter, entityIndex, channel, soundEntry, soundEntryHash, sample, volume, seed, soundLevel, flags, pitch, std::cref(origin), std::cref(direction), utlVecOrigins, updatePositions, soundtime, speakerentity, soundParams);
}

static bool __STDCALL shouldDrawFog() noexcept
{
    if constexpr (std::is_same_v<HookType, MinHook>) {
#ifdef _DEBUG
    // Check if we always get the same return address
    if (c_cast<std::uint32_t*>(_ReturnAddress()) == 0x6274C084) {
        static const auto returnAddress = std::uintptr_t(_ReturnAddress());
        assert(returnAddress == std::uintptr_t(_ReturnAddress()));
    }
#endif

    if (*static_cast<std::uint32_t*>(_ReturnAddress()) != 0x6274C084)
        return hooks->clientMode.callOriginal<bool, 17>();
    }
    
    return !Visuals::shouldRemoveFog();
}

static bool __STDCALL shouldDrawViewModel() noexcept
{
    if (Visuals::isZoomOn() && localPlayer && localPlayer->fov() < 45 && localPlayer->fovStart() < 45)
        return false;
    return hooks->clientMode.callOriginal<bool, 27>();
}

static void __STDCALL lockCursor() noexcept
{
    if (gui->isOpen())
        return interfaces->surface->unlockCursor();
    return hooks->surface.callOriginal<void, 67>();
}

static void __STDCALL setDrawColor(int r, int g, int b, int a) noexcept
{
    if (Visuals::shouldRemoveScopeOverlay() && (RETURN_ADDRESS() == memory->scopeDust || RETURN_ADDRESS() == memory->scopeArc))
        a = 0;
    hooks->surface.callOriginal<void, 15>(r, g, b, a);
}

struct ViewSetup {
    PAD(172);
    void* csm;
    float fov;
    PAD(32);
    float farZ;
};

static void __STDCALL overrideView(ViewSetup* setup) noexcept
{
    if (localPlayer)
    {
        if (!localPlayer->isScoped())
            setup->fov += Visuals::fov();

        config->totalFov = setup->fov;
    }
    setup->farZ += Visuals::farZ() * 10;
    hooks->clientMode.callOriginal<void, 18>(setup);
}

struct RenderableInfo {
    Entity* renderable;
    std::byte pad[18];
    uint16_t flags;
    uint16_t flags2;
};

static int __STDCALL listLeavesInBox(const Vector& mins, const Vector& maxs, unsigned short* list, int listMax) noexcept
{
    if (Misc::shouldDisableModelOcclusion() && RETURN_ADDRESS() == memory->insertIntoTree) {
        if (const auto info = *reinterpret_cast<RenderableInfo**>(FRAME_ADDRESS() + 0x18); info && info->renderable) {
            if (const auto ent = VirtualMethod::call<Entity*, 7>(info->renderable - sizeof(std::uintptr_t)); ent && ent->isPlayer()) {
                constexpr float maxCoord = 16384.0f;
                constexpr float minCoord = -maxCoord;
                constexpr Vector min{ minCoord, minCoord, minCoord };
                constexpr Vector max{ maxCoord, maxCoord, maxCoord };
                return hooks->bspQuery.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);
            }
        }
    }

    return hooks->bspQuery.callOriginal<int, 6>(std::cref(mins), std::cref(maxs), list, listMax);
}

static int __FASTCALL dispatchSound(SoundInfo& soundInfo) noexcept
{
    if (const char* soundName = interfaces->soundEmitter->getSoundName(soundInfo.soundIndex)) {
        Sound::modulateSound(soundName, soundInfo.entityIndex, soundInfo.volume);
        soundInfo.volume = std::clamp(soundInfo.volume, 0.0f, 1.0f);
    }
    return hooks->originalDispatchSound(soundInfo);
}

static void __STDCALL render2dEffectsPreHud(void* viewSetup) noexcept
{
    Visuals::applyScreenEffects();
    Visuals::hitEffect();
    hooks->viewRender.callOriginal<void, 39>(viewSetup);
}

static const DemoPlaybackParameters* __STDCALL getDemoPlaybackParameters() noexcept
{
    const auto params = hooks->engine.callOriginal<const DemoPlaybackParameters*, 218>();

    if (params && Misc::shouldRevealSuspect() && RETURN_ADDRESS() != memory->demoFileEndReached) {
        static DemoPlaybackParameters customParams;
        customParams = *params;
        customParams.anonymousPlayerIdentity = false;
        return &customParams;
    }

    return params;
}

static bool __STDCALL isPlayingDemo() noexcept
{
    if (Misc::shouldRevealMoney() && RETURN_ADDRESS() == memory->demoOrHLTV && *reinterpret_cast<std::uintptr_t*>(FRAME_ADDRESS() + 8) == memory->money)
        return true;

    return hooks->engine.callOriginal<bool, 82>();
}

static void __STDCALL updateColorCorrectionWeights() noexcept
{
    hooks->clientMode.callOriginal<void, 58>();

    Visuals::performColorCorrection();
    if (Visuals::shouldRemoveScopeOverlay())
        *memory->vignette = 0.0f;
}

static float __STDCALL getScreenAspectRatio(int width, int height) noexcept
{
    if (Visuals::aspectRatio() != 0.0f)
        return Visuals::aspectRatio();
    return hooks->engine.callOriginal<float, 101>(width, height);
}

static void __STDCALL renderSmokeOverlay(bool update) noexcept
{
    if (Visuals::shouldRemoveSmoke() || Visuals::isSmokeWireframe())
        *reinterpret_cast<float*>(std::uintptr_t(memory->viewRender) + 0x588) = 0.0f;
    else
        hooks->viewRender.callOriginal<void, 41>(update);
}

static double __STDCALL getArgAsNumber(void* params, int index) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<double, 5>(params, index);
    InventoryChanger::getArgAsNumberHook(static_cast<int>(result), RETURN_ADDRESS());
    return result;
}

static const char* __STDCALL getArgAsString(void* params, int index) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<const char*, 7>(params, index);

    if (result)
        InventoryChanger::getArgAsStringHook(result, RETURN_ADDRESS());

    return result;
}

static bool __STDCALL equipItemInLoadout(Team team, int slot, std::uint64_t itemID, bool swap) noexcept
{
    InventoryChanger::onItemEquip(team, slot, itemID);
    return hooks->inventoryManager.callOriginal<bool, 20>(team, slot, itemID, swap);
}

static void __STDCALL soUpdated(SOID owner, SharedObject* object, int event) noexcept
{
    InventoryChanger::onSoUpdated(object);
    hooks->inventory.callOriginal<void, 1>(owner, object, event);
}

static bool __STDCALL dispatchUserMessage(UserMessageType type, int passthroughFlags, int size, const void* data) noexcept
{
    if (type == UserMessageType::Text)
        InventoryChanger::onUserTextMsg(data, size);
    else if (type == UserMessageType::VoteStart)
        Misc::onVoteStart(data, size);
    else if (type == UserMessageType::VotePass)
        Misc::onVotePass();
    else if (type == UserMessageType::VoteFailed)
        Misc::onVoteFailed();
    
    return hooks->client.callOriginal<bool, 38>(type, passthroughFlags, size, data);
}

static int __STDCALL getUnverifiedFileHashes(void* thisPointer, int maxFiles)
{
    if (Misc::enableSvPureBypass())
        return 0;
    return hooks->fileSystem.callOriginal<int, 101>(thisPointer, maxFiles);
}

static int __STDCALL canLoadThirdPartyFiles(void* thisPointer, void* edx) noexcept
{
    if (Misc::enableSvPureBypass())
        return 1;
    return hooks->fileSystem.callOriginal<int, 128>(thisPointer);
}

Hooks::Hooks(HMODULE moduleHandle) noexcept : moduleHandle{ moduleHandle }
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    interfaces = std::make_unique<const Interfaces>();
    
    if (!Helpers::checkForUpdates())
    {
        memory = std::make_unique<const Memory>();
        window = FindWindowW(L"Valve001", nullptr);
        originalWndProc = WNDPROC(SetWindowLongPtrW(window, GWLP_WNDPROC, LONG_PTR(&wndProc)));
        Helpers::writeDebugConsole(std::string{ "csgo version: " }.append(Helpers::game_version).c_str(), { 0, 255, 0, 255 });
    }
}

void Hooks::install() noexcept
{
    originalPresent = **reinterpret_cast<decltype(originalPresent)**>(memory->present);
    **reinterpret_cast<decltype(present)***>(memory->present) = present;
    originalReset = **reinterpret_cast<decltype(originalReset)**>(memory->reset);
    **reinterpret_cast<decltype(reset)***>(memory->reset) = reset;

    if constexpr (std::is_same_v<HookType, MinHook>) MH_Initialize();
    
    bspQuery.init(interfaces->engine->getBSPTreeQuery());
    bspQuery.hookAt(6, &listLeavesInBox);

    client.init(interfaces->client);
    client.hookAt(37, &frameStageNotify);
    client.hookAt(38, &dispatchUserMessage);

    clientMode.init(memory->clientMode);
    clientMode.hookAt(17, &shouldDrawFog);
    clientMode.hookAt(18, &overrideView);
    clientMode.hookAt(24, &createMove);
    clientMode.hookAt(27, &shouldDrawViewModel);
    clientMode.hookAt(35, &getViewModelFov);
    clientMode.hookAt(44, &doPostScreenEffects);
    clientMode.hookAt(58, &updateColorCorrectionWeights);

    engine.init(interfaces->engine);
    engine.hookAt(82, &isPlayingDemo);
    engine.hookAt(101, &getScreenAspectRatio);
    engine.hookAt(218, &getDemoPlaybackParameters);

    fileSystem.init(interfaces->fileSystem);
    //fileSystem.hookAt(101, getUnverifiedFileHashes);
    //fileSystem.hookAt(128, canLoadThirdPartyFiles);

    inventory.init(memory->inventoryManager->getLocalInventory());
    inventory.hookAt(1, &soUpdated);

    inventoryManager.init(memory->inventoryManager);
    inventoryManager.hookAt(20, &equipItemInLoadout);

    modelRender.init(interfaces->modelRender);
    modelRender.hookAt(21, &drawModelExecute);

    panoramaMarshallHelper.init(memory->panoramaMarshallHelper);
    panoramaMarshallHelper.hookAt(5, &getArgAsNumber);
    panoramaMarshallHelper.hookAt(7, &getArgAsString);

    sound.init(interfaces->sound);
    sound.hookAt(5, &emitSound);

    surface.init(interfaces->surface);
    surface.hookAt(15, &setDrawColor);
    
    svCheats.init(interfaces->cvar->findVar("sv_cheats"));
    svCheats.hookAt(13, &svCheatsGetBool);

    viewRender.init(memory->viewRender);
    viewRender.hookAt(39, &render2dEffectsPreHud);
    viewRender.hookAt(41, &renderSmokeOverlay);

    if (DWORD oldProtection; VirtualProtect(memory->dispatchSound, 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {

        originalDispatchSound = decltype(originalDispatchSound)(uintptr_t(memory->dispatchSound + 1) + *memory->dispatchSound);
        *memory->dispatchSound = uintptr_t(&dispatchSound) - uintptr_t(memory->dispatchSound + 1);

        VirtualProtect(memory->dispatchSound, 4, oldProtection, nullptr);
    }

    surface.hookAt(67, &lockCursor);

    if constexpr (std::is_same_v<HookType, MinHook>)
        MH_EnableHook(MH_ALL_HOOKS);
}
 
static DWORD WINAPI unload(HMODULE moduleHandle) noexcept
{
    Sleep(100);

    interfaces->inputSystem->enableInput(true);
    EventListener::remove();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    antiDetect.uninstall();
}


void Hooks::uninstall() noexcept
{
    Misc::updateEventListeners(true);
    Visuals::updateEventListeners(true);

    if constexpr (std::is_same_v<HookType, MinHook>) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }

    bspQuery.restore();
    client.restore();
    clientMode.restore();
    engine.restore();
    fileSystem.restore();
    inventory.restore();
    inventoryManager.restore();
    modelRender.restore();
    panoramaMarshallHelper.restore();
    sound.restore();
    surface.restore();
    svCheats.restore();
    viewRender.restore();

    Netvars::restore();

    Glow::clearCustomObjects();
    InventoryChanger::clearInventory();

    SetWindowLongPtrW(window, GWLP_WNDPROC, LONG_PTR(originalWndProc));
    **reinterpret_cast<void***>(memory->present) = originalPresent;
    **reinterpret_cast<void***>(memory->reset) = originalReset;

    if (DWORD oldProtection; VirtualProtect(memory->dispatchSound, 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        *memory->dispatchSound = uintptr_t(originalDispatchSound) - uintptr_t(memory->dispatchSound + 1);
        VirtualProtect(memory->dispatchSound, 4, oldProtection, nullptr);
    }

    if (HANDLE thread = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(unload), moduleHandle, 0, nullptr))
        CloseHandle(thread);
}

void Hooks::callOriginalDrawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);
}