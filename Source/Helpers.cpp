#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <fstream>
#include <random>
#include <span>
#include <string_view>
#include <unordered_map>
#include <sstream> 
#include <iomanip>
#include <Windows.h>

#include <Encryption/xorstr.hpp>

#include "Menu/imgui/imgui.h"

#include "SDK/GlobalVars.h"
#include "SDK/Engine.h"
#include "SDK/ClientMode.h"
#include "SDK/Steam.h"
#include <SDK/LocalPlayer.h>

#include "ConfigStructs.h"
#include "GameData.h"
#include "Helpers.h"
#include "Memory.h"
#include "Interfaces.h"

static float alphaFactor = 1.0f;

static auto rainbowColor(float time, float speed, float alpha) noexcept
{
    constexpr float pi = std::numbers::pi_v<float>;
    return std::array{ std::sin(speed * time) * 0.5f + 0.5f,
                       std::sin(speed * time + 2 * pi / 3) * 0.5f + 0.5f,
                       std::sin(speed * time + 4 * pi / 3) * 0.5f + 0.5f,
                       alpha };
}

static void toUpper(std::span<wchar_t> str) noexcept
{
    static std::unordered_map<wchar_t, wchar_t> upperCache;

    for (auto& c : str) {
        if (c >= 'a' && c <= 'z') {
            c -= ('a' - 'A');
        }
        else if (c > 127) {
            if (const auto it = upperCache.find(c); it != upperCache.end()) {
                c = it->second;
            }
            else {
                const auto upper = std::towupper(c);
                upperCache.emplace(c, upper);
                c = upper;
            }
        }
    }
}

static bool transformWorldPositionToScreenPosition(const Matrix4x4& matrix, const Vector& worldPosition, ImVec2& screenPosition) noexcept
{
    const auto w = matrix._41 * worldPosition.x + matrix._42 * worldPosition.y + matrix._43 * worldPosition.z + matrix._44;
    if (w < 0.001f)
        return false;

    screenPosition = ImGui::GetIO().DisplaySize / 2.0f;
    screenPosition.x *= 1.0f + (matrix._11 * worldPosition.x + matrix._12 * worldPosition.y + matrix._13 * worldPosition.z + matrix._14) / w;
    screenPosition.y *= 1.0f - (matrix._21 * worldPosition.x + matrix._22 * worldPosition.y + matrix._23 * worldPosition.z + matrix._24) / w;
    return true;
}

static std::string double2string(double value) noexcept
{
    std::ostringstream oss;
    oss << std::setprecision(8) << std::noshowpoint << value;
    return oss.str();
}

static std::string discordURL(std::string discordcode) noexcept
{
    return std::string{ xorstr_("https://discord.gg/") }.append(discordcode);
}

unsigned int Helpers::calculateColor(Color4 color) noexcept
{
    color.color[3] *= alphaFactor;

   // if (!config->ignoreFlashbang)
        color.color[3] *= (255.0f - GameData::local().flashDuration) / 255.0f;
    return ImGui::ColorConvertFloat4ToU32(color.rainbow ? rainbowColor(memory->globalVars->realtime, color.rainbowSpeed, color.color[3]) : color.color);
}

unsigned int Helpers::calculateColor(Color3 color) noexcept
{
    return ImGui::ColorConvertFloat4ToU32(color.rainbow ? rainbowColor(memory->globalVars->realtime, color.rainbowSpeed, 1.0f) : ImVec4{ color.color[0], color.color[1], color.color[2], 1.0f});
}

unsigned int Helpers::calculateColor(int r, int g, int b, int a) noexcept
{
    a -= static_cast<int>(a * GameData::local().flashDuration / 255.0f);
    return IM_COL32(r, g, b, a * alphaFactor);
}

void Helpers::setAlphaFactor(float newAlphaFactor) noexcept
{
    alphaFactor = newAlphaFactor;
}

float Helpers::getAlphaFactor() noexcept
{
    return alphaFactor;
}

void Helpers::convertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB) noexcept
{
    ImGui::ColorConvertHSVtoRGB(h, s, v, outR, outG, outB);
}

