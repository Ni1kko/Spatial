#pragma once

#include "../ImGui/imgui.h"

struct Color4;
struct ColorToggle;
struct ColorToggle3;
struct ColorToggleRounding;
struct ColorToggleThickness;
struct ColorToggleThicknessRounding;

enum ImGuiStyles
{
    Dark = 0,
    Light,
    Classic,
    Spatial,
    Custom
};

namespace ImGuiCustom
{
    void colorPicker(const char* name, float color[3], float* alpha = nullptr, bool* rainbow = nullptr, float* rainbowSpeed = nullptr, bool* enable = nullptr, float* thickness = nullptr, float* rounding = nullptr) noexcept;
    void colorPicker(const char* name, ColorToggle3& colorConfig) noexcept;
    void colorPicker(const char* name, Color4& colorConfig, bool* enable = nullptr, float* thickness = nullptr) noexcept;
    void colorPicker(const char* name, ColorToggle& colorConfig) noexcept;
    void colorPicker(const char* name, ColorToggleRounding& colorConfig) noexcept;
    void colorPicker(const char* name, ColorToggleThickness& colorConfig) noexcept;
    void colorPicker(const char* name, ColorToggleThicknessRounding& colorConfig) noexcept;
    void colorPickerCircle(const char* name, float color[3], float* alpha = nullptr, bool* rainbow = nullptr, float* rainbowSpeed = nullptr, bool* enable = nullptr, float* thickness = nullptr, float* rounding = nullptr) noexcept;
    void colorPickerCircle(const char* name, ColorToggle3& colorConfig) noexcept;
    void colorPickerCircle(const char* name, Color4& colorConfig, bool* enable = nullptr, float* thickness = nullptr) noexcept;
    void colorPickerCircle(const char* name, ColorToggle& colorConfig) noexcept;
    void colorPickerCircle(const char* name, ColorToggleRounding& colorConfig) noexcept;
    void colorPickerCircle(const char* name, ColorToggleThickness& colorConfig) noexcept;
    void colorPickerCircle(const char* name, ColorToggleThicknessRounding& colorConfig) noexcept;
    void arrowButtonDisabled(const char* id, ImGuiDir dir) noexcept;
    void AddCircleImageFilled(ImTextureID user_texture_id, const ImVec2& centre, float radius, ImU32 col, int num_segments);
    void tooltip(const char* text) noexcept;
    void updateColors(ImGuiStyles style = ImGuiStyles::Classic) noexcept;
    IMGUI_API void StyleSpatial(ImGuiStyle* dst = NULL);
    IMGUI_API void StyleCustom(ImGuiStyle* dst = NULL);
    void HelpMarker(const char* desc, bool sameline = true) noexcept;
}

class KeyBind;

namespace ImGui
{
    void progressBarFullWidth(float fraction, float height) noexcept;
    void textUnformattedCentered(const char* text) noexcept;
    void hotkey(const char* label, KeyBind& key, float samelineOffset = 0.0f, const ImVec2& size = { 100.0f, 0.0f }) noexcept;
}
