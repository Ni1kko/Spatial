#include <algorithm>
#include <array>
#include <iomanip>
#include <mutex>
#include <numbers>
#include <numeric>
#include <sstream>
#include <vector>

#include "../Encryption/xorstr.hpp"

#include "../Menu/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../Menu/imgui/imgui_internal.h"
#include "../Menu/imgui/imgui_stdlib.h"

#include "../ConfigStructs.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../ProtobufReader.h"

#include "EnginePrediction.h"
#include "Misc.h"

#include "../SDK/ClassId.h"
#include "../SDK/Client.h"
#include "../SDK/ClientClass.h"
#include "../SDK/ClientMode.h"
#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/Localize.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/PlayerResource.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/Panorama.h"
#include "../SDK/Platform.h"
#include "../SDK/UserCmd.h"
#include "../SDK/UtlVector.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponId.h"
#include "../SDK/WeaponSystem.h"

#include "../SDK/Steam.h"

#include <Menu/Menu.h>
#include "../Helpers.h"
#include "../Hooks.h"
#include "../GameData.h"

#include <Menu/imguiCustom.h>

/////////////////////////////////////////////////////////////////
// Vars
/////////////////////////////////////////////////////////////////

static std::vector<std::uint64_t> reportedPlayers;
static int reportbotRound;

/////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////

struct PreserveKillfeed {
    bool enabled = false;
    bool onlyHeadshots = false;
};

struct OffscreenEnemies : ColorToggle {
    OffscreenEnemies() : ColorToggle{ 0.0f, 0.0f, 0.0f, 1.0f } {}
    HealthBar healthBar;
};

struct PurchaseList {
    bool enabled = false;
    bool onlyDuringFreezeTime = false;
    bool showPrices = false;
    bool noTitleBar = false;

    enum Mode {
        Details = 0,
        Summary
    };
    int mode = Details;
};

struct MiscConfig {
    MiscConfig() { clanTag[0] = '\0'; }

    bool antiAfkKick{ false };
    bool autoDisconnect{ false };
    bool customClanTag{ false };
    bool clocktag{ false };
    bool animatedClanTag{ false };
    bool autoPistol{ false };
    bool autoReload{ false };
    bool autoAccept{ false };
    bool radarHack{ false };
    bool revealRanks{ false };
    bool revealMoney{ false };
    bool revealSuspect{ false };
    bool revealVotes{ false };
    bool fixAnimationLOD{ false }; 
    bool disableModelOcclusion{ false };
    bool nameStealer{ false };
    bool fakePrime{ false };
    bool disablePanoramablur{ false };
    bool killMessage{ false };
    bool nadePredict{ false };
    bool fixTabletSignal{ false };
    bool quickReload{ false };
    bool prepareRevolver{ false };
    bool oppositeHandKnife = false;

    PreserveKillfeed preserveKillfeed;
    char clanTag[16];
    ColorToggleThickness noscopeCrosshair;
    ColorToggleThickness recoilCrosshair;

    struct SpectatorList {
        bool enabled = false;
        bool noTitleBar = false;
        ImVec2 pos;
        ImVec2 size{ 200.0f, 200.0f };
    };

    SpectatorList spectatorList;
    std::string killMessageString{ "Ez" };
    bool fakeMsgToggled{ false }; int fakeMsgColor{ 6 }; std::string fakeMsgText{ "Cheater has been permanently banned from official CS:GO servers." };
    ColorToggle3 bombTimer{ 0.0f, 0.0f, 0.0f };
    KeyBind prepareRevolverKey;
    bool quickHealthshot = false; KeyBind quickHealthshotKey;
    PurchaseList purchaseList;

    struct Reportbot {
        bool enabled = false;
        bool textAbuse = false;
        bool griefing = false;
        bool wallhack = true;
        bool aimbot = true;
        bool other = true;
        int target = 0;
        int delay = 1;
        int rounds = 1;
    } reportbot;

    OffscreenEnemies offscreenEnemies;
    int forceRelayCluster{ 0 };
    bool bypassSvPure{ false };
} miscConfig;
/////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////

bool Misc::shouldRevealMoney() noexcept
{
    return miscConfig.revealMoney;
}

bool Misc::shouldRevealSuspect() noexcept
{
    return miscConfig.revealSuspect;
}

bool Misc::shouldDisableModelOcclusion() noexcept
{
    return miscConfig.disableModelOcclusion;
}

bool Misc::isRadarHackOn() noexcept
{
    return miscConfig.radarHack;
}

void Misc::updateClanTag(bool tagChanged) noexcept
{
    static std::string clanTag;

    if (tagChanged) {
        clanTag = miscConfig.clanTag;
        if (!clanTag.empty() && clanTag.front() != ' ' && clanTag.back() != ' ')
            clanTag.push_back(' ');
        return;
    }
    
    static auto lastTime = 0.0f;

    if (miscConfig.clocktag) {
        if (memory->globalVars->realtime - lastTime < 1.0f)
            return;

        const auto time = std::time(nullptr);
        const auto localTime = std::localtime(&time);
        char s[11];
        s[0] = '\0';
        snprintf(s, sizeof(s), "[%02d:%02d:%02d]", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
        lastTime = memory->globalVars->realtime;
        memory->setClanTag(s, s);
    } else if (miscConfig.customClanTag) {
        if (memory->globalVars->realtime - lastTime < 0.6f)
            return;

        if (miscConfig.animatedClanTag && !clanTag.empty()) {
            if (const auto offset = Helpers::utf8SeqLen(clanTag[0]); offset <= clanTag.length())
                std::rotate(clanTag.begin(), clanTag.begin() + offset, clanTag.end());
        }
        lastTime = memory->globalVars->realtime;
        memory->setClanTag(clanTag.c_str(), clanTag.c_str());
    }
}

void Misc::spectatorList() noexcept
{
    if (!miscConfig.spectatorList.enabled)
        return;

    GameData::Lock lock;

    const auto& observers = GameData::observers();

    if (std::ranges::none_of(observers, [](const auto& obs) { return obs.targetIsLocalPlayer; }) && !gui->isOpen())
        return;

    if (miscConfig.spectatorList.pos != ImVec2{}) {
        ImGui::SetNextWindowPos(miscConfig.spectatorList.pos);
        miscConfig.spectatorList.pos = {};
    }

    if (miscConfig.spectatorList.size != ImVec2{}) {
        ImGui::SetNextWindowSize(ImClamp(miscConfig.spectatorList.size, {}, ImGui::GetIO().DisplaySize));
        miscConfig.spectatorList.size = {};
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    if (!gui->isOpen())
        windowFlags |= ImGuiWindowFlags_NoInputs;
    if (miscConfig.spectatorList.noTitleBar)
        windowFlags |= ImGuiWindowFlags_NoTitleBar;

    if (!gui->isOpen())
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetColorU32(ImGuiCol_TitleBgActive));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
    ImGui::Begin("Spectator list", nullptr, windowFlags);
    ImGui::PopStyleVar();

    if (!gui->isOpen())
        ImGui::PopStyleColor();

    for (const auto& observer : observers) {
        if (!observer.targetIsLocalPlayer)
            continue;

        if (const auto it = std::ranges::find(GameData::players(), observer.playerHandle, &PlayerData::handle); it != GameData::players().cend()) {
            if (const auto texture = it->getAvatarTexture()) {
                const auto textSize = ImGui::CalcTextSize(it->name.c_str());
                ImGui::Image(texture, ImVec2(textSize.y, textSize.y), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.3f));
                ImGui::SameLine();
                ImGui::TextWrapped("%s", it->name.c_str());
            }
        }
    }

    ImGui::End();
}

