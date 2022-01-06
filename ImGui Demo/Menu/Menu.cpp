#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>
#include <ShlObj.h>
#include <Windows.h>

#include "..\ImGui\imgui.h"
#include "..\ImGui\imgui_impl_dx9.h"
#include "..\ImGui\imgui_impl_win32.h"

#include "Menu.h"

static bool render_demo = true;

Menu::Menu() noexcept
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
}

void Menu::render() noexcept
{ 
    renderDemo();
    renderTest();
}

void Menu::renderTest() noexcept
{
    ImGui::Begin("Test window", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar); {
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