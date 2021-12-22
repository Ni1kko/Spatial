#include <algorithm>
#include <array>
#include <iomanip>
#include <mutex>
#include <numbers>
#include <numeric>
#include <sstream>
#include <vector>
#include <string>

#include <Encryption/xorstr.hpp>

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
#include "Troll.h"

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
#include "../SDK/GameEvent.h"

#include "../Menu/Menu.h"
#include "../Helpers.h"
#include "../Hooks.h"
#include "../GameData.h"

#include "../Menu/imguiCustom.h"


/////////////////////////////////////////////////////////////////
// Vars
/////////////////////////////////////////////////////////////////

static bool windowOpen = false;

std::vector<std::string> chatSpamList =
{
    xorstr_("You Got Rekt'd by Affinity!"),
    xorstr_("Bro u so very bad, refund your cheat"),
    xorstr_("Are you sure with that your c+p?"),
    xorstr_("It's a ez game for me"),
    xorstr_("Can u start play dog?"),
    xorstr_("I love if somebody is mad"),
    xorstr_("Affinity Just You Look Like A Bitch"),
    xorstr_("U can't be better then me, just surrender hhh"),
    xorstr_("Just come i one tap u"),
    xorstr_("Who using aimware in 2021? Maybe u hurensohn"),
    xorstr_("I don't like dog's, but i like cat on a plate"),
    xorstr_("You only killed me because I ran out of health"),
    xorstr_("WOW! imagine if your parents weren't siblings"),
    xorstr_("I PLAY WITH A RACING WHEEL"),
    xorstr_("CS:GO is too hard for you m8 maybe consider a game that requires less skill, like idk.... solitaire"),
    xorstr_("Bro you couldn't hit an elephant in the ass with a shotgun with aim like that"),
    xorstr_("You're the human equivalent of a participation award."),
    xorstr_("server cvar 'sv_rekt' changed to 1"),
    xorstr_("Was that your spray on the wall or are you just happy to see me?"),
    xorstr_("Oops, I must have chosen easy bots by accident"),
    xorstr_("Is this casual?? I have 16k"),
    xorstr_("I thought I already finished chemistry.. So much NaCl around here.."),
    xorstr_("Did you know that csgo isn't just Free to play Its also Free to uninstall?"),
    xorstr_("Even Noah can't carry these animals"),
    xorstr_("Options -> How To Play"),
    xorstr_("deutschland deutschland ohne alles"),
    xorstr_("How much did you tag that wall for??"),
    xorstr_("I thought I put bots on hard, why are they on easy?"),
    xorstr_("Who are you sponsored by? Parkinson's?"),
    xorstr_("How did you change your difficulty settings? My CS:GO is stuck on easy :P"),
    xorstr_("Nice $4750 decoy!!!"),
    xorstr_("If I were to commit suicide, I would jump from your ego to your elo!"),
    xorstr_("Dude Is your monitor even on?"),
    xorstr_("idk if u know but it's mouse1 to shoot!"),
    xorstr_("you guys buy accounts on ebay??"),
    xorstr_("You have the reaction time of a dead puppy!"),
    xorstr_("You always over do it, just the same as the germans did on the french hoilday 1939 :)"),
    xorstr_("The only thing you carry is an extra chromosome"),
    xorstr_("Safest place for us to stand is in front of your gun"),
    xorstr_("is that a decoy, or are you trying to shoot somebody?"),
    xorstr_("I could swallow bullets and shit out a better spray than that"),
    xorstr_("deranking?"),
    xorstr_("a bad awper = $5k decoy"),
    xorstr_("Shut up, I fucked your dad."),
    xorstr_("You REALLY gotta win those"),
    xorstr_("Buy sound next time"),
    xorstr_("mad cuz bad"),
    xorstr_("You couldn't even carry groceries in from the car :p"),
    xorstr_("I kissed your mom last night. Her breath was globally offensive :D"),
    xorstr_("Dude you're so fat you run out of breath rushing B"),
    xorstr_("Did you learn your spray downs in a bukkake video?"),
    xorstr_("You're almost as salty as the semen dripping from your mum's mouth :D"),
    xorstr_("Rest in spaghetti never forgetti"),
    xorstr_("Don't be upsetti, have some spaghetti"),
    xorstr_("Stop buying an awp you $4750 Decoy"),
    xorstr_("This guy is more toxic than the beaches at Fukushima"),
    xorstr_("I'm jealous of people that don't know you."),
    xorstr_("If only your mother beat alcoholism instead of you, you wouldn't act the way you do. "),
    xorstr_("You Got Rosated Bitch"),
    xorstr_("I'd tell you to go outside, but you'd just ruin that for everyone else too"),
    xorstr_("If autism speaks, you're fucking shouting."),
    xorstr_("i don't have the time nor the crayons to explain it to you"),
    xorstr_("You are the reason the gene pool needs a lifeguard"),
    xorstr_("When you go to a family reunion, who brings the condoms?"),
    xorstr_("You are the reason I have to doubt evolution and natural selection"),
    xorstr_("Believe in yourself, because the rest of us think you're an idiot."),
    xorstr_("I'm surprised that you were able hit the 'Install' button"),
    xorstr_("I'm not trash talking, I'm talking to trash"),
    xorstr_("Don't worry guys, I'm a garbage collector. I'm used to carrying trash."),
    xorstr_("I'd love to see things from your perspective, but I don't think I could shove my head that far up my ass."),
    xorstr_("To which foundation do I need to donate to help you?"),
    xorstr_("Does your ass get jealous of all the shit that comes out of your mouth?"),
    xorstr_("A million years of evolution and we get you."),
    xorstr_("You're the reason the gene pool needs a lifeguard."),
    xorstr_("Two wrongs don't make a right, take your parents as an example."),
    xorstr_("I would insult you but nature did a better job."),
    xorstr_("With aim like that, I pity whoever has to clean the floor around your toilet"),
    xorstr_("I would call you cancer, but cancer actually kills people."),
    xorstr_("Stephen Hawking did great with his disability. Why can't you?"),
    xorstr_("I'd tell you to go outside, but you'd just ruin that for everyone else too"),
    xorstr_("Go home and take a look into your mirror. See that? It's called a failure at life.")
};