static void drawCrosshair(ImDrawList* drawList, const ImVec2& pos, ImU32 color) noexcept
{
    // dot
    drawList->AddRectFilled(pos - ImVec2{ 1, 1 }, pos + ImVec2{ 2, 2 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(pos, pos + ImVec2{ 1, 1 }, color);

    // left
    drawList->AddRectFilled(ImVec2{ pos.x - 11, pos.y - 1 }, ImVec2{ pos.x - 3, pos.y + 2 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x - 10, pos.y }, ImVec2{ pos.x - 4, pos.y + 1 }, color);

    // right
    drawList->AddRectFilled(ImVec2{ pos.x + 4, pos.y - 1 }, ImVec2{ pos.x + 12, pos.y + 2 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x + 5, pos.y }, ImVec2{ pos.x + 11, pos.y + 1 }, color);

    // top (left with swapped x/y offsets)
    drawList->AddRectFilled(ImVec2{ pos.x - 1, pos.y - 11 }, ImVec2{ pos.x + 2, pos.y - 3 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x, pos.y - 10 }, ImVec2{ pos.x + 1, pos.y - 4 }, color);

    // bottom (right with swapped x/y offsets)
    drawList->AddRectFilled(ImVec2{ pos.x - 1, pos.y + 4 }, ImVec2{ pos.x + 2, pos.y + 12 }, color & IM_COL32_A_MASK);
    drawList->AddRectFilled(ImVec2{ pos.x, pos.y + 5 }, ImVec2{ pos.x + 1, pos.y + 11 }, color);
}

void Misc::noscopeCrosshair(ImDrawList* drawList) noexcept
{
    if (!miscConfig.noscopeCrosshair.asColorToggle().enabled)
        return;

    {
        GameData::Lock lock;
        if (const auto& local = GameData::local(); !local.exists || !local.alive || !local.noScope)
            return;
    }

    drawCrosshair(drawList, ImGui::GetIO().DisplaySize / 2, Helpers::calculateColor(miscConfig.noscopeCrosshair.asColorToggle().asColor4()));
}

void Misc::recoilCrosshair(ImDrawList* drawList) noexcept
{
    if (!miscConfig.recoilCrosshair.asColorToggle().enabled)
        return;

    GameData::Lock lock;
    const auto& localPlayerData = GameData::local();

    if (!localPlayerData.exists || !localPlayerData.alive)
        return;

    if (!localPlayerData.shooting)
        return;

    if (ImVec2 pos; Helpers::worldToScreenPixelAligned(localPlayerData.aimPunch, pos))
        drawCrosshair(drawList, pos, Helpers::calculateColor(miscConfig.recoilCrosshair.asColorToggle().asColor4()));
}

void Misc::prepareRevolver(UserCmd* cmd) noexcept
{
    constexpr auto timeToTicks = [](float time) {  return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick); };
    constexpr float revolverPrepareTime{ 0.234375f };

    static float readyTime;
    if (miscConfig.prepareRevolver && localPlayer && (!miscConfig.prepareRevolverKey.isSet() || miscConfig.prepareRevolverKey.isDown())) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->itemDefinitionIndex() == WeaponId::Revolver) {
            if (!readyTime) readyTime = memory->globalVars->serverTime() + revolverPrepareTime;
            auto ticksToReady = timeToTicks(readyTime - memory->globalVars->serverTime() - interfaces->engine->getNetworkChannel()->getLatency(0));
            if (ticksToReady > 0 && ticksToReady <= timeToTicks(revolverPrepareTime))
                cmd->buttons |= UserCmd::IN_ATTACK;
            else
                readyTime = 0.0f;
        }
    }
}

void Misc::drawBombTimer() noexcept
{
    if (!miscConfig.bombTimer.enabled)
        return;

    GameData::Lock lock;
    
    const auto& plantedC4 = GameData::plantedC4();
    if (plantedC4.blowTime == 0.0f && !gui->isOpen())
        return;

    if (!gui->isOpen()) {
        ImGui::SetNextWindowBgAlpha(0.3f);
    }

    static float windowWidth = 200.0f;
    ImGui::SetNextWindowPos({ (ImGui::GetIO().DisplaySize.x - 200.0f) / 2.0f, 60.0f }, ImGuiCond_Once);
    ImGui::SetNextWindowSize({ windowWidth, 0 }, ImGuiCond_Once);

    if (!gui->isOpen())
        ImGui::SetNextWindowSize({ windowWidth, 0 });

    ImGui::SetNextWindowSizeConstraints({ 0, -1 }, { FLT_MAX, -1 });
    ImGui::Begin("Bomb Timer", nullptr, ImGuiWindowFlags_NoTitleBar | (gui->isOpen() ? 0 : ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration));

    std::ostringstream ss; ss << "Bomb on " << (!plantedC4.bombsite ? 'A' : 'B') << " : " << std::fixed << std::showpoint << std::setprecision(3) << (std::max)(plantedC4.blowTime - memory->globalVars->currenttime, 0.0f) << " s";

    ImGui::textUnformattedCentered(ss.str().c_str());

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Helpers::calculateColor(miscConfig.bombTimer.asColor3()));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f });
    ImGui::progressBarFullWidth((plantedC4.blowTime - memory->globalVars->currenttime) / plantedC4.timerLength, 5.0f);

    if (plantedC4.defuserHandle != -1) {
        const bool canDefuse = plantedC4.blowTime >= plantedC4.defuseCountDown;

        if (plantedC4.defuserHandle == GameData::local().handle) {
            if (canDefuse) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                ImGui::textUnformattedCentered("You can defuse!");
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                ImGui::textUnformattedCentered("You can not defuse!");
            }
            ImGui::PopStyleColor();
        } else if (const auto defusingPlayer = GameData::playerByHandle(plantedC4.defuserHandle)) {
            std::ostringstream ss; ss << defusingPlayer->name << " is defusing: " << std::fixed << std::showpoint << std::setprecision(3) << (std::max)(plantedC4.defuseCountDown - memory->globalVars->currenttime, 0.0f) << " s";

            ImGui::textUnformattedCentered(ss.str().c_str());

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, canDefuse ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
            ImGui::progressBarFullWidth((plantedC4.defuseCountDown - memory->globalVars->currenttime) / plantedC4.defuseLength, 5.0f);
            ImGui::PopStyleColor();
        }
    }

    windowWidth = ImGui::GetCurrentWindow()->SizeFull.x;

    ImGui::PopStyleColor(2);
    ImGui::End();
}

void Misc::stealNames() noexcept
{
    if (!miscConfig.nameStealer)
        return;

    if (!localPlayer)
        return;

    static std::vector<int> stolenIds;

    for (int i = 1; i <= memory->globalVars->maxClients; ++i) {
        const auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity == localPlayer.get())
            continue;

        PlayerInfo playerInfo;
        if (!interfaces->engine->getPlayerInfo(entity->index(), playerInfo))
            continue;

        if (playerInfo.fakeplayer || std::ranges::find(stolenIds, playerInfo.userId) != stolenIds.cend())
            continue;

        if (changeName(false, (std::string{ playerInfo.name } +'\x1').c_str(), 1.0f))
            stolenIds.push_back(playerInfo.userId);

        return;
    }
    stolenIds.clear();
}

void Misc::disablePanoramablur() noexcept
{
    static auto blur = interfaces->cvar->findVar("@panorama_disable_blur");
    blur->setValue(miscConfig.disablePanoramablur);
}

