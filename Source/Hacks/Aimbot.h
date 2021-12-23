#pragma once

struct UserCmd;
struct Vector;
struct ImDrawList;

namespace Aimbot
{
    Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept;
    void run(UserCmd*) noexcept;
    void autoStop(UserCmd* cmd) noexcept;
    void autoScope(UserCmd* cmd) noexcept;
    void init() noexcept;
    void updateInput() noexcept;
    void drawFov(ImDrawList* drawList) noexcept;
    void drawGUI() noexcept;
}
