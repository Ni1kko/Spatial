#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>
#include <ShlObj.h>
#include <Windows.h>

#include <Menu/imgui/imgui.h>
#include <Menu/imgui/imgui_stdlib.h>
#include <Menu/imguiCustom.h>
#include <Menu/MenuHUD.h>

#include <SDK/InputSystem.h>
#include <SDK/GlobalVars.h>
#include <SDK/Engine.h> 
#include <SDK/Entity.h>
#include <SDK/LocalPlayer.h>
#include <SDK/NetworkChannel.h> 
#include <SDK/Steam.h>

#include <Encryption/xorstr.hpp>

#include <Config.h>
#include <ConfigStructs.h>
#include <Helpers.h>
#include <Interfaces.h>
#include <GameData.h> 
#include "../Memory.h"


ImDrawList* MenuHUD::drawHUD(ImDrawList* drawList, ImVec2 displaySize) noexcept
{
    drawList = drawHUDElement_BottomCenter(drawList, displaySize);
    drawList = drawHUDElement_Leftbottom(drawList, displaySize);
    drawList = drawHUDElement_LeftMiddle(drawList, displaySize);
    drawList = drawHUDElement_LeftTop(drawList, displaySize);
    drawList = drawHUDElement_TopCenter(drawList, displaySize);
    drawList = drawHUDElement_RightTop(drawList, displaySize);
    drawList = drawHUDElement_RightMiddle(drawList, displaySize);
    drawList = drawHUDElement_Rightbottom(drawList, displaySize);
    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_LeftTop(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_LeftMiddle(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_Leftbottom(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_TopCenter(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_RightTop(ImDrawList* drawList, ImVec2 displaySize) noexcept
{
    //NAME
    const char* name = interfaces->engine->getSteamAPIContext()->steamFriends->getPersonaName();

    //FPS
    static auto fps = 1.0f;
    fps = 0.9f * fps + 0.1f * memory->globalVars->absoluteFrameTime;

    //PING
    const auto ping = GameData::getNetOutgoingLatency();

    //TICKRATE
    const auto tick = 1.f / memory->globalVars->intervalPerTick;

    //TIME
    std::time_t t = std::time(nullptr);
    std::ostringstream time;
    time << std::put_time(std::localtime(&t), ("%H:%M:%S"));

    std::ostringstream format;
    format << "Spatial"
        << " | " << name
        << " | " << (fps != 0.0f ? static_cast<int>(1 / fps) : 0) << " fps";

    if (interfaces->engine->isClientLocalToActiveServer()) {
        format << " | local " << tick << " tick";
    }
    else if (interfaces->engine->isInGame()) {
        auto* pInfo = interfaces->engine->getNetworkChannel();
        if (pInfo) {
            if ((*memory->gameRules)->isValveDS())
                format << " | official DS " << pInfo->getAddress();
            else
                format << " | online " << pInfo->getAddress();

            format << " | " << ping << " ms " << tick << " tick";
        }
    }
    else if (interfaces->engine->isConnected())
        format << " | loading";

    format << " | " << time.str().c_str();

    const auto textSize = ImGui::CalcTextSize(format.str().c_str());

    //Top Right
    ImRect window{
        displaySize.x - textSize.x - 9.f,
        1.f,
        displaySize.x - 1.f,
        textSize.y + 9.f
    };

    drawList->AddRectFilled(window.Min, window.Max, ImGui::GetColorU32(ImGuiCol_WindowBg), 4);
    const int vertStartIdx = drawList->VtxBuffer.Size;
    drawList->AddRect(window.Min, window.Max, ImGui::GetColorU32(ImGuiCol_TitleBgActive), 4);
    const int vertEndIdx = drawList->VtxBuffer.Size;

    float r, g, b;
    std::tie(r, g, b) = rainbowColor(3.f);
    Helpers::shadeVertsHSVColorGradientKeepAlpha(drawList, vertStartIdx, vertEndIdx, window.GetTL(), window.GetBR(), ImColor(r, g, b, 1.f), ImGui::GetColorU32(ImGuiCol_TitleBgActive));

    ImVec2 textPos{
        window.GetCenter().x - (textSize.x / 2),
        window.GetCenter().y - (textSize.y / 2)
    };
    drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), format.str().c_str());

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_RightMiddle(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_Rightbottom(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}

ImDrawList* MenuHUD::drawHUDElement_BottomCenter(ImDrawList* drawList, ImVec2 displaySize) noexcept
{

    return drawList;
}