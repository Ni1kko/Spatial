#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <system_error>

#include <Windows.h>
#include <shellapi.h>
#include <ShlObj.h>

#include <Encryption/xorstr.hpp>

#include "nlohmann/json.hpp"

#include <Menu/imgui/imgui.h>
#include <Menu/imgui/imgui_stdlib.h>
#include <Menu/imguiCustom.h>

#include "Config.h"

#include "Hacks/Sound.h"
#include "Hacks/Visuals.h"
#include "Hacks/Misc.h"
#include <Hacks/Movement.h>
#include <Helpers.h>
#include <InventoryChanger/InventoryConfig.h>
#include <InventoryChanger/InventoryChanger.h>

int CALLBACK fontCallback(const LOGFONTW* lpelfe, const TEXTMETRICW*, DWORD, LPARAM lParam)
{
    const wchar_t* const fontName = reinterpret_cast<const ENUMLOGFONTEXW*>(lpelfe)->elfFullName;

    if (fontName[0] == L'@')
        return TRUE;

    if (HFONT font = CreateFontW(0, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, fontName)) {

        DWORD fontData = GDI_ERROR;

        if (HDC hdc = CreateCompatibleDC(nullptr)) {
            SelectObject(hdc, font);
            // Do not use TTC fonts as we only support TTF fonts
            fontData = GetFontData(hdc, 'fctt', 0, NULL, 0);
            DeleteDC(hdc);
        }
        DeleteObject(font);

        if (fontData == GDI_ERROR) {
            if (char buff[1024]; WideCharToMultiByte(CP_UTF8, 0, fontName, -1, buff, sizeof(buff), nullptr, nullptr))
                reinterpret_cast<std::vector<std::string>*>(lParam)->emplace_back(buff);
        }
    }
    return TRUE;
}

[[nodiscard]] static std::filesystem::path buildConfigsFolderPath() noexcept
{
    std::filesystem::path path;

    if (PWSTR pathToDocuments; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathToDocuments))) {
        path = pathToDocuments;
        CoTaskMemFree(pathToDocuments);
    }

    path /= xorstr_("Spatial");
    return path;
}

Config::Config() noexcept : path{ buildConfigsFolderPath() }
{
    listConfigs();

    load(u8"default.json", false);

    LOGFONTW logfont;
    logfont.lfCharSet = ANSI_CHARSET;
    logfont.lfPitchAndFamily = DEFAULT_PITCH;
    logfont.lfFaceName[0] = L'\0';

    EnumFontFamiliesExW(GetDC(nullptr), &logfont, fontCallback, (LPARAM)&systemFonts, 0);

    std::sort(std::next(systemFonts.begin()), systemFonts.end());
}

KeyBind Config::menuKeyBind() noexcept
{
    return config->menuKey;
}

bool Config::isMenuKeyPressed() noexcept
{
    return menuKeyBind().isPressed();
}

