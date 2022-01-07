#pragma once

#include <concepts>
#include <mutex>
#include <numbers>
#include <random>
#include <string>
#include <vector>

#include "Menu/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Menu/imgui/imgui_internal.h"

#include "SDK/WeaponId.h"

struct Color3;
struct Color4;
struct Vector;

enum ColorByte
{  
    Invisible = 0,
    White,
    Red,
    Purple,
    Green,
    LightGreen,
    Turquoise,
    LightRed,
    Grey,
    LightGrey,
    LightBlue,
    Yellow,
    Orange,
    GreyPurpleForSpectaor,
    Blue,
    Pink,
    DarkOrange
};

constexpr Vector operator-(float sub, const std::array<float, 3>& a) noexcept { return Vector{ sub - a[0], sub - a[1], sub - a[2] }; }

namespace Helpers
{
    const auto dll_name = "Spatial";
    const auto dll_version = 1.7;
    const auto dll_release = 2;
    const auto game_version = "1.38.1.3";
    const auto discordcode = "GyzJf5eNPe";
    const auto maintainer_discordname = "Ni1kko#1652";
    unsigned int calculateColor(Color4 color) noexcept;
    unsigned int calculateColor(Color3 color) noexcept;
    unsigned int calculateColor(int r, int g, int b, int a) noexcept;
    void setAlphaFactor(float newAlphaFactor) noexcept;
    float getAlphaFactor() noexcept;
    void convertHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB) noexcept;
    void healthColor(float fraction, float& outR, float& outG, float& outB) noexcept;
    unsigned int healthColor(float fraction) noexcept;
    void shadeVertsHSVColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1) noexcept;
    ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept;
    const char* compileTimestamp() noexcept;
    void writeDebugConsole(const char* message, bool newline) noexcept;
    void writeDebugConsole(const char* message, std::array<std::uint8_t, 4> color, bool newline = true) noexcept;
    void writeInGameChat(const char* message, int filter = 0) noexcept;
    void writeInGameChat(const char* message, ColorByte colorByte, int filter = 0) noexcept;
    std::string getDllNameVersion() noexcept;
    void showWelcomeMessage() noexcept;
    void showDiscordUrl(ColorByte colorByte = ColorByte::Green) noexcept;
    void rainbowMenuBorder(float speed = 2.f) noexcept;
    bool checkForUpdates() noexcept;

    constexpr auto units2meters(float units) noexcept
    {
        return units * 0.0254f;
    }

    ImWchar* getFontGlyphRanges() noexcept;

    constexpr std::uint8_t utf8SeqLen(char firstByte) noexcept
    {
        return (firstByte & 0x80) == 0x00 ? 1 :
               (firstByte & 0xE0) == 0xC0 ? 2 :
               (firstByte & 0xF0) == 0xE0 ? 3 :
               (firstByte & 0xF8) == 0xF0 ? 4 :
               0;
    }

    std::wstring toWideString(const std::string& str) noexcept;
    std::wstring toUpper(std::wstring str) noexcept;

    bool decodeVFONT(std::vector<char>& buffer) noexcept;
    std::vector<char> loadBinaryFile(const std::string& path) noexcept;

    constexpr auto deg2rad(float degrees) noexcept { return degrees * (std::numbers::pi_v<float> / 180.0f); }
    constexpr auto rad2deg(float radians) noexcept { return radians * (180.0f / std::numbers::pi_v<float>); }

    [[nodiscard]] std::size_t calculateVmtLength(const std::uintptr_t* vmt) noexcept;

    constexpr auto isKnife(WeaponId id) noexcept
    {
        return (id >= WeaponId::Bayonet && id <= WeaponId::SkeletonKnife) || id == WeaponId::KnifeT || id == WeaponId::Knife;
    }

    constexpr auto isSouvenirToken(WeaponId id) noexcept
    {
        switch (id) {
        case WeaponId::Berlin2019SouvenirToken:
        case WeaponId::Stockholm2021SouvenirToken:
            return true;
        default:
            return false;
        }
    }
    void messageBox(std::string_view title, std::string_view msg, const int type = 0) noexcept;
    Vector calculateRelativeAngle(const Vector& source, const Vector& destination) noexcept;
    const char* getColorByte(ColorByte colorByte) noexcept;
    void excuteSayCommand(const char* message, bool fromConsoleOrKeybind = false) noexcept;
    void excutePlayCommand(const char* file, bool fromConsoleOrKeybind = false) noexcept;
    long getCurrentTime() noexcept;
    bool worldToScreen(const Vector& worldPosition, ImVec2& screenPosition) noexcept;
    bool worldToScreenPixelAligned(const Vector& worldPosition, ImVec2& screenPosition) noexcept;
    std::pair<std::array<ImVec2, 8>, std::size_t> convexHull(std::array<ImVec2, 8> points) noexcept;

    [[nodiscard]] constexpr auto isMP5LabRats(WeaponId weaponID, int paintKit) noexcept
    {
        return weaponID == WeaponId::Mp5sd && paintKit == 800;
    }

    class RandomGenerator {
    public:
        template <std::integral T>
        [[nodiscard]] static T random(T min, T max) noexcept
        {
            std::scoped_lock lock{ mutex };
            return std::uniform_int_distribution{ min, max }(gen);
        }

        template <std::floating_point T>
        [[nodiscard]] static T random(T min, T max) noexcept
        {
            std::scoped_lock lock{ mutex };
            return std::uniform_real_distribution{ min, max }(gen);
        }

        template <typename T>
        [[nodiscard]] static std::enable_if_t<std::is_enum_v<T>, T> random(T min, T max) noexcept
        {
            return static_cast<T>(random(static_cast<std::underlying_type_t<T>>(min), static_cast<std::underlying_type_t<T>>(max)));
        }
    private:
        inline static std::mt19937 gen{ std::random_device{}() };
        inline static std::mutex mutex;
    };

    template <typename T>
    [[nodiscard]] T random(T min, T max) noexcept
    {
        return RandomGenerator::random(min, max);
    }
}
