#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#endif

#include <Menu/imgui/imgui.h>
#include <Menu/imgui/imgui_stdlib.h>
#include <Menu/imguiCustom.h>
#include <Menu/Menu.h>

#include <SDK/InputSystem.h>
#include <SDK/GlobalVars.h>

#include <Encryption/xorstr.hpp>

#include <Config.h>
#include <ConfigStructs.h>
#include <Helpers.h>
#include <Interfaces.h>
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

const auto menuTitle = "Spatial v1.4 r2";
std::string menuFooter = "Compile timestamp: [" + std::string{ __TIME__ } + "] " +  __DATE__;

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
{
    auto file = Helpers::loadBinaryFile(path);
    if (!Helpers::decodeVFONT(file))
        return nullptr;

    ImFontConfig cfg;
    cfg.FontData = file.data();
    cfg.FontDataSize = file.size();
    cfg.FontDataOwnedByAtlas = false;
    cfg.MergeMode = merge;
    cfg.GlyphRanges = glyphRanges;
    cfg.SizePixels = size;

    return ImGui::GetIO().Fonts->AddFont(&cfg);
}

void Menu::handleToggle() noexcept
{
    if (Misc::isMenuKeyPressed()) {
        open = !open;
        if (!open)
            interfaces->inputSystem->resetInputState();
#ifndef _WIN32
        ImGui::GetIO().MouseDrawCursor = gui->open;
#endif
    }
}

Menu::Menu() noexcept
{
    ImGuiCustom::updateColors(ImGuiStyles::Spatial);
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 9.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFontConfig cfg;
    cfg.SizePixels = 15.0f;

#ifdef _WIN32
    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);

        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.normal15px)
            io.Fonts->AddFontDefault(&cfg);

        cfg.MergeMode = true;
        static constexpr ImWchar symbol[]{
            0x2605, 0x2605, // ★
            0
        };
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }
#else
    fonts.normal15px = addFontFromVFONT("csgo/panorama/fonts/notosans-regular.vfont", 15.0f, Helpers::getFontGlyphRanges(), false);
#endif
    if (!fonts.normal15px)
        io.Fonts->AddFontDefault(&cfg);
    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);

    //config->load(u8"default", false);
}

void Menu::render() noexcept
{
    ImGui::Begin(menuTitle, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar); {
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
}