void Helpers::healthColor(float fraction, float& outR, float& outG, float& outB) noexcept
{
    constexpr auto greenHue = 1.0f / 3.0f;
    constexpr auto redHue = 0.0f;
    convertHSVtoRGB(std::lerp(redHue, greenHue, fraction), 1.0f, 1.0f, outR, outG, outB);
}

unsigned int Helpers::healthColor(float fraction) noexcept
{
    float r, g, b;
    healthColor(fraction, r, g, b);
    return calculateColor(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 255);
}

void Helpers::shadeVertsHSVColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1) noexcept
{// ImGui::ShadeVertsLinearColorGradientKeepAlpha() modified to do interpolation in HSV
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;

    ImVec4 col0HSV = ImGui::ColorConvertU32ToFloat4(col0);
    ImVec4 col1HSV = ImGui::ColorConvertU32ToFloat4(col1);
    ImGui::ColorConvertRGBtoHSV(col0HSV.x, col0HSV.y, col0HSV.z, col0HSV.x, col0HSV.y, col0HSV.z);
    ImGui::ColorConvertRGBtoHSV(col1HSV.x, col1HSV.y, col1HSV.z, col1HSV.x, col1HSV.y, col1HSV.z);
    ImVec4 colDelta = col1HSV - col0HSV;

    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);

        float h = col0HSV.x + colDelta.x * t;
        float s = col0HSV.y + colDelta.y * t;
        float v = col0HSV.z + colDelta.z * t;

        ImVec4 rgb;
        ImGui::ColorConvertHSVtoRGB(h, s, v, rgb.x, rgb.y, rgb.z);
        vert->col = (ImGui::ColorConvertFloat4ToU32(rgb) & ~IM_COL32_A_MASK) | (vert->col & IM_COL32_A_MASK);
    }
}

ImFont* Helpers::addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
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

ImWchar* Helpers::getFontGlyphRanges() noexcept
{
    static ImVector<ImWchar> ranges;
    if (ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        constexpr ImWchar baseRanges[]{
            0x0100, 0x024F, // Latin Extended-A + Latin Extended-B
            0x0300, 0x03FF, // Combining Diacritical Marks + Greek/Coptic
            0x0600, 0x06FF, // Arabic
            0x0E00, 0x0E7F, // Thai
            0
        };
        builder.AddRanges(baseRanges);
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
        builder.AddText("\u9F8D\u738B\u2122");
        builder.BuildRanges(&ranges);
    }
    return ranges.Data;
}

std::wstring Helpers::toWideString(const std::string& str) noexcept
{
    std::wstring wide(str.length(), L'\0');
    if (const auto newLen = std::mbstowcs(wide.data(), str.c_str(), wide.length()); newLen != static_cast<std::size_t>(-1))
        wide.resize(newLen);
    return wide;
}

std::wstring Helpers::toUpper(std::wstring str) noexcept
{
    ::toUpper(str);
    return str;
}

bool Helpers::decodeVFONT(std::vector<char>& buffer) noexcept
{
    constexpr std::string_view tag = "VFONT1";
    unsigned char magic = 0xA7;

    if (buffer.size() <= tag.length())
        return false;

    const auto tagIndex = buffer.size() - tag.length();
    if (std::memcmp(tag.data(), &buffer[tagIndex], tag.length()))
        return false;

    unsigned char saltBytes = buffer[tagIndex - 1];
    const auto saltIndex = tagIndex - saltBytes;
    --saltBytes;

    for (std::size_t i = 0; i < saltBytes; ++i)
        magic ^= (buffer[saltIndex + i] + 0xA7) % 0x100;

    for (std::size_t i = 0; i < saltIndex; ++i) {
        unsigned char xored = buffer[i] ^ magic;
        magic = (buffer[i] + 0xA7) % 0x100;
        buffer[i] = xored;
    }

    buffer.resize(saltIndex);
    return true;
}

