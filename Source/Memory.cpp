#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <limits>
#include <string_view>
#include <utility>

#include <Windows.h>
#include <Psapi.h>

#include "Helpers.h"
#include "Interfaces.h"
#include "Memory.h"
#include "SDK/LocalPlayer.h"

template <typename T>
static constexpr auto relativeToAbsolute(uintptr_t address) noexcept
{
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

struct ModuleInfo {
    void* base;
    std::size_t size;
};

static ModuleInfo getModuleInformation(const char* name) noexcept
{
    if (HMODULE handle = GetModuleHandleA(name)) {
        if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), handle, &moduleInfo, sizeof(moduleInfo)))
            return ModuleInfo{ moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage };
    }
    return {};
}

[[nodiscard]] static auto generateBadCharTable(std::string_view pattern) noexcept
{
    assert(!pattern.empty());

    std::array<std::size_t, (std::numeric_limits<std::uint8_t>::max)() + 1> table;

    auto lastWildcard = pattern.rfind('?');
    if (lastWildcard == std::string_view::npos)
        lastWildcard = 0;

    const auto defaultShift = (std::max)(std::size_t(1), pattern.length() - 1 - lastWildcard);
    table.fill(defaultShift);

    for (auto i = lastWildcard; i < pattern.length() - 1; ++i)
        table[static_cast<std::uint8_t>(pattern[i])] = pattern.length() - 1 - i;

    return table;
}

template <bool ReportNotFound = true>
static std::uintptr_t findPattern(ModuleInfo moduleInfo, std::string_view pattern) noexcept
{
    static auto id = 0;
    ++id;

    if (moduleInfo.base && moduleInfo.size) {
        const auto lastIdx = pattern.length() - 1;
        const auto badCharTable = generateBadCharTable(pattern);

        auto start = static_cast<const char*>(moduleInfo.base);
        const auto end = start + moduleInfo.size - pattern.length();

        while (start <= end) {
            int i = lastIdx;
            while (i >= 0 && (pattern[i] == '?' || start[i] == pattern[i]))
                --i;

            if (i < 0)
                return reinterpret_cast<std::uintptr_t>(start);

            start += badCharTable[static_cast<std::uint8_t>(start[lastIdx])];
        }
    }

    assert(false); 
    Helpers::messageBox("Spatial", ("Failed to find pattern #" + std::to_string(id) + '!').c_str());
    return 0;
}

template <bool ReportNotFound = true>
static std::uintptr_t findPattern(const char* moduleName, std::string_view pattern) noexcept
{
    return findPattern<ReportNotFound>(getModuleInformation(moduleName), pattern);
}