/////////////////////////////////////////////////////////////////
// Structs
/////////////////////////////////////////////////////////////////

struct TrollConfig {

    bool blockbot { false };
    KeyBind blockbotKey{ KeyBind::V };
    bool doorSpam { false }; 
    float doorSpamRange { 0.f }; 
    int chatSpamMode { 0 };
    int chatSpamType{ 0 };
    float chatSpamDelay{ 3 };
    KeyBind chatSpamKey;
    std::string chatSpamCustom{ "" };
} trollConfig;


/////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////

void Troll::blockbot(UserCmd* cmd) noexcept
{
    if (!trollConfig.blockbot || !trollConfig.blockbotKey.isDown())
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    float bestDistance = 200.0f;
    int plyIndex = -1;

    for (int i = 1; i < interfaces->engine->getMaxClients(); i++)
    {
        Entity* ply = interfaces->entityList->getEntity(i);

        if (!ply)
            continue;

        if (!ply->isAlive() || ply->isDormant() || ply == localPlayer.get())
            continue;

        float distance = localPlayer->origin().distTo(ply->origin());

        if (distance < bestDistance)
        {
            bestDistance = distance;
            plyIndex = i;
        }
    }

    if (plyIndex == -1)
        return;

    Entity* target = interfaces->entityList->getEntity(plyIndex);

    if (!target)
        return;

    if (localPlayer->origin().z - target->origin().z > 20)
    {
        Vector vecForward = target->origin() - localPlayer->origin();

        cmd->forwardmove = ((sin(Helpers::deg2rad(cmd->viewangles.y)) * vecForward.y) + (cos(Helpers::deg2rad(cmd->viewangles.y)) * vecForward.x)) * 450.0f;
        cmd->sidemove = ((cos(Helpers::deg2rad(cmd->viewangles.y)) * -vecForward.y) + (sin(Helpers::deg2rad(cmd->viewangles.y)) * vecForward.x)) * 450.0f;
    }
    else {
        Vector angles = Helpers::calculateRelativeAngle(localPlayer->origin(), target->origin());

        angles.y = angles.y - localPlayer->eyeAngles().y;
        angles.normalize();

        if (angles.y < 0.0f)
            cmd->sidemove = 450.0f;
        else if (angles.y > 0.0f)
            cmd->sidemove = -450.0f;
    }
}

