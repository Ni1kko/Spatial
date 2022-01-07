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
}

void Menu::render() noexcept
{ 
    Helpers::rainbowMenuBorder();
    renderDemo();
    renderTest();
}

void Menu::renderTest() noexcept
{
    ImGui::Begin("Spatial Dev", &open_test, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);{
        if (ImGui::TreeNode(xorstr_("Aim"))) {
            //
            ImGui::TreePop();
        }
        if (ImGui::TreeNode(xorstr_("AntiAim"))) {
            //
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