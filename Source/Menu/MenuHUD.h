#pragma once

#include <memory>
#include <string>
#include <deque>

namespace MenuHUD 
{
    ImDrawList* drawHUD(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_BottomCenter(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_Leftbottom(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_LeftMiddle(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_LeftTop(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_TopCenter(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_RightTop(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_RightMiddle(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    ImDrawList* drawHUDElement_Rightbottom(ImDrawList* drawList, ImVec2 displaySize) noexcept;
};