void Misc::quickReload(UserCmd* cmd) noexcept
{
    if (miscConfig.quickReload) {
        static Entity* reloadedWeapon{ nullptr };

        if (reloadedWeapon) {
            for (auto weaponHandle : localPlayer->weapons()) {
                if (weaponHandle == -1)
                    break;

                if (interfaces->entityList->getEntityFromHandle(weaponHandle) == reloadedWeapon) {
                    cmd->weaponselect = reloadedWeapon->index();
                    cmd->weaponsubtype = reloadedWeapon->getWeaponSubType();
                    break;
                }
            }
            reloadedWeapon = nullptr;
        }

        if (auto activeWeapon{ localPlayer->getActiveWeapon() }; activeWeapon && activeWeapon->isInReload() && activeWeapon->clip() == activeWeapon->getWeaponData()->maxClip) {
            reloadedWeapon = activeWeapon;

            for (auto weaponHandle : localPlayer->weapons()) {
                if (weaponHandle == -1)
                    break;

                if (auto weapon{ interfaces->entityList->getEntityFromHandle(weaponHandle) }; weapon && weapon != reloadedWeapon) {
                    cmd->weaponselect = weapon->index();
                    cmd->weaponsubtype = weapon->getWeaponSubType();
                    break;
                }
            }
        }
    }
}

bool Misc::changeName(bool reconnect, const char* newName, float delay) noexcept
{
    static auto exploitInitialized{ false };

    static auto name{ interfaces->cvar->findVar("name") };

    if (reconnect) {
        exploitInitialized = false;
        return false;
    }

    if (!exploitInitialized && interfaces->engine->isInGame()) {
        if (PlayerInfo playerInfo; localPlayer && interfaces->engine->getPlayerInfo(localPlayer->index(), playerInfo) && (!strcmp(playerInfo.name, "?empty") || !strcmp(playerInfo.name, "\n\xAD\xAD\xAD"))) {
            exploitInitialized = true;
        } else {
            name->onChangeCallbacks.size = 0;
            name->setValue("\n\xAD\xAD\xAD");
            return false;
        }
    }

    if (static auto nextChangeTime = 0.0f; nextChangeTime <= memory->globalVars->realtime) {
        name->setValue(newName);
        nextChangeTime = memory->globalVars->realtime + delay;
        return true;
    }
    return false;
}

void Misc::fakeMessage(bool set) noexcept
{
    static bool shouldSet = false;

    if (!miscConfig.fakeMsgToggled && shouldSet) {
        changeName(false, "", 5.0f);
        shouldSet = false;
    }

    if (set || miscConfig.fakeMsgToggled)
        shouldSet = set;
     
    if (shouldSet && interfaces->engine->isInGame() && changeName(false, std::string{ "\x1\xB" }.append(std::string{ static_cast<char>(miscConfig.fakeMsgColor + 1) }).append(miscConfig.fakeMsgText).append("\x1").c_str(), 5.0f))
        shouldSet = false;
}

void Misc::nadePredict() noexcept
{
    static auto nadeVar{ interfaces->cvar->findVar("cl_grenadepreview") };

    nadeVar->onChangeCallbacks.size = 0;
    nadeVar->setValue(miscConfig.nadePredict);
}

void Misc::fixTabletSignal() noexcept
{
    if (miscConfig.fixTabletSignal && localPlayer) {
        if (auto activeWeapon{ localPlayer->getActiveWeapon() }; activeWeapon && activeWeapon->getClientClass()->classId == ClassId::CTablet)
            activeWeapon->tabletReceptionIsBlocked() = false;
    }
}

void Misc::killMessage(GameEvent& event) noexcept
{
    if (!miscConfig.killMessage)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto localUserId = localPlayer->getUserId(); event.getInt("attacker") != localUserId || event.getInt("userid") == localUserId)
        return;

    std::string cmd = "say \"";
    cmd += miscConfig.killMessageString;
    cmd += '"';
    interfaces->engine->clientCmdUnrestricted(cmd.c_str());
}

void Misc::antiAfkKick(UserCmd* cmd) noexcept
{
    if (miscConfig.antiAfkKick && cmd->commandNumber % 2)
        cmd->buttons |= 1 << 27;
}

void Misc::fixAnimationLOD(FrameStage stage) noexcept
{
    if (miscConfig.fixAnimationLOD && stage == FrameStage::RENDER_START) {
        if (!localPlayer)
            return;
        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            Entity* entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()) continue;
            *reinterpret_cast<int*>(entity + 0xA28) = 0;
            *reinterpret_cast<int*>(entity + 0xA30) = memory->globalVars->framecount;
        }
    }
}

void Misc::autoPistol(UserCmd* cmd) noexcept
{
    if (miscConfig.autoPistol && localPlayer) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->isPistol() && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime()) {
            if (activeWeapon->itemDefinitionIndex() == WeaponId::Revolver)
                cmd->buttons &= ~UserCmd::IN_ATTACK2;
            else
                cmd->buttons &= ~UserCmd::IN_ATTACK;
        }
    }
}

void Misc::autoReload(UserCmd* cmd) noexcept
{
    if (miscConfig.autoReload && localPlayer) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && getWeaponIndex(activeWeapon->itemDefinitionIndex()) && !activeWeapon->clip())
            cmd->buttons &= ~(UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2);
    }
}

void Misc::revealRanks(UserCmd* cmd) noexcept
{
    if (miscConfig.revealRanks && cmd->buttons & UserCmd::IN_SCORE)
        interfaces->client->dispatchUserMessage(50, 0, 0, nullptr);
}

void Misc::autoDisconnect() noexcept
{
    if (miscConfig.autoDisconnect)
        interfaces->engine->clientCmdUnrestricted(xorstr_("disconnect"));
}

