#pragma once

#include "../JsonForward.h"

#define Spatial_GLOW() true

namespace Glow
{
    void render() noexcept;
    void clearCustomObjects() noexcept;
    void changeThickness(float width) noexcept;
    void updateInput() noexcept;
    void drawGUI() noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
