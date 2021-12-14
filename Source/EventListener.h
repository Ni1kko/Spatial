#pragma once

#include "SDK/GameEvent.h"

namespace EventListener
{
    void init() noexcept;
    void remove() noexcept;
    void OnKilledEvent(GameEvent &event) noexcept;
    void OnDamgeEvent(GameEvent &event) noexcept;
}
