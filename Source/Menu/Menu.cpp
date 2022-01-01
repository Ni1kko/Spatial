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
#include <Menu/Texture.h>
#include <Menu/imguiCustom.h>
#include <Menu/Menu.h>
#include <Menu/MenuHUD.h>
#include <Menu/PNGTexture.h>

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

#include <InventoryChanger/InventoryChanger.h>
#include <Hacks/Visuals.h>
#include <Hacks/Glow.h>
#include <Hacks/AntiAim.h>
#include <Hacks/Backtrack.h>
#include <Hacks/Sound.h>
#include <Hacks/StreamProofESP.h>
#include <Hacks/Misc.h>
#include <Hacks/Troll.h>
#include <Hacks/Tickbase.h>
#include <Hacks/Movement.h>
#include <Hacks/Triggerbot.h>
#include <Hacks/Chams.h>
#include <Hacks/Aimbot.h>

const auto menuTitle = Helpers::getDllNameVersion();
const auto menuFooter = "Compile timestamp: " + std::string{ Helpers::compileTimestamp() };

Menu::Menu() noexcept
{
    ImGuiCustom::updateColors(static_cast<ImGuiStyles>(config->style.menuColors));

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    style.ScrollbarSize = 9.0f;

    cfg.SizePixels = 15.0f;
    
    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);
        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.normal15px) io.Fonts->AddFontDefault(&cfg);
        cfg.MergeMode = true;
        static constexpr ImWchar symbol[]{ 0x2605, 0x2605,0};
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }

    if (!fonts.normal15px) io.Fonts->AddFontDefault(&cfg);
    Helpers::addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
    Helpers::addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);

    //--- User Config
    //config->load(u8"default", false);
    
    //--- Console welcome
    Helpers::showWelcomeMessage();
}

void Menu::render(ImDrawList* drawList, ImVec2 displaySize) noexcept
{
    if (Misc::isMenuKeyPressed()) 
    {
        open = !open;
        if (!open)
            interfaces->inputSystem->resetInputState();

        Helpers::writeInGameChat(open ? xorstr_("Menu Open") : xorstr_("Menu Closed"), open ? ColorByte::Green : ColorByte::Red);

        //TODO: Move out this scope and have it on timer
        Helpers::showDiscordUrl(ColorByte::Purple);
    }
    
    //Spatial HUD
    drawList = MenuHUD::drawHUD(drawList, displaySize);

    //Spatial Menu
    drawList = drawGUI(drawList, displaySize);
}

ImDrawList* Menu::drawGUI(ImDrawList* drawList, ImVec2 displaySize) noexcept
{
    if (!open) 
        return drawList;
    
    //drawList->AddImage(menuBG.getTexture(), { 0, 0 }, { ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.x / 960 * 174 }, { 0, 0 }, { 1, 0.99f }, 0x00FFFFFF | (static_cast<unsigned>(0.9f + 255 * 0.1f) << IM_COL32_A_SHIFT));

    ImGui::Begin(menuTitle.c_str(), &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar); {
        if (ImGui::TreeNode(xorstr_("Aim"))) {
            Aimbot::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("AntiAim"))) {
            AntiAim::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Triggerbot"))) {
            Triggerbot::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("BackTrack"))) {
            Backtrack::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Movement"))) {
            movement->drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Network"))) {
            Tickbase::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Glow"))) {
            Glow::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Chams"))) {
            chams->drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("ESP"))) {
            StreamProofESP::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Visuals"))) {
            Visuals::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Inventory"))) {
            InventoryChanger::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Sound"))) {
            Sound::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Misc"))) {
            Misc::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Troll"))) {
            Troll::drawGUI();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Config"))) {
            config->drawGUI();
            ImGui::TreePop();
        }
        ImGui::Separator();
        ImGui::TextUnformatted(menuFooter.c_str());
    }
    ImGui::End();
    return drawList;
}