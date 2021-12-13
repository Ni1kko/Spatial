#pragma once

#include <memory>
#include <string>
#include <deque>

struct ImFont;

struct alerts_struct {
    alerts_struct(ImGuiCol c, ImGuiCol c1, float time, std::string title, std::string msg) : color(c), background_color(c1), m_time(time), m_title(msg), m_msg(title) { }
    ImGuiCol color;
    ImGuiCol background_color;
    std::string m_title;
    std::string m_msg;
    float m_time;
};


class GUI {
public:
    GUI() noexcept;
    void render() noexcept;
    void handleToggle() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return open; }
    bool autoload{ false };
    
private:
    bool open = true;
    bool isAlertOpen{ false };
    bool isAlertPending{ false };
    float alertsDuration = 5.f;
    std::deque<alerts_struct> alerts_notification;
    

    void updateColors() const noexcept;
    void showAlert(bool, const std::string, const std::string) noexcept;
    void clearAlerts() noexcept;


    void cbox_colorpicker(const std::string&, bool*, float*) noexcept;
    void change_keybind(int&) noexcept;
    void showTooltip(const char* desc) noexcept;
    void renderAlerts() noexcept;
    void renderNodes() noexcept;

    void renderAimbotWindow() noexcept;
    void renderAntiAimWindow() noexcept;
    void renderTriggerbotWindow() noexcept;
    void renderBackTrackWindow() noexcept;
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
    void renderConfigWindow() noexcept;

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

    float timeToNextConfigRefresh = 0.1f;
};

inline std::unique_ptr<GUI> gui;