Memory::Memory() noexcept
{
    present = findPattern("gameoverlayrenderer", "\xFF\x15????\x8B\xF0\x85\xFF") + 2;
    reset = findPattern("gameoverlayrenderer", "\xC7\x45?????\xFF\x15????\x8B\xD8") + 9;

    clientMode = **reinterpret_cast<ClientMode***>((*reinterpret_cast<uintptr_t**>(interfaces->client))[10] + 5);
    input = *reinterpret_cast<Input**>((*reinterpret_cast<uintptr_t**>(interfaces->client))[16] + 1);
    globalVars = **reinterpret_cast<GlobalVars***>((*reinterpret_cast<uintptr_t**>(interfaces->client))[11] + 10);
    glowObjectManager = *reinterpret_cast<GlowObjectManager**>(findPattern(CLIENT_DLL, "\x0F\x11\x05????\x83\xC8\x01") + 3);
    disablePostProcessing = *reinterpret_cast<bool**>(findPattern(CLIENT_DLL, "\x83\xEC\x4C\x80\x3D") + 5);
    loadSky = relativeToAbsolute<decltype(loadSky)>(findPattern(ENGINE_DLL, "\xE8????\x84\xC0\x74\x2D\xA1") + 1);
    setClanTag = reinterpret_cast<decltype(setClanTag)>(findPattern(ENGINE_DLL, "\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15"));
    lineGoesThroughSmoke = relativeToAbsolute<decltype(lineGoesThroughSmoke)>(findPattern(CLIENT_DLL, "\xE8????\x8B\x4C\x24\x30\x33\xD2") + 1);
    smokeHandles = *reinterpret_cast<decltype(smokeHandles)*>(std::uintptr_t(lineGoesThroughSmoke) + 46);
    cameraThink = findPattern(CLIENT_DLL, "\x85\xC0\x75\x30\x38\x86");
    getSequenceActivity = reinterpret_cast<decltype(getSequenceActivity)>(findPattern(CLIENT_DLL, "\x55\x8B\xEC\x53\x8B\x5D\x08\x56\x8B\xF1\x83"));
    isOtherEnemy = relativeToAbsolute<decltype(isOtherEnemy)>(findPattern(CLIENT_DLL, "\x8B\xCE\xE8????\x02\xC0") + 3);
    auto temp = reinterpret_cast<std::uintptr_t*>(findPattern(CLIENT_DLL, "\xB9????\xE8????\x8B\x5D\x08") + 1);
    hud = *temp;
    findHudElement = relativeToAbsolute<decltype(findHudElement)>(reinterpret_cast<uintptr_t>(temp) + 5);
    clearHudWeapon = relativeToAbsolute<decltype(clearHudWeapon)>(findPattern(CLIENT_DLL, "\xE8????\x8B\xF0\xC6\x44\x24??\xC6\x44\x24") + 1);
    itemSystem = relativeToAbsolute<decltype(itemSystem)>(findPattern(CLIENT_DLL, "\xE8????\x0F\xB7\x0F") + 1);
    setAbsOrigin = relativeToAbsolute<decltype(setAbsOrigin)>(findPattern(CLIENT_DLL, "\xE8????\xEB\x19\x8B\x07") + 1);
    insertIntoTree = findPattern(CLIENT_DLL, "\x56\x52\xFF\x50\x18") + 5;
    dispatchSound = reinterpret_cast<int*>(findPattern(ENGINE_DLL, "\x74\x0B\xE8????\x8B\x3D") + 3);
    traceToExit = findPattern(CLIENT_DLL, "\x55\x8B\xEC\x83\xEC\x4C\xF3\x0F\x10\x75");
    viewRender = **reinterpret_cast<ViewRender***>(findPattern(CLIENT_DLL, "\x8B\x0D????\xFF\x75\x0C\x8B\x45\x08") + 2);
    viewRenderBeams = *reinterpret_cast<ViewRenderBeams**>(findPattern(CLIENT_DLL, "\xB9????\x0F\x11\x44\x24?\xC7\x44\x24?????\xF3\x0F\x10\x84\x24") + 1);
    drawScreenEffectMaterial = relativeToAbsolute<uintptr_t>(findPattern(CLIENT_DLL, "\xE8????\x83\xC4\x0C\x8D\x4D\xF8") + 1);
    submitReportFunction = findPattern(CLIENT_DLL, "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x28\x8B\x4D\x08");
    const auto tier0 = GetModuleHandleW(L"tier0");
    debugMsg = reinterpret_cast<decltype(debugMsg)>(GetProcAddress(tier0, "Msg"));
    conColorMsg = reinterpret_cast<decltype(conColorMsg)>(GetProcAddress(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ"));
    vignette = *reinterpret_cast<float**>(findPattern(CLIENT_DLL, "\x0F\x11\x05????\xF3\x0F\x7E\x87") + 3) + 1;
    equipWearable = reinterpret_cast<decltype(equipWearable)>(findPattern(CLIENT_DLL, "\x55\x8B\xEC\x83\xEC\x10\x53\x8B\x5D\x08\x57\x8B\xF9"));
    predictionRandomSeed = *reinterpret_cast<int**>(findPattern(CLIENT_DLL, "\x8B\x0D????\xBA????\xE8????\x83\xC4\x04") + 2);
    moveData = **reinterpret_cast<MoveData***>(findPattern(CLIENT_DLL, "\xA1????\xF3\x0F\x59\xCD") + 1);
    moveHelper = **reinterpret_cast<MoveHelper***>(findPattern(CLIENT_DLL, "\x8B\x0D????\x8B\x45?\x51\x8B\xD4\x89\x02\x8B\x01") + 2);
    keyValuesFromString = relativeToAbsolute<decltype(keyValuesFromString)>(findPattern(CLIENT_DLL, "\xE8????\x83\xC4\x04\x89\x45\xD8") + 1);
    keyValuesFindKey = relativeToAbsolute<decltype(keyValuesFindKey)>(findPattern(CLIENT_DLL, "\xE8????\xF7\x45") + 1);
    keyValuesSetString = relativeToAbsolute<decltype(keyValuesSetString)>(findPattern(CLIENT_DLL, "\xE8????\x89\x77\x38") + 1);
    weaponSystem = *reinterpret_cast<WeaponSystem**>(findPattern(CLIENT_DLL, "\x8B\x35????\xFF\x10\x0F\xB7\xC0") + 2);
    getPlayerViewmodelArmConfigForPlayerModel = relativeToAbsolute<decltype(getPlayerViewmodelArmConfigForPlayerModel)>(findPattern(CLIENT_DLL, "\xE8????\x89\x87????\x6A") + 1);
    getEventDescriptor = relativeToAbsolute<decltype(getEventDescriptor)>(findPattern(ENGINE_DLL, "\xE8????\x8B\xD8\x85\xDB\x75\x27") + 1);
    activeChannels = *reinterpret_cast<ActiveChannels**>(findPattern(ENGINE_DLL, "\x8B\x1D????\x89\x5C\x24\x48") + 2);
    channels = *reinterpret_cast<Channel**>(findPattern(ENGINE_DLL, "\x81\xC2????\x8B\x72\x54") + 2);
    playerResource = *reinterpret_cast<PlayerResource***>(findPattern(CLIENT_DLL, "\x74\x30\x8B\x35????\x85\xF6") + 4);
    getDecoratedPlayerName = relativeToAbsolute<decltype(getDecoratedPlayerName)>(findPattern(CLIENT_DLL, "\xE8????\x66\x83\x3E") + 1);
    scopeDust = findPattern(CLIENT_DLL, "\xFF\x50\x3C\x8B\x4C\x24\x20") + 3;
    scopeArc = findPattern(CLIENT_DLL, "\x8B\x0D????\xFF\xB7????\x8B\x01\xFF\x90????\x8B\x7C\x24\x1C");
    demoOrHLTV = findPattern(CLIENT_DLL, "\x84\xC0\x75\x09\x38\x05");
    money = findPattern(CLIENT_DLL, "\x84\xC0\x75\x0C\x5B");
    demoFileEndReached = findPattern(CLIENT_DLL, "\x8B\xC8\x85\xC9\x74\x1F\x80\x79\x10");
    plantedC4s = *reinterpret_cast<decltype(plantedC4s)*>(findPattern(CLIENT_DLL, "\x7E\x2C\x8B\x15") + 4);
    gameRules = *reinterpret_cast<Entity***>(findPattern(CLIENT_DLL, "\x8B\xEC\x8B\x0D????\x85\xC9\x74\x07") + 4);
    registeredPanoramaEvents = reinterpret_cast<decltype(registeredPanoramaEvents)>(*reinterpret_cast<std::uintptr_t*>(findPattern(CLIENT_DLL, "\xE8????\xA1????\xA8\x01\x75\x21") + 6) - 36);
    makePanoramaSymbolFn = relativeToAbsolute<decltype(makePanoramaSymbolFn)>(findPattern(CLIENT_DLL, "\xE8????\x0F\xB7\x45\x0E\x8D\x4D\x0E") + 1);
    inventoryManager = *reinterpret_cast<InventoryManager**>(findPattern(CLIENT_DLL, "\x8D\x44\x24\x28\xB9") + 5);
    createEconItemSharedObject = *reinterpret_cast<decltype(createEconItemSharedObject)*>(findPattern(CLIENT_DLL, "\x55\x8B\xEC\x83\xEC\x1C\x8D\x45\xE4\xC7\x45") + 20);
    addEconItem = relativeToAbsolute<decltype(addEconItem)>(findPattern(CLIENT_DLL, "\xE8????\x84\xC0\x74\xE7") + 1);
    clearInventoryImageRGBA = reinterpret_cast<decltype(clearInventoryImageRGBA)>(findPattern(CLIENT_DLL, "\x55\x8B\xEC\x81\xEC????\x57\x8B\xF9\xC7\x47"));
    panoramaMarshallHelper = *reinterpret_cast<decltype(panoramaMarshallHelper)*>(findPattern(CLIENT_DLL, "\x68????\x8B\xC8\xE8????\x8D\x4D\xF4\xFF\x15????\x8B\xCF\xFF\x15????\x5F\x5E\x8B\xE5\x5D\xC3") + 1);
    setStickerToolSlotGetArgAsNumberReturnAddress = findPattern(CLIENT_DLL, "\xFF\xD2\xDD\x5C\x24\x10\xF2\x0F\x2C\x7C\x24") + 2;
    setStickerToolSlotGetArgAsStringReturnAddress = setStickerToolSlotGetArgAsNumberReturnAddress - 49;
    wearItemStickerGetArgAsNumberReturnAddress = findPattern(CLIENT_DLL, "\xDD\x5C\x24\x18\xF2\x0F\x2C\x7C\x24?\x85\xFF");
    wearItemStickerGetArgAsStringReturnAddress = wearItemStickerGetArgAsNumberReturnAddress - 80;
    setNameToolStringGetArgAsStringReturnAddress = findPattern(CLIENT_DLL, "\x8B\xF8\xC6\x45\x08?\x33\xC0");
    clearCustomNameGetArgAsStringReturnAddress = findPattern(CLIENT_DLL, "\xFF\x50\x1C\x8B\xF0\x85\xF6\x74\x21") + 3;
    deleteItemGetArgAsStringReturnAddress = findPattern(CLIENT_DLL, "\x85\xC0\x74\x22\x51");
    setStatTrakSwapToolItemsGetArgAsStringReturnAddress1 = findPattern(CLIENT_DLL, "\x85\xC0\x74\x7E\x8B\xC8\xE8????\x8B\x37");
    setStatTrakSwapToolItemsGetArgAsStringReturnAddress2 = setStatTrakSwapToolItemsGetArgAsStringReturnAddress1 + 44;
    acknowledgeNewItemByItemIDGetArgAsStringReturnAddress = findPattern(CLIENT_DLL, "\x85\xC0\x74\x33\x8B\xC8\xE8????\xB9");

    findOrCreateEconItemViewForItemID = relativeToAbsolute<decltype(findOrCreateEconItemViewForItemID)>(findPattern(CLIENT_DLL, "\xE8????\x8B\xCE\x83\xC4\x08") + 1);
    getInventoryItemByItemID = relativeToAbsolute<decltype(getInventoryItemByItemID)>(findPattern(CLIENT_DLL, "\xE8????\x8B\x33\x8B\xD0") + 1);
    useToolGetArgAsStringReturnAddress = findPattern(CLIENT_DLL, "\x85\xC0\x0F\x84????\x8B\xC8\xE8????\x8B\x37");
    useToolGetArg2AsStringReturnAddress = useToolGetArgAsStringReturnAddress + 52;
    getSOCData = relativeToAbsolute<decltype(getSOCData)>(findPattern(CLIENT_DLL, "\xE8????\x32\xC9") + 1);
    setCustomName = relativeToAbsolute<decltype(setCustomName)>(findPattern(CLIENT_DLL, "\xE8????\x8B\x46\x78\xC1\xE8\x0A\xA8\x01\x74\x13\x8B\x46\x34") + 1);
    setDynamicAttributeValueFn = findPattern(CLIENT_DLL, "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x3C\x53\x8B\x5D\x08\x56\x57\x6A");
    createBaseTypeCache = relativeToAbsolute<decltype(createBaseTypeCache)>(findPattern(CLIENT_DLL, "\xE8????\x8D\x4D\x0F") + 1);
    uiComponentInventory = *reinterpret_cast<void***>(findPattern(CLIENT_DLL, "\xC6\x44\x24??\x83\x3D") + 7);
    setItemSessionPropertyValue = relativeToAbsolute<decltype(setItemSessionPropertyValue)>(findPattern(CLIENT_DLL, "\xE8????\x8B\x4C\x24\x2C\x46") + 1);

    WriteUsercmdDeltaToBufferReturn = *(reinterpret_cast<void**>(findPattern(ENGINE_DLL, "\x84\xC0\x74\x04\xB0\x01\xEB\x02\x32\xC0\x8B\xFE\x46\x3B\xF3\x7E\xC9\x84\xC0\x0F\x84????")));
    WriteUsercmd = findPattern(CLIENT_DLL, "\x55\x8B\xEC\x83\xE4\xF8\x51\x53\x56\x8B\xD9\x8B\x0D");
    invertoryBlock = findPattern(CLIENT_DLL, "\x84\xC0\x75\x04\xB0\x01\x5E") - 2;
    fakePrime = reinterpret_cast<std::uint8_t*>(findPattern(CLIENT_DLL, "\x8B\x0D????\x85\xC9\x75\x04\x33\xC0\xEB\x1E"));
    relayCluster = *(std::string**)(findPattern("steamnetworkingsockets", "\xB8????\xB9????\x0F\x43") + 1);

    localPlayer.init(*reinterpret_cast<Entity***>(findPattern(CLIENT_DLL, "\xA1????\x89\x45\xBC\x85\xC0") + 1));
}
