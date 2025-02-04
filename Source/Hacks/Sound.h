#pragma once

#include <string_view>

#include "../JsonForward.h"

#define Spatial_SOUND() true

class GameEvent;

namespace Sound
{
    //Funcions
    void modulateSound(std::string_view name, int entityIndex, float& volume) noexcept;
    void playHitSound(GameEvent& event) noexcept;
    void playKillSound(GameEvent& event) noexcept;
    void drawGUI() noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}