void Misc::purchaseList(GameEvent* event) noexcept
{
    static std::mutex mtx;
    std::scoped_lock _{ mtx };

    struct PlayerPurchases {
        int totalCost;
        std::unordered_map<std::string, int> items;
    };

    static std::unordered_map<int, PlayerPurchases> playerPurchases;
    static std::unordered_map<std::string, int> purchaseTotal;
    static int totalCost;

    static auto freezeEnd = 0.0f;

    if (event) {
        switch (fnv::hashRuntime(event->getName())) {
        case fnv::hash("item_purchase"): {
            if (const auto player = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid"))); player && localPlayer && localPlayer->isOtherEnemy(player)) {
                if (const auto definition = memory->itemSystem()->getItemSchema()->getItemDefinitionByName(event->getString("weapon"))) {
                    auto& purchase = playerPurchases[player->handle()];
                    if (const auto weaponInfo = memory->weaponSystem->getWeaponInfo(definition->getWeaponId())) {
                        purchase.totalCost += weaponInfo->price;
                        totalCost += weaponInfo->price;
                    }
                    const std::string weapon = interfaces->localize->findAsUTF8(definition->getItemBaseName());
                    ++purchaseTotal[weapon];
                    ++purchase.items[weapon];
                }
            }
            break;
        }
        case fnv::hash("round_start"):
            freezeEnd = 0.0f;
            playerPurchases.clear();
            purchaseTotal.clear();
            totalCost = 0;
            break;
        case fnv::hash("round_freeze_end"):
            freezeEnd = memory->globalVars->realtime;
            break;
        }
    } else {
        if (!miscConfig.purchaseList.enabled)
            return;

        if (static const auto mp_buytime = interfaces->cvar->findVar("mp_buytime"); (!interfaces->engine->isInGame() || freezeEnd != 0.0f && memory->globalVars->realtime > freezeEnd + (!miscConfig.purchaseList.onlyDuringFreezeTime ? mp_buytime->getFloat() : 0.0f) || playerPurchases.empty() || purchaseTotal.empty()) && !gui->isOpen())
            return;

        ImGui::SetNextWindowSize({ 200.0f, 200.0f }, ImGuiCond_Once);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
        if (!gui->isOpen())
            windowFlags |= ImGuiWindowFlags_NoInputs;
        if (miscConfig.purchaseList.noTitleBar)
            windowFlags |= ImGuiWindowFlags_NoTitleBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 0.5f, 0.5f });
        ImGui::Begin("Purchases", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (miscConfig.purchaseList.mode == PurchaseList::Details) {
            GameData::Lock lock;

            for (const auto& [handle, purchases] : playerPurchases) {
                std::string s;
                s.reserve(std::accumulate(purchases.items.begin(), purchases.items.end(), 0, [](int length, const auto& p) { return length + p.first.length() + 2; }));
                for (const auto& purchasedItem : purchases.items) {
                    if (purchasedItem.second > 1)
                        s += std::to_string(purchasedItem.second) + "x ";
                    s += purchasedItem.first + ", ";
                }

                if (s.length() >= 2)
                    s.erase(s.length() - 2);

                if (const auto player = GameData::playerByHandle(handle)) {
                    if (miscConfig.purchaseList.showPrices)
                        ImGui::TextWrapped("%s $%d: %s", player->name.c_str(), purchases.totalCost, s.c_str());
                    else
                        ImGui::TextWrapped("%s: %s", player->name.c_str(), s.c_str());
                }
            }
        } else if (miscConfig.purchaseList.mode == PurchaseList::Summary) {
            for (const auto& purchase : purchaseTotal)
                ImGui::TextWrapped("%d x %s", purchase.second, purchase.first.c_str());

            if (miscConfig.purchaseList.showPrices && totalCost > 0) {
                ImGui::Separator();
                ImGui::TextWrapped("Total: $%d", totalCost);
            }
        }
        ImGui::End();
    }
}

void Misc::oppositeHandKnife(FrameStage stage) noexcept
{
    if (!miscConfig.oppositeHandKnife)
        return;

    if (!localPlayer)
        return;

    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static const auto cl_righthand = interfaces->cvar->findVar("cl_righthand");
    static bool original;

    if (stage == FrameStage::RENDER_START) {
        original = cl_righthand->getInt();

        if (const auto activeWeapon = localPlayer->getActiveWeapon()) {
            if (const auto classId = activeWeapon->getClientClass()->classId; classId == ClassId::CKnife || classId == ClassId::CKnifeGG)
                cl_righthand->setValue(!original);
        }
    } else {
        cl_righthand->setValue(original);
    }
}

void Misc::runReportbot() noexcept
{
    if (!miscConfig.reportbot.enabled)
        return;

    if (!localPlayer)
        return;

    static auto lastReportTime = 0.0f;

    if (lastReportTime + miscConfig.reportbot.delay > memory->globalVars->realtime)
        return;

    if (reportbotRound >= miscConfig.reportbot.rounds)
        return;

    for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
        const auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity == localPlayer.get())
            continue;

        if (miscConfig.reportbot.target != 2 && (localPlayer->isOtherEnemy(entity) ? miscConfig.reportbot.target != 0 : miscConfig.reportbot.target != 1))
            continue;

        PlayerInfo playerInfo;
        if (!interfaces->engine->getPlayerInfo(i, playerInfo))
            continue;

        if (playerInfo.fakeplayer || std::ranges::find(reportedPlayers, playerInfo.xuid) != reportedPlayers.cend())
            continue;

        std::string report;

        if (miscConfig.reportbot.textAbuse)
            report += "textabuse,";
        if (miscConfig.reportbot.griefing)
            report += "grief,";
        if (miscConfig.reportbot.wallhack)
            report += "wallhack,";
        if (miscConfig.reportbot.aimbot)
            report += "aimbot,";
        if (miscConfig.reportbot.other)
            report += "speedhack,";

        if (!report.empty()) {
            memory->submitReport(std::to_string(playerInfo.xuid).c_str(), report.c_str());
            lastReportTime = memory->globalVars->realtime;
            reportedPlayers.push_back(playerInfo.xuid);
        }
        return;
    }

    reportedPlayers.clear();
    ++reportbotRound;
}

void Misc::resetReportbot() noexcept
{
    reportbotRound = 0;
    reportedPlayers.clear();
}

void Misc::preserveKillfeed(bool roundStart) noexcept
{
    if (!miscConfig.preserveKillfeed.enabled)
        return;

    static auto nextUpdate = 0.0f;

    if (roundStart) {
        nextUpdate = memory->globalVars->realtime + 10.0f;
        return;
    }

    if (nextUpdate > memory->globalVars->realtime)
        return;

    nextUpdate = memory->globalVars->realtime + 2.0f;

    const auto deathNotice = std::uintptr_t(memory->findHudElement(memory->hud, "CCSGO_HudDeathNotice"));
    if (!deathNotice)
        return;

    const auto deathNoticePanel = (*(UIPanel**)(*reinterpret_cast<std::uintptr_t*>(deathNotice - 20 + 88) + sizeof(std::uintptr_t)));

    const auto childPanelCount = deathNoticePanel->getChildCount();

    for (int i = 0; i < childPanelCount; ++i) {
        const auto child = deathNoticePanel->getChild(i);
        if (!child)
            continue;

        if (child->hasClass("DeathNotice_Killer") && (!miscConfig.preserveKillfeed.onlyHeadshots || child->hasClass("DeathNoticeHeadShot")))
            child->setAttributeFloat("SpawnTime", memory->globalVars->currenttime);
    }
}

void Misc::voteRevealer(GameEvent& event) noexcept
{
    if (!miscConfig.revealVotes)
        return;

    const auto entity = interfaces->entityList->getEntity(event.getInt("entityid"));
    if (!entity || !entity->isPlayer())
        return;
    
    const auto votedYes = event.getInt("vote_option") == 0;
    const auto isLocal = localPlayer && entity == localPlayer.get();
    
    std::string message = "";

    message.append(isLocal ? "You" : entity->getPlayerName());
    message.append(" ");
    message.append(Helpers::getColorByte(ColorByte::White));
    message.append(" Voted [");
    message.append(Helpers::getColorByte(ColorByte::Orange));
    message.append(votedYes ? "Yes" : "No");
    message.append(Helpers::getColorByte(ColorByte::White));
    message.append("]");

    Helpers::writeInGameChat(message.c_str(), isLocal ? ColorByte::LightGrey : ColorByte::Purple);

}

void Misc::onVoteStart(const void* data, int size) noexcept
{
    if (!miscConfig.revealVotes)
        return;

    constexpr auto voteName = [](int index) {
        switch (index) {
            case 0: return "Kick";
            case 1: return "Change Map";
            case 6: return "Surrender";
            case 13: return "Timeout";
            default: return "Unknown";
        }
    };

    const auto reader = ProtobufReader{ static_cast<const std::uint8_t*>(data), size };
    const auto entityIndex = reader.readInt32(2);

    const auto entity = interfaces->entityList->getEntity(entityIndex);
    if (!entity || !entity->isPlayer())
        return;

    const auto isLocal = localPlayer && entity == localPlayer.get();

    const auto voteType = reader.readInt32(3);
    
    std::string message = "";

    message.append(isLocal ? "You" : entity->getPlayerName());
    message.append(" "); 
    message.append(Helpers::getColorByte(ColorByte::White));
    message.append(" Voted [");
    message.append(Helpers::getColorByte(ColorByte::Orange));
    message.append(voteName(voteType));
    message.append(Helpers::getColorByte(ColorByte::White));
    message.append("]");

    Helpers::writeInGameChat(message.c_str(), isLocal ? ColorByte::LightGrey : ColorByte::Purple);
}

