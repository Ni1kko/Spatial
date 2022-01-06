#include <Menu/imgui/imgui.h>
#include "Menu/imgui/imgui_impl_dx9.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <Menu/imgui/imgui_internal.h>

#include <Menu/imguiCustom.h>
#include <ConfigStructs.h>
#include <InputUtil.h>

struct InputTextCallback_UserDataU8
{
    std::u8string* Str;
    ImGuiInputTextCallback  ChainCallback;
    void* ChainCallbackUserData;
};

static int InputTextCallbackU8(ImGuiInputTextCallbackData* data)
{
    InputTextCallback_UserDataU8* user_data = (InputTextCallback_UserDataU8*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::u8string* str = user_data->Str;
        IM_ASSERT((const char8_t*)data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

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

void ImGuiCustom::arrowButtonDisabled(const char* id, ImGuiDir dir) noexcept
{
    float sz = ImGui::GetFrameHeight();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    ImGui::ArrowButtonEx(id, dir, ImVec2{ sz, sz });
    ImGui::PopStyleVar();
}


void ImGuiCustom::tooltip(const char* text) noexcept
{
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(text);
}

IMGUI_API void ImGuiCustom::StyleSpatial(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    auto mainrounding = 4.0f;
    style->FrameRounding = mainrounding;
    style->ChildRounding = mainrounding;
    style->WindowRounding = mainrounding;
    style->PopupRounding = mainrounding;
    style->TabRounding = mainrounding;
    style->ScrollbarRounding = mainrounding / 2;
    style->GrabRounding = mainrounding / 2;

    style->FrameBorderSize = 0.8f;
    style->ScrollbarSize = 6.0f;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 0.00f, 0.40f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(1.00f, 0.00f, 0.40f, 0.90f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.00f, 0.40f, 0.50f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.21f, 0.21f, 0.21f , 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.70f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.00f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.00f, 0.40f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.51f, 0.51f, 0.51f, 0.90f);
    colors[ImGuiCol_Separator] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 0.90f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.51f, 0.51f, 0.51f, 0.90f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.00f, 0.40f, 1.00f);
}

IMGUI_API void ImGuiCustom::StyleCustom(ImGuiStyle* dst)
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
     
    style->FrameBorderSize = 0.0f;

    style->FrameRounding = 0.0f;
    style->ChildRounding = 0.0f;
    style->WindowRounding = 0.0f;
    style->PopupRounding = 0.0f;
    style->TabRounding = 0.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabRounding = 0.0f;

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

IMGUI_API bool ImGui::InputTextWithHint(const char* label, const char* hint, std::u8string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallback_UserDataU8 cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallbackU8, &cb_user_data);
}