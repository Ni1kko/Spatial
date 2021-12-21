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


constexpr auto windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

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

void Menu::updateColors() const noexcept
{
    switch (config->style.menuColors) {
        case 0: ImGui::StyleColorsDark(); break;
        case 1: ImGui::StyleColorsLight(); break;
        case 2: ImGui::StyleColorsClassic(); break;
    }
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
    ImGui::StyleColorsDark();
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

    /*if (!autoload) {
        static int cc = 0;
        config->load(cc, false);
        autoload = true;
    }*/
}

void Menu::cbox_colorpicker(const std::string& name, bool* enable, float* color) noexcept
{
    ImGui::Checkbox(("##" + name).c_str(), enable);
    ImGui::SameLine(0.0f, 5.0f);
    ImGui::PushID(0);
    bool openPopup = ImGui::ColorButton(("##" + name).c_str(), ImColor{ color[0], color[1], color[2] }, ImGuiColorEditFlags_NoTooltip);
    ImGui::PopID();
    ImGui::SameLine(0.0f, 5.0f);
    ImGui::TextUnformatted(name.c_str());
    ImGui::PushID(1);
    if (openPopup)
        ImGui::OpenPopup(("##" + name).c_str());
    if (ImGui::BeginPopup(("##" + name).c_str())) {
        ImGui::PushID(2);
        ImGui::ColorPicker3(("##" + name).c_str(), color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview);
        ImGui::PopID();
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void Menu::change_keybind(int& key) noexcept
{
    key ? ImGui::Text(xorstr_("[ 0x%x ]"), key) : ImGui::TextUnformatted(xorstr_("[ key ]"));
    if (ImGui::IsItemHovered()) {
        showTooltip(xorstr_("Press any key to change keybind"));
        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
            if (ImGui::IsKeyPressed(i)) key = i != VK_ESCAPE ? i : 0;
        }
        for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
            if (ImGui::IsMouseDown(i) /* && i + (i > 1 ? 2 : 1) != Misc::menuKeyBind()*/) key = i + (i > 1 ? 2 : 1);
        }
    };
}

void Menu::showTooltip(const char* Tooltip) noexcept
{
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(Tooltip);
}

void Menu::showAlert(bool display, std::string title, std::string message) noexcept
{
    static float cur_time{ 0.f };

    if (title.length() < 3) title = { xorstr_("Spatial | Alert") };
    if (message.length() < 1) return;
    if (alerts_notification.size() > 15) alerts_notification.pop_front();

    if (display || isAlertPending && !isAlertOpen)
    {
        isAlertOpen = true;

        for (size_t i = 0; i < alerts_notification.size(); i++) {
            auto& notification = alerts_notification.at(i);

            title = notification.m_title.c_str();
            message = notification.m_msg.c_str();
            cur_time = notification.m_time;
        } 
    }
    else
    {
        if (isAlertOpen && !isAlertPending || memory->globalVars->currenttime - cur_time <= alertsDuration)
        {
            isAlertPending = true;
            for (size_t i = 0; i < alerts_notification.size(); i++) {
                auto& notification = alerts_notification.at(i); 
                notification.m_title = title.c_str();
                notification.m_msg = message.c_str();
                notification.m_time = memory->globalVars->currenttime + alertsDuration;
            }
        }
    }

    if (isAlertOpen && memory->globalVars->currenttime - cur_time > alertsDuration) isAlertOpen = false;

}

void Menu::clearAlerts() noexcept
{
    alerts_notification.clear();
}

void Menu::render() noexcept
{
    renderAlerts();
    ImGui::Begin(xorstr_("Spatial"), &open, windowFlags); {
        renderNodes();
        ImGui::Separator();
        ImGui::TextUnformatted(std::string{ (xorstr_("Spatial | Compiled On The : ") + std::string { __DATE__ }) }.c_str());
    }
    ImGui::End();
}

void Menu::renderAlerts() noexcept
{
    int adjust_height = 5;

    for (size_t i = 0; i < alerts_notification.size(); i++) {
        auto& notification = alerts_notification.at(i);

        // notification.color;
        // = notification.background_color; 
        const char* title = notification.m_title.c_str();
        const char* message = notification.m_msg.c_str();
        const float cur_time = notification.m_time;

        if (memory->globalVars->serverTime() - cur_time > alertsDuration)
        {
            const float DISTANCE = 10.0f;
            static int corner = 0;
            ImGuiIO& io = ImGui::GetIO();
            if (corner != -1)
            {
                ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
                ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            }
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
            if (ImGui::Begin(title, &isAlertOpen, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
            {
                ImGui::Text(message);
                ImGui::Separator();
                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
                    if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
                    if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
                    if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
                    if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
                    if (&isAlertOpen && ImGui::MenuItem("Close"))*&isAlertOpen = false;
                    ImGui::EndPopup();
                }
            };
            ImGui::End();
        }
    }
}

void Menu::renderNodes() noexcept
{
    if (ImGui::TreeNode(xorstr_("Aim"))) {
        renderAimbotWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("AntiAim"))) {
        renderAntiAimWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Triggerbot"))) {
        renderTriggerbotWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("BackTrack"))) {
        renderBackTrackWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Movement"))) {
        renderMovementWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Network"))) {
        renderNetworkWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Glow"))) {
        renderGlowWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Chams"))) {
        renderChamsWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("ESP"))) {
        renderESPWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Visuals"))) {
        renderVisualsWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Inventory"))) {
        renderInventoryWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Sound"))) {
        renderSoundWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Style"))) {
        renderStyleWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Misc"))) {
        renderMiscWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Troll"))) {
        renderTrollWindow();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(xorstr_("Config"))) {
        renderConfigWindow();
        ImGui::TreePop();
    }
}

