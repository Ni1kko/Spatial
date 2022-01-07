#pragma once

#include <memory>
#include <string>
#include <deque>

#define xorstr_(str) str

struct ImFont;

class Menu {
public:
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool autoload{ false };

    Menu() noexcept;
    void render() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return open; }
    [[nodiscard]] bool isOpen2() const noexcept { return open2; }
    
private:

    struct {
        ImFont* normal15px = nullptr;
    } fonts;

    struct Tab {
        const char* name;
        bool open{ false };
        ImVec2 size{ -1,-1 };
    };

    struct Tabs {
        Tab aim;
        Tab antiaim;
        Tab triggerbot;
        Tab backtrack;
        Tab movement;
        Tab network;
        Tab glow;
        Tab chams;
        Tab esp;
        Tab visuals;
        Tab inventory;
        Tab sound;
        Tab misc;
        Tab troll;
        Tab config;
    };

    struct Window {
        const char* name;
        ImGuiWindowFlags flags;
        bool open{ false };
        ImVec2 size{ -1,-1 };
        Tabs tabs;
    } window;

    bool open = true;
    bool open2 = true;

    void renderTest() noexcept;
    void renderDemo() noexcept;
    void tabShow(Tab* tab) noexcept;
};

inline std::unique_ptr<Menu> menu;