void Troll::doorSpam(UserCmd* cmd) noexcept
{
    if (!trollConfig.doorSpam || !localPlayer || localPlayer->isDefusing()) {
        return;
    }

    Trace trace;
    TraceFilter traceFilter{ localPlayer.get() };
    traceFilter.skip = localPlayer.get();

    const auto startPos = localPlayer->getEyePosition();
    const auto endPos = startPos + Vector::fromAngle(cmd->viewangles) * trollConfig.doorSpamRange;
    interfaces->engineTrace->traceRay({ startPos, endPos }, 0x46004009, traceFilter, trace);

    if (trace.entity && trace.entity->getClientClass()->classId == ClassId::CPropDoorRotating) {
        if (cmd->buttons & UserCmd::IN_USE && cmd->tickCount & 1) {
            cmd->buttons &= ~UserCmd::IN_USE;
        }
    }
}

void Troll::chatSpam(ChatSpamEvents spamEvent) noexcept
{
    const auto now = memory->globalVars->realtime;
    static auto lastTime = 0.0f;

    //off
    if (trollConfig.chatSpamMode == 0 || spamEvent == ChatSpamEvents::Off || !localPlayer || !interfaces->engine->isConnected() || !interfaces->engine->isInGame()) return;
 
    //timed
    if (trollConfig.chatSpamType == 0 && (spamEvent != ChatSpamEvents::Timed || (now - lastTime) < trollConfig.chatSpamDelay * 1000.0f)) return;
     
    //onKill
    if (trollConfig.chatSpamType == 1 && (spamEvent != ChatSpamEvents::OnKill || !localPlayer->isAlive())) return;
    
    //onDeath
    if (trollConfig.chatSpamType == 2 && (spamEvent != ChatSpamEvents::OnDeath || localPlayer->isAlive())) return;
    
    //onKey
    if (trollConfig.chatSpamType == 3 && (spamEvent != ChatSpamEvents::OnKey || !trollConfig.chatSpamKey.isPressed() || (now - lastTime) < trollConfig.chatSpamDelay * 1000.0f)) return;
    
    //OnMVP
    if (trollConfig.chatSpamType == 4 && spamEvent != ChatSpamEvents::OnMVP) return;

    //OnDMG
    if (trollConfig.chatSpamType == 5 && spamEvent != ChatSpamEvents::OnDMG) return;
      
    //mode message
    std::srand(time(NULL));
    std::string message;
    switch (trollConfig.chatSpamMode)
    {
        case 1:  message = chatSpamList[rand() % chatSpamList.size()]; break; //Random
        case 2:  message = trollConfig.chatSpamCustom;                 break; //Custom
        case 3:  message = xorstr_("\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9\xE2\x80\xA9"); break;
        case 4:  message = xorstr_("\uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD \uFDFD"); break;
    }
    
    //no message
    if (message.empty()) return;
    
    //excute command
    Helpers::excuteSayCommand(message.c_str());
}

/////////////////////////////////////////////////////////////////
// GUI Functions
/////////////////////////////////////////////////////////////////

void Troll::menuBarItem() noexcept
{
    if (ImGui::MenuItem(xorstr_("Troll"))) {
        windowOpen = true;
        ImGui::SetWindowFocus(xorstr_("Troll"));
        ImGui::SetWindowPos(xorstr_("Troll"), { 100.0f, 100.0f });
    }
}