void Menu::renderAimbotWindow() noexcept
{
    ImGui::Checkbox("On key", &config->aimbotOnKey);
    ImGui::SameLine();
    ImGui::PushID("Aimbot Key");
    ImGui::hotkey("", config->aimbotKey);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushItemWidth(70.0f);
    ImGui::Combo("", &config->aimbotKeyMode, "Hold\0Toggle\0");
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::SameLine();
    ImGuiCustom::colorPicker("Draw AimBot FOV", config->drawaimbotFov);
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->aimbot[currentWeapon].enabled);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 220.0f);
    ImGui::Checkbox("Aimlock", &config->aimbot[currentWeapon].aimlock);
    ImGui::Checkbox("Silent", &config->aimbot[currentWeapon].silent);
    ImGui::Checkbox("Friendly fire", &config->aimbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Visible only", &config->aimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->aimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->aimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->aimbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Auto shot", &config->aimbot[currentWeapon].autoShot);
    ImGui::Checkbox("Auto scope", &config->aimbot[currentWeapon].autoScope);
    ImGui::Checkbox("Auto stop", &config->aimbot[currentWeapon].autoStop);
    ImGui::Combo("Bone", &config->aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(240.0f);
    ImGui::SliderFloat("Fov", &config->aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Smooth", &config->aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Max aim inaccuracy", &config->aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max shot inaccuracy", &config->aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::InputInt("Min damage", &config->aimbot[currentWeapon].minDamage);
    config->aimbot[currentWeapon].minDamage = std::clamp(config->aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Between shots", &config->aimbot[currentWeapon].betweenShots);
    ImGui::Columns(1);
}

void Menu::renderAntiAimWindow() noexcept
{
    AntiAim::drawGUI(true);
}

void Menu::renderTriggerbotWindow() noexcept
{
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0Zeus x27\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 5:
        currentWeapon = 39;
        ImGui::NewLine();
        break;

    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void*, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            }
            else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->triggerbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::hotkey("Hold Key", config->triggerbotHoldKey);
    ImGui::Checkbox("Friendly fire", &config->triggerbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Scoped only", &config->triggerbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->triggerbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->triggerbot[currentWeapon].ignoreSmoke);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::Combo("Hitgroup", &config->triggerbot[currentWeapon].hitgroup, "All\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Shot delay", &config->triggerbot[currentWeapon].shotDelay, 0, 250, "%d ms");
    ImGui::InputInt("Min damage", &config->triggerbot[currentWeapon].minDamage);
    config->triggerbot[currentWeapon].minDamage = std::clamp(config->triggerbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->triggerbot[currentWeapon].killshot);
    ImGui::SliderFloat("Burst Time", &config->triggerbot[currentWeapon].burstTime, 0.0f, 0.5f, "%.3f s");
}

void Menu::renderMovementWindow() noexcept
{
    Movement::drawGUI(true);
}

void Menu::renderBackTrackWindow() noexcept
{
    Backtrack::drawGUI(true);
}

void Menu::renderNetworkWindow() noexcept
{
    Tickbase::drawGUI(true);
}

void Menu::renderGlowWindow() noexcept
{
    Glow::drawGUI(true);
}

void Menu::renderChamsWindow() noexcept
{
    ImGui::hotkey("Toggle Key", config->chamsToggleKey, 110.0f);
    ImGui::hotkey("Hold Key", config->chamsHoldKey, 110.0f);
    ImGui::Separator();

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);

    static int material = 1;

    if (ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Weapons\0Hands\0Backtrack\0Sleeves\0"))
        material = 1;

    ImGui::PopID();

    ImGui::SameLine();

    if (material <= 1)
        ImGuiCustom::arrowButtonDisabled("##left", ImGuiDir_Left);
    else if (ImGui::ArrowButton("##left", ImGuiDir_Left))
        --material;

    ImGui::SameLine();
    ImGui::Text("%d", material);

    constexpr std::array categories{ "Allies", "Enemies", "Planting", "Defusing", "Local player", "Weapons", "Hands", "Backtrack", "Sleeves" };

    ImGui::SameLine();

    if (material >= int(config->chams[categories[currentCategory]].materials.size()))
        ImGuiCustom::arrowButtonDisabled("##right", ImGuiDir_Right);
    else if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        ++material;

    ImGui::SameLine();

    auto& chams{ config->chams[categories[currentCategory]].materials[material - 1] };

    ImGui::Checkbox("Enabled", &chams.enabled);
    ImGui::Separator();
    ImGui::Checkbox("Health based", &chams.healthBased);
    ImGui::Checkbox("Blinking", &chams.blinking);
    ImGui::Combo("Material", &chams.material, "Normal\0Flat\0Animated\0Platinum\0Glass\0Chrome\0Crystal\0Silver\0Gold\0Plastic\0Glow\0Pearlescent\0Metallic\0");
    ImGui::Checkbox("Wireframe", &chams.wireframe);
    ImGui::Checkbox("Cover", &chams.cover);
    ImGui::Checkbox("Ignore-Z", &chams.ignorez);
    ImGuiCustom::colorPicker("Color", chams);
}

void Menu::renderESPWindow() noexcept
{
    StreamProofESP::drawGUI(true);
}

void Menu::renderVisualsWindow() noexcept
{
    Visuals::drawGUI(true);
}

void Menu::renderInventoryWindow() noexcept
{ 
    InventoryChanger::drawGUI(true);
}

void Menu::renderSoundWindow() noexcept
{
    Sound::drawGUI(true);
}

void Menu::renderMiscWindow() noexcept
{
    Misc::drawGUI(true);
}

void Menu::renderTrollWindow() noexcept
{
    Troll::drawGUI(true);
}

void Menu::renderStyleWindow() noexcept
{
    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark\0Light\0Classic\0Custom\0"))
        updateColors();
    ImGui::PopItemWidth();

    if (config->style.menuColors == 3) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], &style.Colors[i].w);
        }
    }
}

