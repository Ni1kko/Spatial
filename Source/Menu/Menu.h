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

    void renderAimbotWindow() noexcept;
    void renderAntiAimWindow() noexcept;
    void renderTriggerbotWindow() noexcept;
    void renderBackTrackWindow() noexcept;
    void renderMovementWindow() noexcept;
    void renderNetworkWindow() noexcept;
    void renderGlowWindow() noexcept;
    void renderChamsWindow() noexcept;
    void renderESPWindow() noexcept;
    void renderVisualsWindow() noexcept;
    void renderInventoryWindow() noexcept;
    void renderSoundWindow() noexcept;
    void renderMiscWindow() noexcept;
    void renderTrollWindow() noexcept;
    void renderStyleWindow() noexcept;

    struct {
        bool aimbot = false;
        bool triggerbot = false;
        bool chams = false;
        bool sound = false; 
        bool config = false;
    } window;

    struct {
        ImFont* normal15px = nullptr;
    } fonts;
};

inline std::unique_ptr<Menu> gui;