std::vector<char> Helpers::loadBinaryFile(const std::string& path) noexcept
{
    std::vector<char> result;
    std::ifstream in{ path, std::ios::binary };
    if (!in)
        return result;
    in.seekg(0, std::ios_base::end);
    result.resize(static_cast<std::size_t>(in.tellg()));
    in.seekg(0, std::ios_base::beg);
    in.read(result.data(), result.size());
    return result;
}

std::size_t Helpers::calculateVmtLength(const std::uintptr_t* vmt) noexcept
{
    std::size_t length = 0;

    MEMORY_BASIC_INFORMATION memoryInfo;
    while (VirtualQuery(LPCVOID(vmt[length]), &memoryInfo, sizeof(memoryInfo)) && memoryInfo.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))
        ++length;

    return length;
}

bool Helpers::worldToScreen(const Vector& worldPosition, ImVec2& screenPosition) noexcept
{
    return transformWorldPositionToScreenPosition(GameData::toScreenMatrix(), worldPosition, screenPosition);
}

bool Helpers::worldToScreenPixelAligned(const Vector& worldPosition, ImVec2& screenPosition) noexcept
{
    const bool onScreen = transformWorldPositionToScreenPosition(GameData::toScreenMatrix(), worldPosition, screenPosition);
    screenPosition = ImFloor(screenPosition);
    return onScreen;
}

void Helpers::messageBox(std::string_view title, std::string_view msg, const int type) noexcept
{
    const auto flags = [type]() {
        switch (type) {
            case 1: return MB_OK | MB_ICONWARNING;
            case 2: return MB_OK | MB_ICONINFORMATION;
            default: return MB_OK | MB_ICONERROR;
        }
    }();

    MessageBoxA(nullptr, msg.data(), title.data(), flags);
}

Vector Helpers::calculateRelativeAngle(const Vector& source, const Vector& destination) noexcept
{
    Vector delta = destination - source;
    Vector angles{ Helpers::rad2deg(atan2f(-delta.z, std::hypotf(delta.x, delta.y))), Helpers::rad2deg(atan2f(delta.y, delta.x)), 0.f };
    return angles.normalize();;
}

long Helpers::getCurrentTime() noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

const char* Helpers::compileTimestamp() noexcept
{
    return ("[" + std::string{ __TIME__ } + "] " + __DATE__).c_str();
}

const char* Helpers::getColorByte(ColorByte colorByte)  noexcept
{
    switch (colorByte) {
        case ColorByte::Invisible: return std::string{ xorstr_("\n") }.append(xorstr_("\xAD")).append(xorstr_("\xAD")).append(xorstr_("\xAD")).c_str();/*INVISIBLE*/
        case ColorByte::White: return xorstr_("\x01");/*WHITE*/
        case ColorByte::Red: return xorstr_("\x02");/*RED*/
        case ColorByte::Purple: return xorstr_("\x03");/*PURPLE*/
        case ColorByte::Green: return xorstr_("\x04");/*GREEN*/
        case ColorByte::LightGreen: return xorstr_("\x05");/*LIGHT GREEN*/
        case ColorByte::Turquoise: return xorstr_("\x06");/*TURQUOISE*/
        case ColorByte::LightRed: return xorstr_("\x07");/*LIGHT RED*/
        case ColorByte::Grey: return xorstr_("\x08");/*GRAY*/
        case ColorByte::Yellow: return xorstr_("\x09");/*YELLOW*/
        case ColorByte::Orange: return xorstr_("\x10");/*ORANGE*/
        case ColorByte::LightGrey: return xorstr_("\x0A");/*LIGHT GRAY*/
        case ColorByte::LightBlue: return xorstr_("\x0B");/*LIGHT BLUE*/
        case ColorByte::Blue: return xorstr_("\x0D");/*BLUE*/
        case ColorByte::Pink: return xorstr_("\x0E");/*PINK*/
        case ColorByte::DarkOrange: return xorstr_("\x0F");/*DARK ORANGE*/
        default: return xorstr_("\x0C");/*GRAY (PURPLE FOR SPEC)*/
    };
}

