#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>
#include <ShlObj.h>
#include <Windows.h>
#include <iostream>
#include <filesystem>

#include "../ImGui/imgui.h"
#include "imguiCustom.h"
#include "Menu.h"

#include "../Helpers.h"

Menu::Menu() noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFontConfig cfg;
    
    ImGuiCustom::updateColors(ImGuiStyles::Spatial);

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    cfg.SizePixels = 15.0f;
    
    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);
        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.normal15px) io.Fonts->AddFontDefault(&cfg);
        cfg.MergeMode = true;
        static constexpr ImWchar symbol[]{ 0x2605, 0x2605,0 };
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }

    if (!fonts.normal15px) io.Fonts->AddFontDefault(&cfg);

    window.name = xorstr_("Spatial Dev");
    window.flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    window.tabs.aim.name = xorstr_("Aim");
    window.tabs.antiaim.name = xorstr_("AntiAim");
    window.tabs.triggerbot.name = xorstr_("Triggerbot");
    window.tabs.backtrack.name = xorstr_("BackTrack");
    window.tabs.movement.name = xorstr_("Movement");
    window.tabs.network.name = xorstr_("Network");
    window.tabs.glow.name = xorstr_("Glow");
    window.tabs.chams.name = xorstr_("Chams");
    window.tabs.esp.name = xorstr_("ESP");
    window.tabs.visuals.name = xorstr_("Visuals");
    window.tabs.inventory.name = xorstr_("Inventory");
    window.tabs.sound.name = xorstr_("Sound");
    window.tabs.misc.name = xorstr_("Misc");
    window.tabs.troll.name = xorstr_("Troll");
    window.tabs.config.name = xorstr_("Config");
}

void Menu::render() noexcept
{ 
    Helpers::rainbowMenuBorder();
    renderDemo();
    renderTest();
}
  
void Menu::tabShow(Tab* tab) noexcept
{
    window.tabs.aim.open = false;
    window.tabs.antiaim.open = false;
    window.tabs.triggerbot.open = false;
    window.tabs.backtrack.open = false;
    window.tabs.movement.open = false;
    window.tabs.network.open = false;
    window.tabs.glow.open = false;
    window.tabs.esp.open = false;
    window.tabs.visuals.open = false;
    window.tabs.inventory.open = false;
    window.tabs.sound.open = false;
    window.tabs.misc.open = false;
    window.tabs.troll.open = false;
    window.tabs.config.open = false;
    tab->open = true;
}

void Menu::renderTest() noexcept
{
    ImGui::Begin(window.name, &window.open, window.flags);{
         
        if (ImGui::TreeNode(window.tabs.aim.name)) tabShow(&window.tabs.aim); else window.tabs.aim.open = false;
        if (window.tabs.aim.open) {
            ImGui::TextUnformatted("aim test tab line1");
            ImGui::TextUnformatted("aim test tab line2");
            ImGui::TextUnformatted("aim test tab line3");
            ImGui::TextUnformatted("aim test tab line4");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode(window.tabs.antiaim.name)) tabShow(&window.tabs.antiaim); else window.tabs.antiaim.open = false;
        if (window.tabs.antiaim.open) {
            ImGui::TextUnformatted("antiaim test tab line1");
            ImGui::TextUnformatted("antiaim test tab line2");
            ImGui::TextUnformatted("antiaim test tab line3");
            ImGui::TextUnformatted("antiaim test tab line4");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode(xorstr_("Triggerbot"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("BackTrack"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Movement"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Network"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Glow"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Chams"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("ESP"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Visuals"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Inventory"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Sound"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Misc"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Troll"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("Config"))) {
            //
            ImGui::TreePop();
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Footer text");
    }
    ImGui::End(); 
}

void Menu::renderDemo() noexcept
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (isOpen())
        ImGui::ShowDemoWindow(&open);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &open);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &open2);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (isOpen2())
    {
        ImGui::Begin("Another Window", &open2);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            open2 = false;
        ImGui::End();
    }

}