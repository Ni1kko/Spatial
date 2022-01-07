#include "../ImGui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ImGui/imgui_internal.h"

#include "../ConfigStructs.h"
#include "../InputUtil.h"

#include "imguiCustom.h"

void ImGuiCustom::AddCircleImageFilled(ImTextureID user_texture_id, const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
    auto window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    window->DrawList->PathClear();

    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;


    const bool push_texture_id = window->DrawList->_TextureIdStack.empty() || user_texture_id != window->DrawList->_TextureIdStack.back();
    if (push_texture_id)
        window->DrawList->PushTextureID(user_texture_id);

    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    const float a_max = IM_PI * 2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    window->DrawList->PathArcTo(centre, radius, 0.0f, a_max, num_segments - 1);
    window->DrawList->PathFillConvex(col);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;

    ImGui::ShadeVertsLinearUV(window->DrawList, vert_start_idx, vert_end_idx, ImVec2(centre.x - radius, centre.y - radius), ImVec2(centre.x + radius, centre.y + radius), ImVec2(0, 0), ImVec2(1, 1), true);

    if (push_texture_id)
        window->DrawList->PopTextureID();


}

void ImGuiCustom::colorPicker(const char* name, float color[3], float* alpha, bool* rainbow, float* rainbowSpeed, bool* enable, float* thickness, float* rounding) noexcept
{
    ImGui::PushID(name);
    if (enable) {
        ImGui::Checkbox("##check", enable);
        ImGui::SameLine(0.0f, 5.0f);
    }
    bool openPopup = ImGui::ColorButton("##btn", ImVec4{ color[0], color[1], color[2], alpha ? *alpha : 1.0f }, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview);
    if (ImGui::BeginDragDropTarget()) {
        if (alpha) {
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F)) {
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
                *alpha = 1.0f;
            }
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                std::copy((float*)payload->Data, (float*)payload->Data + 4, color);
        } else {
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine(0.0f, 5.0f);
    ImGui::TextUnformatted(name);

    if (openPopup)
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup")) {
        if (alpha) {
            float col[]{ color[0], color[1], color[2], *alpha }; 
            ImGui::ColorPicker4("##picker", col, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar);
            color[0] = col[0];
            color[1] = col[1];
            color[2] = col[2];
            *alpha = col[3];
        } else {
            ImGui::ColorPicker3("##picker", color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview);
        }

        if (rainbow || rainbowSpeed || thickness || rounding) {
            ImGui::SameLine();
            if (ImGui::BeginChild("##child", { 150.0f, 0.0f })) {
                if (rainbow)
                    ImGui::Checkbox("Rainbow", rainbow);
                ImGui::PushItemWidth(85.0f);
                if (rainbowSpeed)
                    ImGui::InputFloat("Speed", rainbowSpeed, 0.01f, 0.15f, "%.2f");

                if (rounding || thickness)
                    ImGui::Separator();

                if (rounding) {
                    ImGui::InputFloat("Rounding", rounding, 0.1f, 0.0f, "%.1f");
                    *rounding = std::max(*rounding, 0.0f);
                }

                if (thickness) {
                    ImGui::InputFloat("Thickness", thickness, 0.1f, 0.0f, "%.1f");
                    *thickness = std::max(*thickness, 1.0f);
                }

                ImGui::PopItemWidth();
            }
            ImGui::EndChild();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void ImGuiCustom::colorPicker(const char* name, ColorToggle3& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor3().color.data(), nullptr, &colorConfig.asColor3().rainbow, &colorConfig.asColor3().rainbowSpeed, &colorConfig.enabled);
}

void ImGuiCustom::colorPicker(const char* name, Color4& colorConfig, bool* enable, float* thickness) noexcept
{
    colorPicker(name, colorConfig.color.data(), &colorConfig.color[3], &colorConfig.rainbow, &colorConfig.rainbowSpeed, enable, thickness);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggle& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggleRounding& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled, nullptr, &colorConfig.rounding);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggleThickness& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColorToggle().asColor4().color.data(), &colorConfig.asColorToggle().asColor4().color[3], &colorConfig.asColorToggle().asColor4().rainbow, &colorConfig.asColorToggle().asColor4().rainbowSpeed, &colorConfig.asColorToggle().enabled, &colorConfig.thickness);
}

void ImGuiCustom::colorPicker(const char* name, ColorToggleThicknessRounding& colorConfig) noexcept
{
    colorPicker(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled, &colorConfig.thickness, &colorConfig.rounding);
}