void Misc::onVotePass() noexcept
{
    if (miscConfig.revealVotes)
        Helpers::writeInGameChat(std::string{ xorstr_("Vote") }.append(Helpers::getColorByte(ColorByte::LightGreen)).append(xorstr_(" PASSED")).c_str(), ColorByte::Green);
}

void Misc::onVoteFailed() noexcept
{
    if (miscConfig.revealVotes)
        Helpers::writeInGameChat(std::string{ xorstr_("Vote") }.append(Helpers::getColorByte(ColorByte::LightRed)).append(xorstr_(" FAILED")).c_str(), ColorByte::Red);
}

void Misc::drawOffscreenEnemies(ImDrawList* drawList) noexcept
{
    if (!miscConfig.offscreenEnemies.enabled)
        return;

    const auto yaw = Helpers::deg2rad(interfaces->engine->getViewAngles().y);

    GameData::Lock lock;
    for (auto& player : GameData::players()) {
        if ((player.dormant && player.fadingAlpha() == 0.0f) || !player.alive || !player.enemy || player.inViewFrustum)
            continue;

        const auto positionDiff = GameData::local().origin - player.origin;

        auto x = std::cos(yaw) * positionDiff.y - std::sin(yaw) * positionDiff.x;
        auto y = std::cos(yaw) * positionDiff.x + std::sin(yaw) * positionDiff.y;
        if (const auto len = std::sqrt(x * x + y * y); len != 0.0f) {
            x /= len;
            y /= len;
        }

        constexpr auto avatarRadius = 13.0f;
        constexpr auto triangleSize = 10.0f;

        const auto pos = ImGui::GetIO().DisplaySize / 2 + ImVec2{ x, y } * 200;
        const auto trianglePos = pos + ImVec2{ x, y } * (avatarRadius + (miscConfig.offscreenEnemies.healthBar.enabled ? 5 : 3));

        Helpers::setAlphaFactor(player.fadingAlpha());
        const auto white = Helpers::calculateColor(255, 255, 255, 255);
        const auto background = Helpers::calculateColor(0, 0, 0, 80);
        const auto color = Helpers::calculateColor(miscConfig.offscreenEnemies.asColor4());
        const auto healthBarColor = miscConfig.offscreenEnemies.healthBar.type == HealthBar::HealthBased ? Helpers::healthColor(std::clamp(player.health / 100.0f, 0.0f, 1.0f)) : Helpers::calculateColor(miscConfig.offscreenEnemies.healthBar.asColor4());
        Helpers::setAlphaFactor(1.0f);

        const ImVec2 trianglePoints[]{
            trianglePos + ImVec2{  0.4f * y, -0.4f * x } * triangleSize,
            trianglePos + ImVec2{  1.0f * x,  1.0f * y } * triangleSize,
            trianglePos + ImVec2{ -0.4f * y,  0.4f * x } * triangleSize
        };

        drawList->AddConvexPolyFilled(trianglePoints, 3, color);
        drawList->AddCircleFilled(pos, avatarRadius + 1, white & IM_COL32_A_MASK, 40);

        const auto texture = player.getAvatarTexture();

        const bool pushTextureId = drawList->_TextureIdStack.empty() || texture != drawList->_TextureIdStack.back();
        if (pushTextureId)
            drawList->PushTextureID(texture);

        const int vertStartIdx = drawList->VtxBuffer.Size;
        drawList->AddCircleFilled(pos, avatarRadius, white, 40);
        const int vertEndIdx = drawList->VtxBuffer.Size;
        ImGui::ShadeVertsLinearUV(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ avatarRadius, avatarRadius }, pos + ImVec2{ avatarRadius, avatarRadius }, { 0, 0 }, { 1, 1 }, true);

        if (pushTextureId)
            drawList->PopTextureID();

        if (miscConfig.offscreenEnemies.healthBar.enabled) {
            const auto radius = avatarRadius + 2;
            const auto healthFraction = std::clamp(player.health / 100.0f, 0.0f, 1.0f);

            drawList->AddCircle(pos, radius, background, 40, 3.0f);

            const int vertStartIdx = drawList->VtxBuffer.Size;
            if (healthFraction == 1.0f) { // sometimes PathArcTo is missing one top pixel when drawing a full circle, so draw it with AddCircle
                drawList->AddCircle(pos, radius, healthBarColor, 40, 2.0f);
            } else {
                constexpr float pi = std::numbers::pi_v<float>;
                drawList->PathArcTo(pos, radius - 0.5f, pi / 2 - pi * healthFraction, pi / 2 + pi * healthFraction, 40);
                drawList->PathStroke(healthBarColor, false, 2.0f);
            }
            const int vertEndIdx = drawList->VtxBuffer.Size;

            if (miscConfig.offscreenEnemies.healthBar.type == HealthBar::Gradient)
                Helpers::shadeVertsHSVColorGradientKeepAlpha(drawList, vertStartIdx, vertEndIdx, pos - ImVec2{ 0.0f, radius }, pos + ImVec2{ 0.0f, radius }, IM_COL32(0, 255, 0, 255), IM_COL32(255, 0, 0, 255));
        }
    }
}

