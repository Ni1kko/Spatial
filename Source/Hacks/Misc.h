#pragma once

#include "../JsonForward.h"

#include "../InputUtil.h"

enum class FrameStage;
class GameEvent;
struct ImDrawList;
struct UserCmd;

namespace Misc
{
    bool shouldRevealMoney() noexcept;
    bool shouldRevealSuspect() noexcept;
    bool shouldDisableModelOcclusion() noexcept;
    bool isRadarHackOn() noexcept;
    bool isMenuKeyPressed() noexcept;
    KeyBind menuKeyBind() noexcept;
    float aspectRatio() noexcept;
    void updateClanTag(bool = false) noexcept;
    void spectatorList() noexcept;
    void noscopeCrosshair(ImDrawList* drawlist) noexcept;
    void recoilCrosshair(ImDrawList* drawList) noexcept;
    void watermark(ImDrawList* drawList) noexcept;
    void prepareRevolver(UserCmd*) noexcept;
    void drawBombTimer() noexcept;
    void stealNames() noexcept;
    void disablePanoramablur() noexcept;
    void quickReload(UserCmd*) noexcept;
    bool changeName(bool, const char*, float) noexcept;
    void fakeBan(bool = false) noexcept;
    void nadePredict() noexcept;
    void fixTabletSignal() noexcept;
    void killMessage(GameEvent& event) noexcept;
    void antiAfkKick(UserCmd* cmd) noexcept;
    void fixAnimationLOD(FrameStage stage) noexcept;
    void autoPistol(UserCmd* cmd) noexcept;
    void autoReload(UserCmd* cmd) noexcept;
    void autoDisconnect() noexcept;
    void revealRanks(UserCmd* cmd) noexcept;
    void playHitSound(GameEvent& event) noexcept;
    void killSound(GameEvent& event) noexcept;
    void purchaseList(GameEvent* event = nullptr) noexcept;
    void oppositeHandKnife(FrameStage stage) noexcept;
    void runReportbot() noexcept;
    void resetReportbot() noexcept;
    void preserveKillfeed(bool roundStart = false) noexcept;
    void voteRevealer(GameEvent& event) noexcept;
    void onVoteStart(const void* data, int size) noexcept;
    void onVotePass() noexcept;
    void onVoteFailed() noexcept;
    void drawOffscreenEnemies(ImDrawList* drawList) noexcept;
    void autoAccept(const char* soundEntry) noexcept; 
    void fakePrime() noexcept;
    void forceRelayCluster() noexcept;
    void updateEventListeners(bool forceRemove = false) noexcept;
    void updateInput() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