void Menu::renderConfigWindow() noexcept
{
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 170.0f);

    static bool incrementalLoad = false;
    ImGui::Checkbox("Incremental Load", &incrementalLoad);

    ImGui::PushItemWidth(160.0f);

    auto& configItems = config->getConfigs();
    static int currentConfig = -1;

    static std::u8string buffer;

    timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;
    if (timeToNextConfigRefresh <= 0.0f) {
        config->listConfigs();
        if (const auto it = std::find(configItems.begin(), configItems.end(), buffer); it != configItems.end())
            currentConfig = std::distance(configItems.begin(), it);
        timeToNextConfigRefresh = 0.1f;
    }

    if (static_cast<std::size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::u8string>*>(data);
        *out_text = (const char*)vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 5) && currentConfig != -1)
        buffer = configItems[currentConfig];

        ImGui::PushID(0);
        if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer);
        }
        ImGui::PopID();
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        if (ImGui::Button("Open config directory"))
            config->openConfigDir();

        if (ImGui::Button("Create config", { 100.0f, 25.0f }))
            config->add(buffer.c_str());

        if (ImGui::Button("Reset config", { 100.0f, 25.0f }))
            ImGui::OpenPopup("Config to reset");

        if (ImGui::BeginPopup("Config to reset")) {
            static constexpr const char* names[]{ "Whole", "Aimbot", "Triggerbot", "Backtrack", "Movement", "Anti aim", "Glow", "Chams", "ESP", "Visuals", "Inventory Changer", "Sound", "Style", "Misc", "Troll", "TickFucker" };
            for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                if (i == 1) ImGui::Separator();

                if (ImGui::Selectable(names[i])) {
                    switch (i) {
                        case 0: config->reset(); updateColors(); Misc::updateClanTag(true); InventoryChanger::scheduleHudUpdate(); break;
                        case 1: config->aimbot = { }; break;
                        case 2: config->triggerbot = { }; break;
                        case 3: Backtrack::resetConfig(); break;
                        case 4: Movement::resetConfig(); break;
                        case 5: AntiAim::resetConfig(); break;
                        case 6: Glow::resetConfig(); break;
                        case 7: config->chams = { }; break;
                        case 8: config->streamProofESP = { }; break;
                        case 9: Visuals::resetConfig(); break;
                        case 10: InventoryChanger::resetConfig(); InventoryChanger::scheduleHudUpdate(); break;
                        case 11: Sound::resetConfig(); break;
                        case 12: config->style = { }; updateColors(); break;
                        case 13: Troll::resetConfig(); break;
                        case 14: Tickbase::resetConfig(); break;
                    }
                }
            }
            ImGui::EndPopup();
        }
        if (currentConfig != -1) {
            if (ImGui::Button("Load selected", { 100.0f, 25.0f })) {
                config->load(currentConfig, incrementalLoad);
                updateColors();
                InventoryChanger::scheduleHudUpdate();
                Misc::updateClanTag(true);
            }
            if (ImGui::Button("Save selected", { 100.0f, 25.0f }))
                config->save(currentConfig);
            if (ImGui::Button("Delete selected", { 100.0f, 25.0f })) {
                config->remove(currentConfig);

                if (static_cast<std::size_t>(currentConfig) < configItems.size())
                    buffer = configItems[currentConfig];
                else
                    buffer.clear();
            }
        }
        ImGui::Columns(1);
}