void Troll::tabItem() noexcept
{
    if (ImGui::BeginTabItem(xorstr_("Troll"))) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Troll::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen) return;
        ImGui::SetNextWindowSize({ 580.0f, 0.0f });
        ImGui::Begin(xorstr_("Troll"), &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    //col 1
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 325.0f); 
    ImGui::Checkbox("Block Bot", &trollConfig.blockbot);
    ImGui::SameLine();
    ImGui::PushID("Block Bot Key");
    ImGui::hotkey("", trollConfig.blockbotKey);
    ImGui::PopID();
    ImGui::Spacing();

    //col 2
    ImGui::NextColumn();
    ImGui::Checkbox("Door spam", &trollConfig.doorSpam);
    ImGui::SameLine();
    ImGui::PushItemWidth(220.0f);
    ImGui::PushID("Door spam range");
    ImGui::SliderFloat("", &trollConfig.doorSpamRange, 0, 500, "Range (%.0f) meters");
    ImGui::PopID();
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(80.0f); 
    ImGui::PushID("Spam Mode");
    ImGui::Combo("", &trollConfig.chatSpamMode, "Off\0Random\0Custom\0Nuke\0Basmala\0");
    ImGui::PopID();
    ImGui::PopItemWidth();
    if (trollConfig.chatSpamMode != 0) {
        if (trollConfig.chatSpamMode == 2) {
            ImGui::SameLine();
            ImGui::PushItemWidth(120.0f);
            ImGui::PushID("Spam Custom");
            ImGui::InputText("", &trollConfig.chatSpamCustom);
            ImGui::PopID();
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(80.0f);
        ImGui::PushID("Spam Type");
        ImGui::Combo("", &trollConfig.chatSpamType, "Timed\0OnKill\0OnDeath\0OnKey\0OnMVP\0OnDMG\0");
        ImGui::PopID();
        ImGui::PopItemWidth();
        if(trollConfig.chatSpamType == 0) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(140.0f);
            ImGui::PushID("Spam Delay");
            ImGui::SliderFloat("", &trollConfig.chatSpamDelay, 1.450f, 60.0f, "Delay (%.3f) seconds"); 
            ImGui::PopID();
        } else if (trollConfig.chatSpamType == 3) {
            ImGui::SameLine();
            ImGui::PushID("Spam Key");
            ImGui::hotkey("", trollConfig.chatSpamKey);
            ImGui::PopID();
        }
    }
     
    ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}

/////////////////////////////////////////////////////////////////
// Config Functions
/////////////////////////////////////////////////////////////////

static void from_json(const json& j, TrollConfig& m)
{
    read(j, xorstr_("Block Bot"), m.blockbot);
    read(j, xorstr_("Slowwalk key"), m.blockbotKey);
    read(j, xorstr_("Door spam"), m.doorSpam);
    read(j, xorstr_("Door spam range"), m.doorSpamRange);
    read(j, xorstr_("Chat spam mode"), m.chatSpamMode);
    read(j, xorstr_("Chat spam type"), m.chatSpamType);
    read(j, xorstr_("Chat spam key"), m.chatSpamKey);
    read(j, xorstr_("Chat spam delay"), m.chatSpamDelay);
    read<value_t::string>(j, xorstr_("Chat spam custom"), m.chatSpamCustom);
}

static void to_json(json& j, const TrollConfig& o)
{
    const TrollConfig dummy;
    WRITE(xorstr_("Block Bot"), blockbot);
    WRITE(xorstr_("Block Bot Key"), blockbotKey);
    WRITE(xorstr_("Door spam"), doorSpam);
    WRITE(xorstr_("Door spam range"), doorSpamRange);
    WRITE(xorstr_("Chat spam mode"), chatSpamMode);
    WRITE(xorstr_("Chat spam type"), chatSpamType);
    WRITE(xorstr_("Chat spam key"), chatSpamKey);
    WRITE(xorstr_("Chat spam delay"), chatSpamDelay);
    WRITE(xorstr_("Chat spam custom"), chatSpamCustom);
}

json Troll::toJson() noexcept
{
    json j;
    to_json(j, trollConfig);
    return j;
}

void Troll::fromJson(const json& j) noexcept
{
    from_json(j, trollConfig);
}

void Troll::resetConfig() noexcept
{
    trollConfig = {};
}