void ImGuiCustom::colorPickerCircle(const char* name, float color[3], float* alpha, bool* rainbow, float* rainbowSpeed, bool* enable, float* thickness, float* rounding) noexcept
{
    ImGui::PushID(name);
    if (enable)
    {
        ImGui::Checkbox("##check", enable);
        ImGui::SameLine(0.0f, 5.0f);
    }
    bool openPopup = ImGui::ColorButton("##btn", { color[0], color[1], color[2], alpha ? *alpha : 1.0f }, ImGuiColorEditFlags_AlphaPreviewHalf);
    if (ImGui::BeginDragDropTarget())
    {
        if (alpha)
        {
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
            {
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
                *alpha = 1.0f;
            }
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                std::copy((float*)payload->Data, (float*)payload->Data + 4, color);
        }
        else
        {
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
            if (const auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                std::copy((float*)payload->Data, (float*)payload->Data + 3, color);
        }

        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine();

    if (std::strncmp(name, "##", 2))
        ImGui::TextUnformatted(name, std::strstr(name, "##"));

    if (openPopup)
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup"))
    {
        if (alpha)
        {
            float col[] = { color[0], color[1], color[2], *alpha };
            ImGui::ColorPicker4("##picker", col, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_Float);
            color[0] = col[0];
            color[1] = col[1];
            color[2] = col[2];
            *alpha = col[3];
        }
        else
        {
            ImGui::ColorPicker3("##picker", color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_Float);
        }

        if (rainbow || rainbowSpeed || thickness || rounding)
        {
            ImGui::SameLine();
            if (ImGui::BeginChild("##child", { 86.0f, 0.0f }))
            {
                if (rainbow)
                {
                    ImGui::Checkbox("Rainbow", rainbow);
                    ImGui::PushItemWidth(85.0f);
                    if (rainbowSpeed)
                        ImGui::InputFloat("Speed", rainbowSpeed, 0.01f, 0.15f, "%.2f");
                }

                if (rounding)
                {
                    ImGui::DragFloat("##rounding", rounding, 0.1f, 0.0f, 100.0f, "Corner %.1f");
                    *rounding = std::max(*rounding, 0.0f);
                }
                if (thickness)
                {
                    ImGui::DragFloat("##thickness", thickness, 0.1f, 1.0f, 10.0f, "Thick %.2f");
                    *thickness = std::max(*thickness, 1.0f);
                }

                ImGui::PopItemWidth();
            }

            ImGui::EndChild();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void ImGuiCustom::colorPickerCircle(const char* name, ColorToggle3& colorConfig) noexcept
{
    colorPickerCircle(name, colorConfig.asColor3().color.data(), nullptr, &colorConfig.asColor3().rainbow, &colorConfig.asColor3().rainbowSpeed, &colorConfig.enabled);
}

void ImGuiCustom::colorPickerCircle(const char* name, Color4& colorConfig, bool* enable, float* thickness) noexcept
{
    colorPickerCircle(name, colorConfig.color.data(), &colorConfig.color[3], &colorConfig.rainbow, &colorConfig.rainbowSpeed, enable, thickness);
}

void ImGuiCustom::colorPickerCircle(const char* name, ColorToggle& colorConfig) noexcept
{
    colorPickerCircle(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled);
}

void ImGuiCustom::colorPickerCircle(const char* name, ColorToggleRounding& colorConfig) noexcept
{
    colorPickerCircle(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled, nullptr, &colorConfig.rounding);
}

void ImGuiCustom::colorPickerCircle(const char* name, ColorToggleThickness& colorConfig) noexcept
{
    colorPickerCircle(name, colorConfig.asColorToggle().asColor4().color.data(), &colorConfig.asColorToggle().asColor4().color[3], &colorConfig.asColorToggle().asColor4().rainbow, &colorConfig.asColorToggle().asColor4().rainbowSpeed, &colorConfig.asColorToggle().enabled, &colorConfig.thickness);
}

void ImGuiCustom::colorPickerCircle(const char* name, ColorToggleThicknessRounding& colorConfig) noexcept
{
    colorPickerCircle(name, colorConfig.asColor4().color.data(), &colorConfig.asColor4().color[3], &colorConfig.asColor4().rainbow, &colorConfig.asColor4().rainbowSpeed, &colorConfig.enabled, &colorConfig.thickness, &colorConfig.rounding);
}

void ImGuiCustom::arrowButtonDisabled(const char* id, ImGuiDir dir) noexcept
{
    float sz = ImGui::GetFrameHeight();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    ImGui::ArrowButtonEx(id, dir, ImVec2{ sz, sz }, 16384);
    ImGui::PopStyleVar();
}

void ImGuiCustom::tooltip(const char* text) noexcept
{
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(text);
}

void ImGuiCustom::StyleSpatial(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    auto mainrounding = 12.0f;

    style->WindowRounding = mainrounding;
    style->FrameRounding = mainrounding;
    style->ChildRounding = mainrounding;
    style->FrameRounding = mainrounding;
    style->PopupRounding = mainrounding;
    style->ScrollbarRounding = mainrounding;
    style->GrabRounding = mainrounding;
    style->TabRounding = mainrounding / 2;

    style->WindowBorderSize = 1.0f;
    style->ChildBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;
    style->FrameBorderSize = 0.0f;
    style->TabBorderSize = 1.0f;

    style->WindowPadding = ImVec2{ 20.0f, 10.0f };
    style->FramePadding = ImVec2{ 4.0f, 4.0f };
    style->CellPadding = ImVec2{ 4.0f, 2.0f };
    style->ItemSpacing = ImVec2{ 10.0f, 4.0f };
    style->ItemInnerSpacing = ImVec2{ 10.0f, 4.0f };

    style->WindowTitleAlign = ImVec2{ 0.5f, 0.5f };
    style->WindowMenuButtonPosition = ImGuiDir_None;
    style->ColorButtonPosition = ImGuiDir_Left;

    style->IndentSpacing = 25.0f;
    style->GrabMinSize = 6.0f;
    style->ScrollbarSize = 10.0f;

    colors[ImGuiCol_Text] = ImVec4(0.76f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(1.00f, 0.00f, 0.00f, 0.45f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 0.17f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 0.00f, 0.00f, 0.45f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.00f, 0.00f, 0.45f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.00f, 0.00f, 0.45f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_Header] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_Separator] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.80f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.53f, 0.11f, 0.11f, 0.45f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.00f, 0.00f, 0.00f, 0.45f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.30f, 0.30f, 0.45f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.45f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGuiCustom::StyleCustom(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

     
}

static void StyleDefault(ImGuiStyle* style)
{ 
    style->ScrollbarSize = 9.0f;
    style->WindowRounding = 2.0f;
    style->FrameRounding = 4.0f;
}

static void StyleReset()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    //Default rounding
    style->WindowRounding = 0.0f;
    style->FrameRounding = 0.0f;
    style->ChildRounding = 0.0f;
    style->FrameRounding = 0.0f;
    style->PopupRounding = 0.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabRounding = 0.0f;
    style->TabRounding = 0.0f;

    //Default borders
    style->WindowBorderSize = 1.0f;
    style->ChildBorderSize = 0.0f;
    style->PopupBorderSize = 0.0f;
    style->FrameBorderSize = 0.0f;
    style->TabBorderSize = 0.0f;

    //Default padding
    style->WindowPadding = ImVec2{ 8.0f, 8.0f };
    style->FramePadding = ImVec2{ 4.0f, 3.0f };
    style->CellPadding = ImVec2{ 4.0f, 2.0f };

    //Default spacing
    style->ItemSpacing = ImVec2{ 8.0f, 4.0f };
    style->ItemInnerSpacing = ImVec2{ 4.0f, 4.0f };
    style->IndentSpacing = 21.0f;

    //Default positions
    style->WindowMenuButtonPosition = ImGuiDir_Left;
    style->ColorButtonPosition = ImGuiDir_Right;

    //Default others
    style->WindowTitleAlign = ImVec2{ 0.00f, 0.05f };
    style->GrabMinSize = 10.0f;
    style->ScrollbarSize = 14.0f;

    StyleDefault(style);
}

void ImGuiCustom::updateColors(ImGuiStyles style) noexcept
{
    StyleReset();
    switch (style) {
        case ImGuiStyles::Dark: ImGui::StyleColorsDark(); break;
        case ImGuiStyles::Light: ImGui::StyleColorsLight(); break;
        case ImGuiStyles::Classic: ImGui::StyleColorsClassic(); break;
        case ImGuiStyles::Spatial: ImGuiCustom::StyleSpatial(); break;
        default: ImGuiCustom::StyleCustom(); break;
    }
}

void ImGui::progressBarFullWidth(float fraction, float height) noexcept
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2{ -1, 0 }, CalcItemWidth(), height + style.FramePadding.y * 2.0f);
    ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    fraction = ImSaturate(fraction);
    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));

    if (fraction == 0.0f)
        return;

    const ImVec2 p0{ ImLerp(bb.Min.x, bb.Max.x, 0.0f), bb.Min.y };
    const ImVec2 p1{ ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y };

    const float x0 = ImMax(p0.x, bb.Min.x);
    const float x1 = ImMin(p1.x, bb.Max.x);

    window->DrawList->PathLineTo({ x0, p1.y });
    window->DrawList->PathLineTo({ x0, p0.y });
    window->DrawList->PathLineTo({ x1, p0.y });
    window->DrawList->PathLineTo({ x1, p1.y });
    window->DrawList->PathFillConvex(GetColorU32(ImGuiCol_PlotHistogram));
}

void ImGui::textUnformattedCentered(const char* text) noexcept
{
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) / 2.0f);
    ImGui::TextUnformatted(text);
}

void ImGui::hotkey(const char* label, KeyBind& key, float samelineOffset, const ImVec2& size) noexcept
{
    const auto id = GetID(label);
    PushID(label);

    TextUnformatted(label);
    SameLine(samelineOffset);

    if (GetActiveID() == id) {
        PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
        Button("...", size);
        PopStyleColor();

        GetCurrentContext()->ActiveIdAllowOverlap = true;
        if ((!IsItemHovered() && GetIO().MouseClicked[0]) || key.setToPressedKey())
            ClearActiveID();
    } else if (Button(key.toString(), size)) {
        SetActiveID(id, GetCurrentWindow());
    }

    PopID();
}

void ImGuiCustom:: HelpMarker(const char* desc, bool sameline) noexcept
{
    if(sameline) ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}