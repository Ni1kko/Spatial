#pragma once

#include <memory>
#include <string>
#include <deque>

struct ImFont;

class Menu {
public:
    Menu() noexcept;
    void render() noexcept;
    void handleToggle() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return open; }
    bool autoload{ false };
    
private:
    bool open = true;

    struct {
        ImFont* normal15px = nullptr;
    } fonts;
};

inline std::unique_ptr<Menu> gui;