void Misc::autoAccept(const char* soundEntry) noexcept
{
    if (!miscConfig.autoAccept)
        return;

    if (std::strcmp(soundEntry, "UIPanorama.popup_accept_match_beep"))
        return;

    if (const auto idx = memory->registeredPanoramaEvents->find(memory->makePanoramaSymbol("MatchAssistedAccept")); idx != -1) {
        if (const auto eventPtr = memory->registeredPanoramaEvents->memory[idx].value.makeEvent(nullptr))
            interfaces->panoramaUIEngine->accessUIEngine()->dispatchEvent(eventPtr);
    }

    auto window = FindWindowW(L"Valve001", NULL);
    FLASHWINFO flash{ sizeof(FLASHWINFO), window, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
    FlashWindowEx(&flash);
    ShowWindow(window, SW_RESTORE);
}

void Misc::fakePrime() noexcept
{
    static bool lastState = false;

    if (miscConfig.fakePrime != lastState)
    {
        lastState = miscConfig.fakePrime;

        if (DWORD oldProtect; VirtualProtect(memory->fakePrime, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            constexpr uint8_t patch[]{ 0x31, 0xC0, 0x40, 0xC3 };
            std::memcpy(memory->fakePrime, patch, 4);
            VirtualProtect(memory->fakePrime, 4, oldProtect, nullptr);
        }
    }
}

void Misc::forceRelayCluster() noexcept
{
    std::string dataCentersList[] = { "", "syd", "vie", "gru", "scl", "dxb", "par", "fra", "hkg",
    "maa", "bom", "tyo", "lux", "ams", "limc", "man", "waw", "sgp", "jnb",
    "mad", "sto", "lhr", "atl", "eat", "ord", "lax", "mwh", "okc", "sea", "iad" };

    *memory->relayCluster = dataCentersList[miscConfig.forceRelayCluster];
}

bool Misc::enableSvPureBypass() noexcept
{
    return miscConfig.bypassSvPure;
}

void Misc::updateEventListeners(bool forceRemove) noexcept
{
    class PurchaseEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override { purchaseList(event); }
    };

    static PurchaseEventListener listener;
    static bool listenerRegistered = false;

    if (miscConfig.purchaseList.enabled && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "item_purchase");
        listenerRegistered = true;
    } else if ((!miscConfig.purchaseList.enabled || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
}

void Misc::updateInput() noexcept
{

}


/////////////////////////////////////////////////////////////////
// GUI Function
/////////////////////////////////////////////////////////////////

void Misc::drawGUI() noexcept
{
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 230.0f);
    ImGui::Checkbox("Anti AFK kick", &miscConfig.antiAfkKick);
    ImGui::Checkbox("Auto Disconnect", &miscConfig.autoDisconnect);
    ImGui::Checkbox("Auto pistol", &miscConfig.autoPistol);
    //ImGui::Checkbox("Auto reload", &miscConfig.autoReload);
    ImGui::Checkbox("Auto accept", &miscConfig.autoAccept);
    ImGui::Checkbox("Nade Prediction", &miscConfig.nadePredict);//TODO: move to visuals
    ImGui::Checkbox("Radar hack", &miscConfig.radarHack);
    ImGui::Checkbox("Reveal ranks", &miscConfig.revealRanks);
    ImGui::Checkbox("Reveal money", &miscConfig.revealMoney);
    ImGui::Checkbox("Reveal suspect", &miscConfig.revealSuspect);
    ImGui::Checkbox("Reveal votes", &miscConfig.revealVotes);

    //ImGui::Checkbox("Fix animation LOD", &miscConfig.fixAnimationLOD); //TODO: move to visuals
    //ImGui::Checkbox("Disable model occlusion", &miscConfig.disableModelOcclusion);//TODO: move to visuals
    //ImGui::Checkbox("Opposite Hand Knife", &miscConfig.oppositeHandKnife);//TODO: move to visuals
    //ImGui::Checkbox("Disable HUD blur", &miscConfig.disablePanoramablur);//TODO: move to visuals
    ImGui::SetNextItemWidth(140);
    ImGui::Combo(xorstr_("Region"), &miscConfig.forceRelayCluster, "Default\0Australia\0Austria\0Brazil\0Chile\0Dubai\0France\0Germany\0Hong Kong\0India (Chennai)\0India (Mumbai)\0Japan\0Luxembourg\0Netherlands\0Peru\0Philipines\0Poland\0Singapore\0South Africa\0Spain\0Sweden\0UK\0USA (Atlanta)\0USA (Seattle)\0USA (Chicago)\0USA (Los Angeles)\0USA (Moses Lake)\0USA (Oklahoma)\0USA (Seattle)\0USA (Washington DC)\0");
    ImGui::NextColumn();

    ImGuiCustom::colorPicker("Bomb timer", miscConfig.bombTimer);//TODO: move to visuals
    ImGuiCustom::colorPicker("Offscreen Enemies", miscConfig.offscreenEnemies.asColor4(), &miscConfig.offscreenEnemies.enabled);//TODO: move to visuals
    if (miscConfig.offscreenEnemies.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Offscreen Enemies");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::Checkbox("Health Bar", &miscConfig.offscreenEnemies.healthBar.enabled);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("Type", &miscConfig.offscreenEnemies.healthBar.type, "Gradient\0Solid\0Health-based\0");
            if (miscConfig.offscreenEnemies.healthBar.type == HealthBar::Solid) {
                ImGui::SameLine();
                ImGuiCustom::colorPicker("", miscConfig.offscreenEnemies.healthBar.asColor4());
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGuiCustom::colorPicker("Noscope crosshair", miscConfig.noscopeCrosshair);//TODO: move to visuals
    //ImGuiCustom::colorPicker("Recoil crosshair", miscConfig.recoilCrosshair);//TODO: move to visuals
    //ImGui::Checkbox("Bypass SvPure", &miscConfig.bypassSvPure);
    //ImGui::Checkbox("Fix tablet signal", &miscConfig.fixTabletSignal);
    //ImGui::Checkbox("Fake Prime", &miscConfig.fakePrime);
    ImGui::Checkbox("Quick reload", &miscConfig.quickReload);
    ImGui::Checkbox("Name stealer", &miscConfig.nameStealer);
    
    //ImGui::Checkbox("Animated clan tag", &miscConfig.animatedClanTag);
    //ImGui::Checkbox("Clock tag", &miscConfig.clocktag);
    ImGui::Checkbox("Custom clantag", &miscConfig.customClanTag);
    if (miscConfig.customClanTag) {
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::PushID("Custom clantag text");
        if (ImGui::InputText("", miscConfig.clanTag, sizeof(miscConfig.clanTag)))
            Misc::updateClanTag(true);
        ImGui::PopID();
    }

    ImGui::Checkbox("Kill message", &miscConfig.killMessage);
    if (miscConfig.killMessage) {
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::PushID("Custom KillMessage Text");
        ImGui::InputText("", &miscConfig.killMessageString);
        ImGui::PopID();
    }
    
    /*
    ImGui::Checkbox("Fake Message", &miscConfig.fakeMsgToggled);
    if (miscConfig.fakeMsgToggled) {
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::PushID("Fake Message Color");
        ImGui::Combo("", &miscConfig.fakeMsgColor, "White\0Red\0Purple\0Green\0Light green\0Turquoise\0Light red\0Gray\0Yellow\0Gray 2\0Light blue\0Gray/Purple\0Blue\0Pink\0Dark orange\0Orange\0");
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID("Fake Message Text");
        ImGui::InputText("", &miscConfig.fakeMsgText);
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::Button("Update"))
            Misc::fakeMessage(true);
    }

    ImGui::Checkbox("Quick healthshot", &miscConfig.quickHealthshot);
    if (miscConfig.quickHealthshot) {
        ImGui::SameLine();
        ImGui::PushID("Prepare revolver Key");
        ImGui::hotkey("", miscConfig.quickHealthshotKey);
    }*/

    ImGui::Checkbox("Prepare revolver", &miscConfig.prepareRevolver);
    if (miscConfig.prepareRevolver) {
        ImGui::SameLine();
        ImGui::PushID("Prepare revolver Key");
        ImGui::hotkey("", miscConfig.prepareRevolverKey);
        ImGui::PopID();
    }

    ImGui::Checkbox("Preserve Killfeed", &miscConfig.preserveKillfeed.enabled);//TODO: move to visuals
    if (miscConfig.preserveKillfeed.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Preserve Killfeed");
        if (ImGui::Button("...")) ImGui::OpenPopup("");
        if (ImGui::BeginPopup("")) {
            ImGui::Checkbox("Only Headshots", &miscConfig.preserveKillfeed.onlyHeadshots);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    /*
    ImGui::Checkbox("Purchase List", &miscConfig.purchaseList.enabled);//TODO: move to visuals
    if (miscConfig.purchaseList.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Purchase List");
        if (ImGui::Button("...")) ImGui::OpenPopup("");
        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(75.0f);
            ImGui::Combo("Mode", &miscConfig.purchaseList.mode, "Details\0Summary\0");
            ImGui::Checkbox("Only During Freeze Time", &miscConfig.purchaseList.onlyDuringFreezeTime);
            ImGui::Checkbox("Show Prices", &miscConfig.purchaseList.showPrices);
            ImGui::Checkbox("No Title Bar", &miscConfig.purchaseList.noTitleBar);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    
    ImGui::Checkbox("Spectator list", &miscConfig.spectatorList.enabled);//TODO: move to visuals
    if (miscConfig.spectatorList.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Spectator list");
        if (ImGui::Button("...")) ImGui::OpenPopup("");
        if (ImGui::BeginPopup("")) {
            ImGui::Checkbox("No Title Bar", &miscConfig.spectatorList.noTitleBar);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    
    ImGui::Checkbox("Reportbot", &miscConfig.reportbot.enabled);
    if (miscConfig.reportbot.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Reportbot");
        if (ImGui::Button("...")) ImGui::OpenPopup("");
        if (ImGui::BeginPopup("")) {
            ImGui::PushItemWidth(80.0f);
            ImGui::Combo("Target", &miscConfig.reportbot.target, "Enemies\0Allies\0All\0");
            ImGui::InputInt("Delay (s)", &miscConfig.reportbot.delay);
            miscConfig.reportbot.delay = (std::max)(miscConfig.reportbot.delay, 1);
            ImGui::InputInt("Rounds", &miscConfig.reportbot.rounds);
            miscConfig.reportbot.rounds = (std::max)(miscConfig.reportbot.rounds, 1);
            ImGui::PopItemWidth();
            ImGui::Checkbox("Abusive Communications", &miscConfig.reportbot.textAbuse);
            ImGui::Checkbox("Griefing", &miscConfig.reportbot.griefing);
            ImGui::Checkbox("Wall Hacking", &miscConfig.reportbot.wallhack);
            ImGui::Checkbox("Aim Hacking", &miscConfig.reportbot.aimbot);
            ImGui::Checkbox("Other Hacking", &miscConfig.reportbot.other);
            if (ImGui::Button("Reset"))
                Misc::resetReportbot();
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }*/
     
    if (ImGui::Button(xorstr_("Unload DLL"))) hooks->uninstall();
    
    ImGui::Columns(1);

    {
        GameData::Lock lock;
             
        const auto pr = *memory->playerResource;

        bool dangerzone = false;//temp
       
        if (localPlayer && pr)
        {
            ImGui::Separator();

            if (ImGui::BeginTable("playerinfo", 8))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("SteamID");
                ImGui::TableSetupColumn("Wins");
                ImGui::TableSetupColumn("Level");
                ImGui::TableSetupColumn("Rank");

                //ImGui::TableSetupColumn("Commends Friendly");
                //ImGui::TableSetupColumn("Commends Teacher");
                //ImGui::TableSetupColumn("Commends Leader");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::PushID(ImGui::TableGetRowIndex());
                
                auto localPlayerData = GameData::local();
                
                if (ImGui::TableNextColumn())
                    ImGui::TextUnformatted((std::string{ "Localplayer ("}.append(localPlayerData.name).append(")")).c_str());
                
                if (ImGui::TableNextColumn()) {
                    ImGui::TextUnformatted(localPlayerData.steamID.c_str());
                    ImGui::SameLine();
                    if (ImGui::Button("Copy")) {
                        ImGui::SetClipboardText(localPlayerData.steamID.c_str());
                    }
                }
                ImGuiCustom::HelpMarker("Copy your steamID.");
                 
                if (ImGui::TableNextColumn()) {
                    ImGui::InputInt("Wins", &pr->wins()[localPlayer->index()]);
                    ImGuiCustom::HelpMarker("Edit your comp wins.");
                } 

                if (ImGui::TableNextColumn()) { 
                    ImGui::InputInt("Level", &pr->level()[localPlayer->index()]);
                    ImGuiCustom::HelpMarker("Edit your level.");
                }

                if (ImGui::TableNextColumn()) {
                    if (pr->rank()[localPlayer->index()] > -1 && pr->rank()[localPlayer->index()] < 20) {
                        ImGui::Combo(
                            xorstr_("Rank"),
                            &pr->rank()[localPlayer->index()],
                            dangerzone ?
                            "Unranked\0Lab Rat I\0Lab Rat II\0Sprinting Hare I\0Sprinting Hare II\0Wild Scout I\0Wild Scout II\0Wild Scout Elite\0Hunter Fox I\0Hunter Fox II\0Hunter Fox III\0Hunter Fox Elite\0Timber Wolf\0Ember Wolf\0Wildfire Wolf\0The Howling Alpha\0"
                            :
                            "Unranked\0Silver I\0Silver II\0Silver III\0Silver IV\0Silver Elite\0Silver Elite Master\0Gold Nova I\0Gold Nova II\0Gold Nova III\0Gold Nova Master\0Master Guardian I\0Master Guardian II\0Master Guardian Elite\0Distinguished Master Guardian\0Legendary Eagle\0Legendary Eagle Master\0Supreme Master First Class\0The Global Elite\0"
                        );
                    } 
                    ImGuiCustom::HelpMarker("Edit your comp rank.");
                }
                    
                /*if (ImGui::TableNextColumn()) {
                    ImGui::InputInt("Friendly", &pr->commendsFriendly()[localPlayer->index()]);
                    ImGuiCustom::HelpMarker("Edit your friendly commends.");
                }

                if (ImGui::TableNextColumn()) {
                    ImGui::InputInt("Teacher", &pr->commendsTeacher()[localPlayer->index()]);
                    ImGuiCustom::HelpMarker("Edit your teacher commends.");
                }

                if (ImGui::TableNextColumn()) {
                    ImGui::InputInt("Leader", &pr->commendsLeader()[localPlayer->index()]);
                    ImGuiCustom::HelpMarker("Edit your leader commends.");
                }*/
                 
                for (auto& playersData : GameData::players())
                {
                    auto* entity = interfaces->entityList->getEntityFromHandle(playersData.handle);
                    if (!entity || !entity->isPlayer() || playersData.steamID.empty() || playersData.steamID.compare("0") == 0) continue;

                    ImGui::TableNextRow();
                    ImGui::PushID(ImGui::TableGetRowIndex());
                     
                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(playersData.name.c_str());

                    if (ImGui::TableNextColumn()) {
                        ImGui::TextUnformatted(playersData.steamID.c_str()); 
                        ImGui::SameLine();
                        if (ImGui::Button("Copy")) {
                            ImGui::SetClipboardText(playersData.steamID.c_str());
                        }
                        ImGuiCustom::HelpMarker("Copy users steamID.");
                    }

                    if (ImGui::TableNextColumn())
                        ImGui::Text("%i", playersData.wins);

                    if (ImGui::TableNextColumn())
                        ImGui::Text("%i", playersData.level);

                    if (ImGui::TableNextColumn())
                        ImGui::TextUnformatted(playersData.rank.c_str());

                    /*if (ImGui::TableNextColumn())
                        ImGui::Text("%i", playersData.commends.Friendly);

                    if (ImGui::TableNextColumn())
                        ImGui::Text("%i", playersData.commends.Teacher);

                    if (ImGui::TableNextColumn())
                        ImGui::Text("%i", playersData.commends.Leader);
                    */
                }

                ImGui::EndTable();
                ImGui::Separator();
            }
        }
    }
}


/////////////////////////////////////////////////////////////////
// Config Functions
/////////////////////////////////////////////////////////////////

static void from_json(const json& j, ImVec2& v)
{
    read(j, "X", v.x);
    read(j, "Y", v.y);
}

static void from_json(const json& j, PurchaseList& pl)
{
    read(j, "Enabled", pl.enabled);
    read(j, "Only During Freeze Time", pl.onlyDuringFreezeTime);
    read(j, "Show Prices", pl.showPrices);
    read(j, "No Title Bar", pl.noTitleBar);
    read(j, "Mode", pl.mode);
}

static void from_json(const json& j, OffscreenEnemies& o)
{
    from_json(j, static_cast<ColorToggle&>(o));

    read<value_t::object>(j, "Health Bar", o.healthBar);
}

static void from_json(const json& j, MiscConfig::SpectatorList& sl)
{
    read(j, "Enabled", sl.enabled);
    read(j, "No Title Bar", sl.noTitleBar);
    read<value_t::object>(j, "Pos", sl.pos);
    read<value_t::object>(j, "Size", sl.size);
}
 
static void from_json(const json& j, PreserveKillfeed& o)
{
    read(j, "Enabled", o.enabled);
    read(j, "Only Headshots", o.onlyHeadshots);
}

static void from_json(const json& j, MiscConfig& m)
{
    read(j, "Anti AFK kick", m.antiAfkKick);
    read(j, xorstr_("Auto disconnect"), m.autoDisconnect);
    read(j, "Custom clan tag", m.customClanTag);
    read(j, "Clock tag", m.clocktag);
    read(j, "Clan tag", m.clanTag, sizeof(m.clanTag));
    read(j, "Animated clan tag", m.animatedClanTag);
    read<value_t::object>(j, "Noscope crosshair", m.noscopeCrosshair);
    read<value_t::object>(j, "Recoil crosshair", m.recoilCrosshair);
    read(j, "Auto pistol", m.autoPistol);
    read(j, "Auto reload", m.autoReload);
    read(j, "Auto accept", m.autoAccept);
    read(j, "Radar hack", m.radarHack);
    read(j, "Reveal ranks", m.revealRanks);
    read(j, "Reveal money", m.revealMoney);
    read(j, "Reveal suspect", m.revealSuspect);
    read(j, "Reveal votes", m.revealVotes);
    read<value_t::object>(j, "Spectator list", m.spectatorList);
    read<value_t::object>(j, "Offscreen Enemies", m.offscreenEnemies);
    read(j, "Fix animation LOD", m.fixAnimationLOD);
    read(j, "Disable model occlusion", m.disableModelOcclusion);
    read(j, "Kill message", m.killMessage);
    read<value_t::string>(j, "Kill message string", m.killMessageString);
    read(j, "Name stealer", m.nameStealer);
    read(j, "Fake prime", m.fakePrime);
    read(j, "Disable HUD blur", m.disablePanoramablur);
    read(j, "Fake Message", m.fakeMsgToggled);
    read(j, "Fake Message color", m.fakeMsgColor);
    read<value_t::string>(j, "Fake Message text", m.fakeMsgText);
    read<value_t::object>(j, "Bomb timer", m.bombTimer);
    read(j, "Quick reload", m.quickReload);
    read(j, "Prepare revolver", m.prepareRevolver);
    read(j, "Prepare revolver key", m.prepareRevolverKey);
    read(j, "Quick healthshot", m.quickHealthshot);
    read(j, "Quick healthshot key", m.quickHealthshotKey);
    read(j, "Grenade predict", m.nadePredict);
    read(j, "Fix tablet signal", m.fixTabletSignal);
    read<value_t::object>(j, "Purchase List", m.purchaseList);
    read<value_t::object>(j, "Reportbot", m.reportbot);
    read(j, "Opposite Hand Knife", m.oppositeHandKnife);
    read<value_t::object>(j, "Preserve Killfeed", m.preserveKillfeed);
    read(j, "Relay cluster", m.forceRelayCluster);
    read(j, "Bypass SvPure", m.bypassSvPure);
}

static void from_json(const json& j, MiscConfig::Reportbot& r)
{
    read(j, "Enabled", r.enabled);
    read(j, "Target", r.target);
    read(j, "Delay", r.delay);
    read(j, "Rounds", r.rounds);
    read(j, "Abusive Communications", r.textAbuse);
    read(j, "Griefing", r.griefing);
    read(j, "Wall Hacking", r.wallhack);
    read(j, "Aim Hacking", r.aimbot);
    read(j, "Other Hacking", r.other);
}

static void to_json(json& j, const MiscConfig::Reportbot& o, const MiscConfig::Reportbot& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Target", target);
    WRITE("Delay", delay);
    WRITE("Rounds", rounds);
    WRITE("Abusive Communications", textAbuse);
    WRITE("Griefing", griefing);
    WRITE("Wall Hacking", wallhack);
    WRITE("Aim Hacking", aimbot);
    WRITE("Other Hacking", other);
}

static void to_json(json& j, const PurchaseList& o, const PurchaseList& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Only During Freeze Time", onlyDuringFreezeTime);
    WRITE("Show Prices", showPrices);
    WRITE("No Title Bar", noTitleBar);
    WRITE("Mode", mode);
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const OffscreenEnemies& o, const OffscreenEnemies& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);

    WRITE("Health Bar", healthBar);
}

static void to_json(json& j, const MiscConfig::SpectatorList& o, const MiscConfig::SpectatorList& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("No Title Bar", noTitleBar);

    if (const auto window = ImGui::FindWindowByName("Spectator list")) {
        j["Pos"] = window->Pos;
        j["Size"] = window->SizeFull;
    }
}

static void to_json(json& j, const PreserveKillfeed& o, const PreserveKillfeed& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Only Headshots", onlyHeadshots);
}
 
static void to_json(json& j, const MiscConfig& o)
{
    const MiscConfig dummy;
    
    WRITE("Anti AFK kick", antiAfkKick);
    WRITE(xorstr_("Auto disconnect"), autoDisconnect);
    WRITE("Custom clan tag", customClanTag);
    WRITE("Clock tag", clocktag);

    if (o.clanTag[0])
        j["Clan tag"] = o.clanTag;

    WRITE("Animated clan tag", animatedClanTag);
    WRITE("Noscope crosshair", noscopeCrosshair);
    WRITE("Recoil crosshair", recoilCrosshair);
    WRITE("Auto pistol", autoPistol);
    WRITE("Auto reload", autoReload);
    WRITE("Auto accept", autoAccept);
    WRITE("Radar hack", radarHack);
    WRITE("Reveal ranks", revealRanks);
    WRITE("Reveal money", revealMoney);
    WRITE("Reveal suspect", revealSuspect);
    WRITE("Reveal votes", revealVotes);
    WRITE("Spectator list", spectatorList);
    WRITE("Offscreen Enemies", offscreenEnemies);
    WRITE("Fix animation LOD", fixAnimationLOD);
    WRITE("Disable model occlusion", disableModelOcclusion);
    WRITE("Kill message", killMessage);
    WRITE("Kill message string", killMessageString);
    WRITE("Name stealer", nameStealer);
    WRITE("Fake prime", fakePrime);
    WRITE("Disable HUD blur", disablePanoramablur);
    WRITE("Fake Message", fakeMsgToggled);
    WRITE("Fake Message color", fakeMsgColor);
    WRITE("Fake Message text", fakeMsgText);
    WRITE("Bomb timer", bombTimer);
    WRITE("Quick reload", quickReload);
    WRITE("Prepare revolver", prepareRevolver);
    WRITE("Prepare revolver key", prepareRevolverKey);
    WRITE("Quick healthshot", quickHealthshot);
    WRITE("Quick healthshot key", quickHealthshotKey);
    WRITE("Grenade predict", nadePredict);
    WRITE("Fix tablet signal", fixTabletSignal);
    WRITE("Purchase List", purchaseList);
    WRITE("Reportbot", reportbot);
    WRITE("Opposite Hand Knife", oppositeHandKnife);
    WRITE("Preserve Killfeed", forceRelayCluster); 
    WRITE("Relay cluster", forceRelayCluster);
    WRITE("Bypass SvPure", bypassSvPure);
}

json Misc::toJson() noexcept
{
    json j;
    to_json(j, miscConfig);
    return j;
}

void Misc::fromJson(const json& j) noexcept
{
    from_json(j, miscConfig);
}

void Misc::resetConfig() noexcept
{
    miscConfig = {};
}