void Helpers::excuteSayCommand(const char* message, bool fromConsoleOrKeybind) noexcept
{
    //build command
    std::string command = xorstr_("say ");
    command.append(message);

    //excute command
    interfaces->engine->clientCmdUnrestricted(command.c_str(), fromConsoleOrKeybind);
}

void Helpers::excutePlayCommand(const char* file, bool fromConsoleOrKeybind) noexcept
{
    //build command (audio file must be put in csgo/sound/ directory)
    std::string command = xorstr_("play ");
    command.append(file);

    //excute command
    interfaces->engine->clientCmdUnrestricted(command.c_str(), fromConsoleOrKeybind);
}

void Helpers::writeDebugConsole(const char* message, bool newline) noexcept
{
    //build
    std::string text = message;
    if(newline)
        text.append("\n");
     
    //excute
    memory->debugMsg(text.c_str());
}

void Helpers::writeDebugConsole(const char* message, std::array<std::uint8_t, 4> color, bool newline) noexcept
{
    //build
    std::string text = message;
    if (newline)
        text.append("\n");

    //excute
    memory->conColorMsg(color, text.c_str());
}

void Helpers::writeInGameChat(const char* message, int filter) noexcept
{
    if (interfaces->engine->isInGame())
        memory->clientMode->getHudChat()->printf(filter, message);
    else
        Helpers::writeDebugConsole(message, true);
}

void Helpers::writeInGameChat(const char* message, ColorByte colorByte, int filter) noexcept
{
    std::string text;

    if (interfaces->engine->isInGame()) {
        text = Helpers::getColorByte(ColorByte::GreyPurpleForSpectaor);
        text.append(" ");
        text.append(Helpers::getColorByte(colorByte));
        text.append(message);
    }
    
    Helpers::writeInGameChat(text.c_str());
}

std::string Helpers::getDllNameVersion() noexcept
{
    std::string text{std::string { dll_name } + " " };
    text.append(std::string{ "v" }.append(::double2string(dll_version)));

    if (dll_release > 0)
        text.append(std::string{ "r" }.append(std::to_string(dll_release)));

    return text;
}

void Helpers::showWelcomeMessage() noexcept
{
    std::string str1{ Helpers::getDllNameVersion() };
    str1.append(xorstr_(" P2C"));
    Helpers::writeDebugConsole(str1.c_str(), { 0, 120, 255, 255 });

    std::string str2{ xorstr_("Welcome ") };
    str2.append(interfaces->engine->getSteamAPIContext()->steamFriends->getPersonaName());
    Helpers::writeDebugConsole(str2.c_str(), { 0, 200, 0, 255 });
   
    std::string str3{ xorstr_("Join ") };
    str3.append(dll_name);
    str3.append(xorstr_(" P2C Discord: "));
    Helpers::writeDebugConsole(str3.c_str(), { 201, 120, 40, 255 }, false);
    Helpers::writeDebugConsole(::discordURL(discordcode).c_str(), true);
}

void Helpers::showDiscordUrl(ColorByte colorByte) noexcept
{
    std::string str3{ xorstr_("Join ") };
    str3.append(dll_name);
    str3.append(xorstr_(" P2C Discord: "));
    str3.append(Helpers::getColorByte(ColorByte::White)); 
    str3.append(::discordURL(Helpers::discordcode));
    Helpers::writeInGameChat(str3.c_str(), colorByte);
}

void Helpers::rainbowMenuBorder(float speed) noexcept
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Border] = rainbowColor(memory->globalVars->realtime, speed, 1.0f);
}

bool Helpers::checkForUpdates() noexcept
{ 
    auto version = interfaces->engine->GetProductVersionString();

    if (strcmp(version, game_version) != 0)
    {
        auto title = Helpers::getDllNameVersion();
        auto message = std::string{ "cheat outdated" };

        message.append("\nold cs:go version: ").append(game_version);
        message.append("\ncurrent cs:go version: ").append(version);

        Helpers::messageBox(title.c_str(), message.c_str());
        std::exit(EXIT_FAILURE);
    }
    return false;
}