void Config::drawGUI() noexcept
{
    ImGui::hotkey("Menu Key", config->menuKey);
    ImGui::SameLine();
    ImGui::PushItemWidth(160.0f);
    if (ImGui::Combo(xorstr_("Menu Theme"), &style.menuColors, xorstr_("Dark\0Light\0Classic\0Spatial\0Custom\0")))
        ImGuiCustom::updateColors(static_cast<ImGuiStyles>(style.menuColors));
    ImGui::PopItemWidth();

    if (style.menuColors == 4) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));
            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], &style.Colors[i].w);
        }
    }
    ImGui::Separator();

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 190.0f);
    
    static bool incrementalLoad = false;
    //ImGui::Checkbox(xorstr_("Incremental Load"), &incrementalLoad);
    ImGui::TextUnformatted("Saved Conifgs");
    ImGui::PushItemWidth(160.0f);

    auto& configItems = getConfigs();
    static int currentConfig = -1;

    static const ImVec2 btnsize = { -1, 20 }; //{ 100.0f, 25.0f };
    static const ImVec2 btnsize_popup = { 45.0f, 0.0f };

    static std::u8string buffer;

    timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;
    if (timeToNextConfigRefresh <= 0.0f) {
        listConfigs();
        if (const auto it = std::find(configItems.begin(), configItems.end(), buffer); it != configItems.end())
            currentConfig = std::distance(configItems.begin(), it);
        timeToNextConfigRefresh = 0.1f;
    }

    if (static_cast<std::size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) { auto& vector = *static_cast<std::vector<std::u8string>*>(data); *out_text = (const char*)vector[idx].c_str();return true;}, &configItems, configItems.size(), 5) && currentConfig != -1)
        buffer = configItems[currentConfig];
        
        if (ImGui::BeginPopup("##create_new"))
        {
            ImGui::TextUnformatted("Enter Config name");
            ImGui::PushID(0);
            if (ImGui::InputTextWithHint("", xorstr_("config name"), &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) { if (currentConfig != -1) rename(currentConfig, buffer); }
            ImGui::PopID();
            if (ImGui::Button("Save", btnsize_popup)) { add(buffer.c_str()); ImGui::CloseCurrentPopup(); } ImGui::SameLine();
            if (ImGui::Button("Abort", btnsize_popup)) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        ImGui::NewLine();//push buttons inline with listbox
         
        //-- Open config directory
        if (ImGui::Button(xorstr_("Open Directory"), btnsize)) openConfigDir();

        //-- Create config
        if (ImGui::Button(xorstr_("New"), btnsize)) ImGui::OpenPopup("##create_new");
         
        if (currentConfig != -1) 
        {
            //-- Load config
            if (ImGui::Button(xorstr_("Load"), btnsize)) ImGui::OpenPopup("##load_check");
            if (ImGui::BeginPopup("##load_check"))
            {
                ImGui::TextUnformatted("Are you sure?");
                if (ImGui::Button("No", btnsize_popup)) ImGui::CloseCurrentPopup();
                ImGui::SameLine();
                if (ImGui::Button("Yes", btnsize_popup))
                {
                    load(currentConfig, incrementalLoad);
                    ImGuiCustom::updateColors(static_cast<ImGuiStyles>(style.menuColors));
                    InventoryChanger::scheduleHudUpdate();
                    Misc::updateClanTag(true);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            //-- Save config
            if (ImGui::Button(xorstr_("Save"), btnsize)) ImGui::OpenPopup("##save_check");
            if (ImGui::BeginPopup("##save_check"))
            {
                ImGui::TextUnformatted("Are you sure?");
                if (ImGui::Button("No", btnsize_popup)) ImGui::CloseCurrentPopup();
                ImGui::SameLine();
                if (ImGui::Button("Yes", btnsize_popup))
                {
                    save(currentConfig);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            //-- Reset config
            if (ImGui::Button(xorstr_("Reset"), btnsize)) ImGui::OpenPopup(xorstr_("Config to reset"));
            if (ImGui::BeginPopup(xorstr_("Config to reset"))) {
                static constexpr const char* names[]{ "Whole", "Aimbot", "Triggerbot", "Backtrack", "Movement", "Anti aim", "Glow", "Chams", "ESP", "Visuals", "Inventory Changer", "Sound", "Style", "Misc", "Troll", "TickFucker" };
                for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                    if (i == 1) ImGui::Separator();

                    if (ImGui::Selectable(names[i])) {
                        switch (i) 
                        {
                            case 0: config->reset(); ImGuiCustom::updateColors(static_cast<ImGuiStyles>(config->style.menuColors)); Misc::updateClanTag(true); InventoryChanger::scheduleHudUpdate(); break;
                            case 1: config->aimbot = { }; break;
                            case 2: config->triggerbot = { }; break;
                            case 3: movement->resetConfig(); break;
                            case 4: config->streamProofESP = { }; break;
                            case 5: Visuals::resetConfig(); break;
                            case 6: InventoryChanger::resetConfig(); InventoryChanger::scheduleHudUpdate(); break;
                            case 7: Sound::resetConfig(); break;
                            case 8: config->style = { }; ImGuiCustom::updateColors(static_cast<ImGuiStyles>(config->style.menuColors)); break;
                        }
                    }
                }
                ImGui::EndPopup();
            }

            //-- Delete config
            if (ImGui::Button(xorstr_("Delete"), btnsize)) ImGui::OpenPopup("##delete_check");
            if (ImGui::BeginPopup("##delete_check"))
            {
                ImGui::TextUnformatted("Are you sure?");
                if (ImGui::Button("No", btnsize_popup)) ImGui::CloseCurrentPopup();
                ImGui::SameLine();
                if (ImGui::Button("Yes", btnsize_popup))
                {
                    remove(currentConfig);

                    if (static_cast<std::size_t>(currentConfig) < configItems.size())
                        buffer = configItems[currentConfig];
                    else
                        buffer.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::Columns(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Read functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void from_json(const json& j, ColorToggleRounding& ctr)
{
    from_json(j, static_cast<ColorToggle&>(ctr));

    read(j, "Rounding", ctr.rounding);
}

static void from_json(const json& j, Font& f)
{
    read<value_t::string>(j, "Name", f.name);

    if (!f.name.empty())
        config->scheduleFontLoad(f.name);

    if (const auto it = std::ranges::find(config->getSystemFonts(), f.name); it != config->getSystemFonts().end())
        f.index = std::distance(config->getSystemFonts().begin(), it);
    else
        f.index = 0;
}

static void from_json(const json& j, Snapline& s)
{
    from_json(j, static_cast<ColorToggleThickness&>(s));

    read(j, "Type", s.type);
}

static void from_json(const json& j, Box& b)
{
    from_json(j, static_cast<ColorToggleRounding&>(b));

    read(j, "Type", b.type);
    read(j, "Scale", b.scale);
    read<value_t::object>(j, "Fill", b.fill);
}

static void from_json(const json& j, Shared& s)
{
    read(j, "Enabled", s.enabled);
    read<value_t::object>(j, "Font", s.font);
    read<value_t::object>(j, "Snapline", s.snapline);
    read<value_t::object>(j, "Box", s.box);
    read<value_t::object>(j, "Name", s.name);
    read(j, "Text Cull Distance", s.textCullDistance);
}

static void from_json(const json& j, Weapon& w)
{
    from_json(j, static_cast<Shared&>(w));

    read<value_t::object>(j, "Ammo", w.ammo);
}

static void from_json(const json& j, Trail& t)
{
    from_json(j, static_cast<ColorToggleThickness&>(t));

    read(j, "Type", t.type);
    read(j, "Time", t.time);
}

static void from_json(const json& j, Trails& t)
{
    read(j, "Enabled", t.enabled);
    read<value_t::object>(j, "Local Player", t.localPlayer);
    read<value_t::object>(j, "Allies", t.allies);
    read<value_t::object>(j, "Enemies", t.enemies);
}

static void from_json(const json& j, Projectile& p)
{
    from_json(j, static_cast<Shared&>(p));

    read<value_t::object>(j, "Trails", p.trails);
}

static void from_json(const json& j, Player& p)
{
    from_json(j, static_cast<Shared&>(p));

    read<value_t::object>(j, "Weapon", p.weapon);
    read<value_t::object>(j, "Flash Duration", p.flashDuration);
    read(j, "Audible Only", p.audibleOnly);
    read(j, "Spotted Only", p.spottedOnly);
    read<value_t::object>(j, "Health Bar", p.healthBar);
    read<value_t::object>(j, "Health", p.health);
    read<value_t::object>(j, "Skeleton", p.skeleton);
    read<value_t::object>(j, "Head Box", p.headBox);
}

static void from_json(const json& j, Config::Aimbot& a)
{
    read(j, "Enabled", a.enabled);
    read(j, "Aimlock", a.aimlock);
    read(j, "Silent", a.silent);
    read(j, "Friendly fire", a.friendlyFire);
    read(j, "Visible only", a.visibleOnly);
    read(j, "Scoped only", a.scopedOnly);
    read(j, "Ignore flash", a.ignoreFlash);
    read(j, "Ignore smoke", a.ignoreSmoke);
    read(j, "Auto shot", a.autoShot);
    read(j, "Auto scope", a.autoScope);
    read(j, "Auto stop", a.autoStop);
    read(j, "Fov", a.fov);
    read(j, "Smooth", a.smooth);
    read(j, "Bone", a.bone);
    read(j, "Max aim inaccuracy", a.maxAimInaccuracy);
    read(j, "Max shot inaccuracy", a.maxShotInaccuracy);
    read(j, "Min damage", a.minDamage);
    read(j, "Killshot", a.killshot);
    read(j, "Between shots", a.betweenShots);
}

static void from_json(const json& j, Config::Triggerbot& t)
{
    read(j, "Enabled", t.enabled);
    read(j, "Friendly fire", t.friendlyFire);
    read(j, "Scoped only", t.scopedOnly);
    read(j, "Ignore flash", t.ignoreFlash);
    read(j, "Ignore smoke", t.ignoreSmoke);
    read(j, "Hitgroup", t.hitgroup);
    read(j, "Shot delay", t.shotDelay);
    read(j, "Min damage", t.minDamage);
    read(j, "Killshot", t.killshot);
    read(j, "Burst Time", t.burstTime);
}

static void from_json(const json& j, Config::StreamProofESP& e)
{
    read(j, "Toggle Key", e.toggleKey);
    read(j, "Hold Key", e.holdKey);
    read(j, "Allies", e.allies);
    read(j, "Enemies", e.enemies);
    read(j, "Weapons", e.weapons);
    read(j, "Projectiles", e.projectiles);
    read(j, "Loot Crates", e.lootCrates);
    read(j, "Other Entities", e.otherEntities);
}

static void from_json(const json& j, Config::Style& s)
{ 
    read(j, "Menu colors", s.menuColors);

    if (j.contains("Colors") && j["Colors"].is_object()) {
        const auto& colors = j["Colors"];

        ImGuiStyle& style = ImGui::GetStyle();

        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (const char* name = ImGui::GetStyleColorName(i); colors.contains(name)) {
                std::array<float, 4> temp;
                read(colors, name, temp);
                style.Colors[i].x = temp[0];
                style.Colors[i].y = temp[1];
                style.Colors[i].z = temp[2];
                style.Colors[i].w = temp[3];
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void to_json(json& j, const ColorToggleRounding& o, const ColorToggleRounding& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
    WRITE("Rounding", rounding);
}

static void to_json(json& j, const ColorToggleThicknessRounding& o, const ColorToggleThicknessRounding& dummy = {})
{
    to_json(j, static_cast<const ColorToggleRounding&>(o), dummy);
    WRITE("Thickness", thickness);
}

static void to_json(json& j, const Font& o, const Font& dummy = {})
{
    WRITE("Name", name);
}

static void to_json(json& j, const Snapline& o, const Snapline& dummy = {})
{
    to_json(j, static_cast<const ColorToggleThickness&>(o), dummy);
    WRITE("Type", type);
}

static void to_json(json& j, const Box& o, const Box& dummy = {})
{
    to_json(j, static_cast<const ColorToggleRounding&>(o), dummy);
    WRITE("Type", type);
    WRITE("Scale", scale);
    WRITE("Fill", fill);
}

static void to_json(json& j, const Shared& o, const Shared& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Font", font);
    WRITE("Snapline", snapline);
    WRITE("Box", box);
    WRITE("Name", name);
    WRITE("Text Cull Distance", textCullDistance);
}

static void to_json(json& j, const Player& o, const Player& dummy = {})
{
    to_json(j, static_cast<const Shared&>(o), dummy);
    WRITE("Weapon", weapon);
    WRITE("Flash Duration", flashDuration);
    WRITE("Audible Only", audibleOnly);
    WRITE("Spotted Only", spottedOnly);
    WRITE("Health Bar", healthBar);
    WRITE("Health", health);
    WRITE("Skeleton", skeleton);
    WRITE("Head Box", headBox);
}

static void to_json(json& j, const Weapon& o, const Weapon& dummy = {})
{
    to_json(j, static_cast<const Shared&>(o), dummy);
    WRITE("Ammo", ammo);
}

static void to_json(json& j, const Trail& o, const Trail& dummy = {})
{
    to_json(j, static_cast<const ColorToggleThickness&>(o), dummy);
    WRITE("Type", type);
    WRITE("Time", time);
}

static void to_json(json& j, const Trails& o, const Trails& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Local Player", localPlayer);
    WRITE("Allies", allies);
    WRITE("Enemies", enemies);
}

static void to_json(json& j, const Projectile& o, const Projectile& dummy = {})
{
    j = static_cast<const Shared&>(o);

    WRITE("Trails", trails);
}

static void to_json(json& j, const ImVec2& o, const ImVec2& dummy = {})
{
    WRITE("X", x);
    WRITE("Y", y);
}

static void to_json(json& j, const Config::Aimbot& o, const Config::Aimbot& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Aimlock", aimlock);
    WRITE("Silent", silent);
    WRITE("Friendly fire", friendlyFire);
    WRITE("Visible only", visibleOnly);
    WRITE("Scoped only", scopedOnly);
    WRITE("Ignore flash", ignoreFlash);
    WRITE("Ignore smoke", ignoreSmoke);
    WRITE("Auto shot", autoShot);
    WRITE("Auto scope", autoScope);
    WRITE("Auto stop", autoStop);
    WRITE("Fov", fov);
    WRITE("Smooth", smooth);
    WRITE("Bone", bone);
    WRITE("Max aim inaccuracy", maxAimInaccuracy);
    WRITE("Max shot inaccuracy", maxShotInaccuracy);
    WRITE("Min damage", minDamage);
    WRITE("Killshot", killshot);
    WRITE("Between shots", betweenShots);
}

static void to_json(json& j, const Config::Triggerbot& o, const Config::Triggerbot& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Friendly fire", friendlyFire);
    WRITE("Scoped only", scopedOnly);
    WRITE("Ignore flash", ignoreFlash);
    WRITE("Ignore smoke", ignoreSmoke);
    WRITE("Hitgroup", hitgroup);
    WRITE("Shot delay", shotDelay);
    WRITE("Min damage", minDamage);
    WRITE("Killshot", killshot);
    WRITE("Burst Time", burstTime);
}

static void to_json(json& j, const Config::StreamProofESP& o, const Config::StreamProofESP& dummy = {})
{
    WRITE("Toggle Key", toggleKey);
    WRITE("Hold Key", holdKey);
    j["Allies"] = o.allies;
    j["Enemies"] = o.enemies;
    j["Weapons"] = o.weapons;
    j["Projectiles"] = o.projectiles;
    j["Loot Crates"] = o.lootCrates;
    j["Other Entities"] = o.otherEntities;
}

static void to_json(json& j, const ImVec4& o)
{
    j[0] = o.x;
    j[1] = o.y;
    j[2] = o.z;
    j[3] = o.w;
}

static void to_json(json& j, const Config::Style& o)
{
    const Config::Style dummy;
    
    WRITE("Menu colors", menuColors);

    auto& colors = j["Colors"];
    ImGuiStyle& style = ImGui::GetStyle();

    for (int i = 0; i < ImGuiCol_COUNT; i++)
        colors[ImGui::GetStyleColorName(i)] = style.Colors[i];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void removeEmptyObjects(json& j) noexcept
{
    for (auto it = j.begin(); it != j.end();) {
        auto& val = it.value();
        if (val.is_object() || val.is_array())
            removeEmptyObjects(val);
        if (val.empty() && !j.is_array())
            it = j.erase(it);
        else
            ++it;
    }
}

void Config::load(size_t id, bool incremental) noexcept
{
    load(configs[id].c_str(), incremental);
}

/// <summary>
/// Main Config Load Function
/// </summary>
/// <param name="name"></param>
/// <param name="incremental"></param>
/// <returns></returns>
void Config::load(const char8_t* name, bool incremental) noexcept
{
    json j;

    if (std::ifstream in{ path / name }; in.good()) {
        j = json::parse(in, nullptr, false, true);
        if (j.is_discarded())
            return;
    }
    else 
    {
        if (name == u8"default") {
            add(name); 
            if (std::ifstream in{ path / name }; in.good()) {
                j = json::parse(in, nullptr, false, true);
                if (j.is_discarded())
                    return;
            } else
                return;
        } else
            return;
    }

    if (!incremental)
        reset();

    read(j, "Aimbot", aimbot);
    read(j, "Aimbot On key", aimbotOnKey);
    read(j, "Aimbot Key", aimbotKey);
    read(j, "Aimbot Key mode", aimbotKeyMode);

    read(j, "Triggerbot", triggerbot);
    read(j, "Triggerbot Key", triggerbotHoldKey);

    read(j, "Menu key", menuKey);

    read<value_t::object>(j, "Draw Aimbot FOV", drawaimbotFov);
    read<value_t::object>(j, "ESP", streamProofESP);
    read<value_t::object>(j, "Style", style);

    movement->fromJson(j["Movement"]);
    Visuals::fromJson(j["Visuals"]);
    InventoryChanger::fromJson(j["Inventory Changer"]);
    Sound::fromJson(j["Sound"]);
    Misc::fromJson(j["Misc"]);
}

/// <summary>
/// Main Config Save Function
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
void Config::save(size_t id) const noexcept
{
    json j;

    j["Aimbot"] = aimbot;
    j["Aimbot On key"] = aimbotOnKey;
    to_json(j["Aimbot Key"], aimbotKey, {});
    j["Aimbot Key mode"] = aimbotKeyMode;

    j["Triggerbot"] = triggerbot;
    to_json(j["Triggerbot Key"], triggerbotHoldKey, {});
    to_json(j["Menu key"], menuKey, {});

    j["Movement"] = movement->toJson();
    j["Draw Aimbot FOV"] = drawaimbotFov;
    j["ESP"] = streamProofESP;
    j["Sound"] = Sound::toJson();
    j["Visuals"] = Visuals::toJson();
    j["Misc"] = Misc::toJson();
    j["Style"] = style;
    j["Inventory Changer"] = InventoryChanger::toJson();

    removeEmptyObjects(j);

    createConfigDir();
    if (std::ofstream out{ path / configs[id] }; out.good())
        out << std::setw(2) << j;
}

/// <summary>
/// Main Config Reset Function
/// </summary>
/// <returns></returns>
void Config::reset() noexcept
{
    aimbot = { };
    triggerbot = { };
    streamProofESP = { };
    style = { };

    movement->resetConfig();
    Visuals::resetConfig();
    InventoryChanger::resetConfig();
    Sound::resetConfig();
    Misc::resetConfig();
}

void Config::add(const char8_t* name) noexcept
{
    if (*name && std::ranges::find(configs, name) == configs.cend()) {
        configs.emplace_back(name);
        save(configs.size() - 1);
    }
}

void Config::remove(size_t id) noexcept
{
    std::error_code ec;
    std::filesystem::remove(path / configs[id], ec);
    configs.erase(configs.cbegin() + id);
}

void Config::rename(size_t item, std::u8string_view newName) noexcept
{
    std::error_code ec;
    std::filesystem::rename(path / configs[item], path / newName, ec);
    configs[item] = newName;
}

void Config::listConfigs() noexcept
{
    configs.clear();

    std::error_code ec;
    std::transform(std::filesystem::directory_iterator{ path, ec },
                   std::filesystem::directory_iterator{ },
                   std::back_inserter(configs),
                   [](const auto& entry) { return entry.path().filename().u8string(); });
}

void Config::createConfigDir() const noexcept
{
    std::error_code ec; std::filesystem::create_directory(path, ec);
}

void Config::openConfigDir() const noexcept
{
    createConfigDir();
    ShellExecuteW(nullptr, L"open", path.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void Config::scheduleFontLoad(const std::string& name) noexcept
{
    scheduledFonts.push_back(name);
}

static auto getFontData(const std::string& fontName) noexcept
{
    HFONT font = CreateFontA(0, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, fontName.c_str());

    std::unique_ptr<std::byte[]> data;
    DWORD dataSize = GDI_ERROR;

    if (font) {
        HDC hdc = CreateCompatibleDC(nullptr);

        if (hdc) {
            SelectObject(hdc, font);
            dataSize = GetFontData(hdc, 0, 0, nullptr, 0);

            if (dataSize != GDI_ERROR) {
                data = std::make_unique<std::byte[]>(dataSize);
                dataSize = GetFontData(hdc, 0, 0, data.get(), dataSize);

                if (dataSize == GDI_ERROR)
                    data.reset();
            }
            DeleteDC(hdc);
        }
        DeleteObject(font);
    }
    return std::make_pair(std::move(data), dataSize);
}

bool Config::loadScheduledFonts() noexcept
{
    bool result = false;

    for (const auto& fontName : scheduledFonts) {
        if (fontName == "Default") {
            if (fonts.find("Default") == fonts.cend()) {
                ImFontConfig cfg;
                cfg.OversampleH = cfg.OversampleV = 1;
                cfg.PixelSnapH = true;
                cfg.RasterizerMultiply = 1.7f;

                Font newFont;

                cfg.SizePixels = 13.0f;
                newFont.big = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

                cfg.SizePixels = 10.0f;
                newFont.medium = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

                cfg.SizePixels = 8.0f;
                newFont.tiny = ImGui::GetIO().Fonts->AddFontDefault(&cfg);

                fonts.emplace(fontName, newFont);
                result = true;
            }
            continue;
        }
         
        const auto [fontData, fontDataSize] = getFontData(fontName);
        if (fontDataSize == GDI_ERROR)
            continue;

        if (fonts.find(fontName) == fonts.cend()) {
            const auto ranges = Helpers::getFontGlyphRanges();
            ImFontConfig cfg;
            cfg.FontDataOwnedByAtlas = false;
            cfg.RasterizerMultiply = 1.7f;

            Font newFont;
            newFont.tiny = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 8.0f, &cfg, ranges);
            newFont.medium = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 10.0f, &cfg, ranges);
            newFont.big = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontData.get(), fontDataSize, 13.0f, &cfg, ranges);
            fonts.emplace(fontName, newFont);
            result = true;
        }
    }
    scheduledFonts.clear();
    return result;
}