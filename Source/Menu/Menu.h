#pragma once

#include <memory>
#include <string>
#include <deque>

struct ImFont;

class Menu {
public:
    Menu() noexcept;
    void render(ImDrawList* drawList, ImVec2 displaySize) noexcept;
    bool autoload{ false };
    [[nodiscard]] bool isOpen() const noexcept { return open; }
private:
    bool open = true;
    ImDrawList* drawGUI(ImDrawList* drawList, ImVec2 displaySize) noexcept;

    struct {
        ImFont* normal15px = nullptr;
    } fonts;
};

inline std::unique_ptr<Menu> gui;