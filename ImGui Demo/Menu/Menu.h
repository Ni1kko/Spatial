#pragma once

#include <memory>
#include <string>
#include <deque>

#define xorstr_(str) str

struct ImFont;

class Menu {
public:
    Menu() noexcept;
    void render() noexcept;
    bool autoload{ false };
    [[nodiscard]] bool isOpen() const noexcept { return open; }
    [[nodiscard]] bool isOpen2() const noexcept { return open2; }
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
private:
    void renderTest() noexcept;
    void renderDemo() noexcept;
    bool open = true;
    bool open2 = true;
    struct {
        ImFont* normal15px = nullptr;
    } fonts;
};

inline std::unique_ptr<Menu> menu;