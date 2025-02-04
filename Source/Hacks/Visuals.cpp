#include <array>
#include <cstring>
#include <string_view>
#include <utility>
#include <vector>
#include <iomanip>

#include "../Menu/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../Menu/imgui/imgui_internal.h"

#include <Encryption/xorstr.hpp>

#include "../ConfigStructs.h"
#include "../GameData.h"
#include "../Helpers.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../Menu/imguiCustom.h"
#include "Visuals.h"

#include "../SDK/ConVar.h"
#include "../SDK/Cvar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Input.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h" 
#include "../SDK/ViewRenderBeams.h"
#include <Hacks/Movement.h>

#undef xor
#define DRAW_SCREEN_EFFECT(material) \
{ \
    const auto drawFunction = memory->drawScreenEffectMaterial; \
    int w, h; \
    interfaces->engine->getScreenSize(w, h); \
    __asm { \
        __asm push h \
        __asm push w \
        __asm push 0 \
        __asm xor edx, edx \
        __asm mov ecx, material \
        __asm call drawFunction \
        __asm add esp, 12 \
    } \
}

struct BulletTracers : ColorToggle {
    BulletTracers() : ColorToggle{ 0.0f, 0.75f, 1.0f, 1.0f } {}
};

struct VisualsConfig {
    bool disablePostProcessing{ false };
    bool inverseRagdollGravity{ false };
    bool noFog{ false };
    bool no3dSky{ false };
    bool noAimPunch{ false };
    bool noViewPunch{ false };
    bool noHands{ false };
    bool noSleeves{ false };
    bool noWeapons{ false };
    bool noSmoke{ false };
    bool noBlur{ false };
    bool noScopeOverlay{ false };
    bool noGrass{ false };
    bool noShadows{ false };
    bool wireframeSmoke{ false };
    ColorToggle autoPeekToggle{ 1.0f, 1.0f, 1.0f, 1.0f };
    bool zoom{ false };
    KeyBindToggle zoomKey;
    int ragdollForce{ 1 };
    bool fullBright{ false };
    bool thirdperson{ false };
    KeyBindToggle thirdpersonKey;
    int thirdpersonDistance{ 0 };
    int viewmodelFov{ 0 };
    bool customPos{ false };
    float x{ 0.0 };
    float y{ 0.0 };
    float z{ 0.0 };
    int fov{ 0 };
    int farZ{ 0 };
    int flashReduction{ 0 };
    float brightness{ 0.0f };
    int skybox{ 0 };
    ColorToggle3 world;
    ColorToggle3 sky;
    bool deagleSpinner{ false };
    int screenEffect{ 0 };
    int hitEffect{ 0 };
    float hitEffectTime{ 0.6f };
    int hitMarker{ 0 };
    float hitMarkerTime{ 0.6f };
    BulletTracers bulletTracers;
    ColorToggle molotovHull{ 1.0f, 0.27f, 0.0f, 0.3f };
    float aspectratio{ 0 };

    struct ColorCorrection {
        bool enabled = false;
        float blue = 0.0f;
        float red = 0.0f;
        float mono = 0.0f;
        float saturation = 0.0f;
        float ghost = 0.0f;
        float green = 0.0f;
        float yellow = 0.0f;
    } colorCorrection;

    struct SmokeTimer {
        bool enabled = false;
        Color4 backgroundColor{ 1.0f, 1.0f, 1.0f, 0.5f };
        Color4 timerColor{ 0.0f, 0.0f, 1.0f, 1.0f };
        float timerThickness{ 1.f };
        Color4 textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    } smokeTimer;

} visualsConfig;

static int lastRagdollForce{ 0 };
static bool lastFullBrightState{ false };

static bool worldToScreen(const Vector& in, ImVec2& out) noexcept
{
    const auto& matrix = GameData::toScreenMatrix();

    const auto w = matrix._41 * in.x + matrix._42 * in.y + matrix._43 * in.z + matrix._44;
    if (w < 0.001f)
        return false;

    out = ImGui::GetIO().DisplaySize / 2.0f;
    out.x *= 1.0f + (matrix._11 * in.x + matrix._12 * in.y + matrix._13 * in.z + matrix._14) / w;
    out.y *= 1.0f - (matrix._21 * in.x + matrix._22 * in.y + matrix._23 * in.z + matrix._24) / w;
    out = ImFloor(out);
    return true;
}


bool Visuals::isThirdpersonOn() noexcept
{
    return visualsConfig.thirdperson;
}

bool Visuals::isZoomOn() noexcept
{
    return visualsConfig.zoom;
}

bool Visuals::isSmokeWireframe() noexcept
{
    return visualsConfig.wireframeSmoke;
}

bool Visuals::isDeagleSpinnerOn() noexcept
{
    return visualsConfig.deagleSpinner;
}

bool Visuals::shouldRemoveFog() noexcept
{
    return visualsConfig.noFog;
}

bool Visuals::shouldRemoveScopeOverlay() noexcept
{
    return visualsConfig.noScopeOverlay;
}

bool Visuals::shouldRemoveSmoke() noexcept
{
    return visualsConfig.noSmoke;
}

float Visuals::viewModelFov() noexcept
{
    return static_cast<float>(visualsConfig.viewmodelFov);
}

float Visuals::fov() noexcept
{
    return static_cast<float>(visualsConfig.fov);
}

float Visuals::farZ() noexcept
{
    return static_cast<float>(visualsConfig.farZ);
}

void Visuals::performColorCorrection() noexcept
{
    if (const auto& cfg = visualsConfig.colorCorrection; cfg.enabled) {
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x49C) = cfg.blue;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4A4) = cfg.red;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4AC) = cfg.mono;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4B4) = cfg.saturation;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4C4) = cfg.ghost;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4CC) = cfg.green;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4D4) = cfg.yellow;
    }
}

void Visuals::inverseRagdollGravity() noexcept
{
    static auto ragdollGravity = interfaces->cvar->findVar("cl_ragdoll_gravity");
    ragdollGravity->setValue(visualsConfig.inverseRagdollGravity ? -600 : 600);
}

void Visuals::colorWorld() noexcept
{
    if (!visualsConfig.world.enabled && !visualsConfig.sky.enabled)
        return;

    for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial(); h = interfaces->materialSystem->nextMaterial(h)) {
        const auto mat = interfaces->materialSystem->getMaterial(h);

        if (!mat || !mat->isPrecached())
            continue;

        const std::string_view textureGroup = mat->getTextureGroupName();

        if (visualsConfig.world.enabled && (textureGroup.starts_with("World") || textureGroup.starts_with("StaticProp"))) {
            if (visualsConfig.world.asColor3().rainbow)
                mat->colorModulate(rainbowColor(visualsConfig.world.asColor3().rainbowSpeed));
            else
                mat->colorModulate(visualsConfig.world.asColor3().color);
        } else if (visualsConfig.sky.enabled && textureGroup.starts_with("SkyBox")) {
            if (visualsConfig.sky.asColor3().rainbow)
                mat->colorModulate(rainbowColor(visualsConfig.sky.asColor3().rainbowSpeed));
            else
                mat->colorModulate(visualsConfig.sky.asColor3().color);
        }
    }
}

void Visuals::modifySmoke(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr std::array smokeMaterials{
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade"
    };

    for (const auto mat : smokeMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && visualsConfig.noSmoke);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == FrameStage::RENDER_START && visualsConfig.wireframeSmoke);
    }
}

void Visuals::thirdperson() noexcept
{
    if (!visualsConfig.thirdperson)
        return;

    memory->input->isCameraInThirdPerson = (!visualsConfig.thirdpersonKey.isSet() || visualsConfig.thirdpersonKey.isToggled()) && localPlayer && localPlayer->isAlive();
    memory->input->cameraOffset.z = static_cast<float>(visualsConfig.thirdpersonDistance); 
}

void Visuals::removeVisualRecoil(FrameStage stage) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static Vector aimPunch;
    static Vector viewPunch;

    if (stage == FrameStage::RENDER_START) {
        aimPunch = localPlayer->aimPunchAngle();
        viewPunch = localPlayer->viewPunchAngle();

        if (visualsConfig.noAimPunch)
            localPlayer->aimPunchAngle() = Vector{ };

        if (visualsConfig.noViewPunch)
            localPlayer->viewPunchAngle() = Vector{ };

    } else if (stage == FrameStage::RENDER_END) {
        localPlayer->aimPunchAngle() = aimPunch;
        localPlayer->viewPunchAngle() = viewPunch;
    }
}

void Visuals::removeBlur(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static auto blur = interfaces->materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && visualsConfig.noBlur);
}

void Visuals::updateBrightness() noexcept
{
    static auto brightness = interfaces->cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(visualsConfig.brightness);
}

void Visuals::removeGrass(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr auto getGrassMaterialName = []() noexcept -> const char* {
        switch (fnv::hashRuntime(interfaces->engine->getLevelName())) {
        case fnv::hash("dz_blacksite"): return "detail/detailsprites_survival";
        case fnv::hash("dz_sirocco"): return "detail/dust_massive_detail_sprites";
        case fnv::hash("coop_autumn"): return "detail/autumn_detail_sprites";
        case fnv::hash("dz_frostbite"): return "ski/detail/detailsprites_overgrown_ski";
        // dz_junglety has been removed in 7/23/2020 patch
        // case fnv::hash("dz_junglety"): return "detail/tropical_grass";
        default: return nullptr;
        }
    };

    if (const auto grassMaterialName = getGrassMaterialName())
        interfaces->materialSystem->findMaterial(grassMaterialName)->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && visualsConfig.noGrass);
}

void Visuals::remove3dSky() noexcept
{
    static auto sky = interfaces->cvar->findVar("r_3dsky");
    sky->setValue(!visualsConfig.no3dSky);
}

void Visuals::removeShadows() noexcept
{
    static auto shadows = interfaces->cvar->findVar("cl_csm_enabled");
    shadows->setValue(!visualsConfig.noShadows);
}

void Visuals::applyZoom(FrameStage stage) noexcept
{
    if (visualsConfig.zoom && localPlayer) {
        if (stage == FrameStage::RENDER_START && (localPlayer->fov() == 90 || localPlayer->fovStart() == 90)) {
            if (visualsConfig.zoomKey.isToggled()) {
                localPlayer->fov() = 40;
                localPlayer->fovStart() = 40;
            }
        }
    }
}

void Visuals::applyScreenEffects() noexcept
{
    if (!visualsConfig.screenEffect)
        return;

    const auto material = interfaces->materialSystem->findMaterial([] {
        constexpr std::array effects{
            "effects/dronecam",
            "effects/underwater_overlay",
            "effects/healthboost",
            "effects/dangerzone_screen"
        };

        if (visualsConfig.screenEffect <= 2 || static_cast<std::size_t>(visualsConfig.screenEffect - 2) >= effects.size())
            return effects[0];
        return effects[visualsConfig.screenEffect - 2];
    }());

    if (visualsConfig.screenEffect == 1)
        material->findVar("$c0_x")->setValue(0.0f);
    else if (visualsConfig.screenEffect == 2)
        material->findVar("$c0_x")->setValue(0.1f);
    else if (visualsConfig.screenEffect >= 4)
        material->findVar("$c0_x")->setValue(1.0f);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::hitEffect(GameEvent* event) noexcept
{
    if (visualsConfig.hitEffect && localPlayer) {
        static float lastHitTime = 0.0f;

        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
            lastHitTime = memory->globalVars->realtime;
            return;
        }

        if (lastHitTime + visualsConfig.hitEffectTime >= memory->globalVars->realtime) {
            constexpr auto getEffectMaterial = [] {
                static constexpr const char* effects[]{
                "effects/dronecam",
                "effects/underwater_overlay",
                "effects/healthboost",
                "effects/dangerzone_screen"
                };

                if (visualsConfig.hitEffect <= 2)
                    return effects[0];
                return effects[visualsConfig.hitEffect - 2];
            };

           
            auto material = interfaces->materialSystem->findMaterial(getEffectMaterial());
            if (visualsConfig.hitEffect == 1)
                material->findVar("$c0_x")->setValue(0.0f);
            else if (visualsConfig.hitEffect == 2)
                material->findVar("$c0_x")->setValue(0.1f);
            else if (visualsConfig.hitEffect >= 4)
                material->findVar("$c0_x")->setValue(1.0f);

            DRAW_SCREEN_EFFECT(material)
        }
    }
}

void Visuals::hitMarker(GameEvent* event, ImDrawList* drawList) noexcept
{
    if (visualsConfig.hitMarker == 0)
        return;

    static float lastHitTime = 0.0f;

    if (event) {
        if (localPlayer && event->getInt("attacker") == localPlayer->getUserId())
            lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + visualsConfig.hitMarkerTime < memory->globalVars->realtime)
        return;

    switch (visualsConfig.hitMarker) {
    case 1:
        const auto& mid = ImGui::GetIO().DisplaySize / 2.0f;
        constexpr auto color = IM_COL32(255, 255, 255, 255);
        drawList->AddLine({ mid.x - 10, mid.y - 10 }, { mid.x - 4, mid.y - 4 }, color);
        drawList->AddLine({ mid.x + 10.5f, mid.y - 10.5f }, { mid.x + 4.5f, mid.y - 4.5f }, color);
        drawList->AddLine({ mid.x + 10.5f, mid.y + 10.5f }, { mid.x + 4.5f, mid.y + 4.5f }, color);
        drawList->AddLine({ mid.x - 10, mid.y + 10 }, { mid.x - 4, mid.y + 4 }, color);
        break;
    }
}

void Visuals::disablePostProcessing(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    *memory->disablePostProcessing = stage == FrameStage::RENDER_START && visualsConfig.disablePostProcessing;
}

void Visuals::reduceFlashEffect() noexcept
{
    if (localPlayer)
        localPlayer->flashMaxAlpha() = 255.0f - visualsConfig.flashReduction * 2.55f;
}

bool Visuals::removeHands(const char* modelName) noexcept
{
    return visualsConfig.noHands && std::strstr(modelName, "arms") && !std::strstr(modelName, "sleeve");
}

bool Visuals::removeSleeves(const char* modelName) noexcept
{
    return visualsConfig.noSleeves && std::strstr(modelName, "sleeve");
}

bool Visuals::removeWeapons(const char* modelName) noexcept
{
    return visualsConfig.noWeapons && std::strstr(modelName, "models/weapons/v_")
        && !std::strstr(modelName, "arms") && !std::strstr(modelName, "tablet")
        && !std::strstr(modelName, "parachute") && !std::strstr(modelName, "fists");
}

void Visuals::skybox(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    if (stage == FrameStage::RENDER_START && visualsConfig.skybox > 0 && static_cast<std::size_t>(visualsConfig.skybox) < skyboxList.size()) {
        memory->loadSky(skyboxList[visualsConfig.skybox]);
    } else {
        static const auto sv_skyname = interfaces->cvar->findVar("sv_skyname");
        memory->loadSky(sv_skyname->string);
    }
}

void Visuals::bulletTracer(GameEvent& event) noexcept
{
    if (!visualsConfig.bulletTracers.enabled)
        return;

    if (!localPlayer)
        return;

    if (event.getInt("userid") != localPlayer->getUserId())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    BeamInfo beamInfo;

    if (!localPlayer->shouldDraw()) {
        const auto viewModel = interfaces->entityList->getEntityFromHandle(localPlayer->viewModel());
        if (!viewModel)
            return;

        if (!viewModel->getAttachment(activeWeapon->getMuzzleAttachmentIndex1stPerson(viewModel), beamInfo.start))
            return;
    } else {
        const auto worldModel = interfaces->entityList->getEntityFromHandle(activeWeapon->weaponWorldModel());
        if (!worldModel)
            return;

        if (!worldModel->getAttachment(activeWeapon->getMuzzleAttachmentIndex3rdPerson(), beamInfo.start))
            return;
    }

    beamInfo.end.x = event.getFloat("x");
    beamInfo.end.y = event.getFloat("y");
    beamInfo.end.z = event.getFloat("z");

    beamInfo.modelName = "sprites/physbeam.vmt";
    beamInfo.modelIndex = -1;
    beamInfo.haloName = nullptr;
    beamInfo.haloIndex = -1;

    beamInfo.red = 255.0f * visualsConfig.bulletTracers.asColor4().color[0];
    beamInfo.green = 255.0f * visualsConfig.bulletTracers.asColor4().color[1];
    beamInfo.blue = 255.0f * visualsConfig.bulletTracers.asColor4().color[2];
    beamInfo.brightness = 255.0f * visualsConfig.bulletTracers.asColor4().color[3];

    beamInfo.type = 0;
    beamInfo.life = 0.0f;
    beamInfo.amplitude = 0.0f;
    beamInfo.segments = -1;
    beamInfo.renderable = true;
    beamInfo.speed = 0.2f;
    beamInfo.startFrame = 0;
    beamInfo.frameRate = 0.0f;
    beamInfo.width = 2.0f;
    beamInfo.endWidth = 2.0f;
    beamInfo.flags = 0x40;
    beamInfo.fadeLength = 20.0f;

    if (const auto beam = memory->viewRenderBeams->createBeamPoints(beamInfo)) {
        constexpr auto FBEAM_FOREVER = 0x4000;
        beam->flags &= ~FBEAM_FOREVER;
        beam->die = memory->globalVars->currenttime + 2.0f;
    }
}

void Visuals::drawMolotovHull(ImDrawList* drawList) noexcept
{
    if (!visualsConfig.molotovHull.enabled)
        return;

    const auto color = Helpers::calculateColor(visualsConfig.molotovHull.asColor4());

    GameData::Lock lock;

    static const auto flameCircumference = [] {
        std::array<Vector, 72> points;
        for (std::size_t i = 0; i < points.size(); ++i) {
            constexpr auto flameRadius = 60.0f; // https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/server/cstrike15/Effects/inferno.cpp#L889
            points[i] = Vector{ flameRadius * std::cos(Helpers::deg2rad(i * (360.0f / points.size()))),
                                flameRadius * std::sin(Helpers::deg2rad(i * (360.0f / points.size()))),
                                0.0f };
        }
        return points;
    }();

    for (const auto& molotov : GameData::infernos()) {
        for (const auto& pos : molotov.points) {
            std::array<ImVec2, flameCircumference.size()> screenPoints;
            std::size_t count = 0;

            for (const auto& point : flameCircumference) {
                if (Helpers::worldToScreen(pos + point, screenPoints[count]))
                    ++count;
            }

            if (count < 1)
                continue;

            std::swap(screenPoints[0], *std::min_element(screenPoints.begin(), screenPoints.begin() + count, [](const auto& a, const auto& b) { return a.y < b.y || (a.y == b.y && a.x < b.x); }));

            constexpr auto orientation = [](const ImVec2& a, const ImVec2& b, const ImVec2& c) {
                return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
            };

            std::sort(screenPoints.begin() + 1, screenPoints.begin() + count, [&](const auto& a, const auto& b) { return orientation(screenPoints[0], a, b) > 0.0f; });
            drawList->AddConvexPolyFilled(screenPoints.data(), count, color);
        }
    }
}

void Visuals::drawSmokeTimer(ImDrawList* drawList) noexcept
{
    if (!visualsConfig.smokeTimer.enabled)
        return;

    if (!interfaces->engine->isInGame())
        return;

    GameData::Lock lock;
    for (const auto& smoke : GameData::smokes()) {
        const auto time = std::clamp(smoke.explosionTime + SMOKEGRENADE_LIFETIME - memory->globalVars->realtime, 0.f, SMOKEGRENADE_LIFETIME);
        std::ostringstream text; text << std::fixed << std::showpoint << std::setprecision(1) << time << " sec.";

        auto text_size = ImGui::CalcTextSize(text.str().c_str());
        ImVec2 pos;

        if (Helpers::worldToScreen(smoke.origin, pos)) {
            const auto radius = 10.f + visualsConfig.smokeTimer.timerThickness;
            const auto fraction = std::clamp(time / SMOKEGRENADE_LIFETIME, 0.0f, 1.0f);

            Helpers::setAlphaFactor(smoke.fadingAlpha());
            drawList->AddCircle(pos, radius, Helpers::calculateColor(visualsConfig.smokeTimer.backgroundColor), 40, 3.0f + visualsConfig.smokeTimer.timerThickness);
            if (fraction == 1.0f) {
                drawList->AddCircle(pos, radius, Helpers::calculateColor(visualsConfig.smokeTimer.timerColor), 40, 2.0f + visualsConfig.smokeTimer.timerThickness);
            }
            else {
                constexpr float pi = std::numbers::pi_v<float>;
                const auto arc270 = (3 * pi) / 2;
                drawList->PathArcTo(pos, radius - 0.5f, arc270 - (2 * pi * fraction), arc270, 40);
                drawList->PathStroke(Helpers::calculateColor(visualsConfig.smokeTimer.timerColor), false, 2.0f + visualsConfig.smokeTimer.timerThickness);
            }
            drawList->AddText(ImVec2(pos.x - (text_size.x / 2), pos.y + (visualsConfig.smokeTimer.timerThickness * 2.f) + (text_size.y / 2)), Helpers::calculateColor(visualsConfig.smokeTimer.textColor), text.str().c_str());
            constexpr auto s = "S";
            text_size = ImGui::CalcTextSize(s);
            drawList->AddText(ImVec2(pos.x - (text_size.x / 2), pos.y - (text_size.y / 2)), Helpers::calculateColor(visualsConfig.smokeTimer.textColor), s);
            Helpers::setAlphaFactor(1.f);
        }
    }
}

void Visuals::viewModel() noexcept
{
    static auto view_x{ interfaces->cvar->findVar("viewmodel_offset_x") };
    static auto view_y{ interfaces->cvar->findVar("viewmodel_offset_y") };
    static auto view_z{ interfaces->cvar->findVar("viewmodel_offset_z") };
    static auto minspec{ interfaces->cvar->findVar("sv_competitive_minspec") };

    //X Original
    static float original_x;
    static bool original_x_get = false;
    if (!original_x_get)
    {
        original_x = view_x->getFloat();
        original_x_get = true;
    }

    //Y Original
    static float original_y;
    static bool original_y_get = false;
    if (!original_y_get)
    {
        original_y = view_y->getFloat();
        original_y_get = true;
    }

    //Z Original
    static float original_z;
    static bool original_z_get = false;
    if (!original_z_get)
    {
        original_z = view_z->getFloat();
        original_z_get = true;
    }

    static bool originalPos = false;;

    if (visualsConfig.customPos)
    {
        minspec->onChangeCallbacks.size = 0;
        minspec->setValue(0);
        view_x->setValue(visualsConfig.x);
        view_y->setValue(visualsConfig.y);
        view_z->setValue(visualsConfig.z);

        originalPos = false;
    }
    else if (!visualsConfig.customPos && !originalPos)
    {
        view_x->setValue(original_x);
        view_y->setValue(original_y);
        view_z->setValue(original_z);
        minspec->onChangeCallbacks.size = 0;
        minspec->setValue(1);

        originalPos = true;
    }
}

void Visuals::drawAutoPeek(ImDrawList* drawList) noexcept
{
    if (!movement->config.autoPeek || !visualsConfig.autoPeekToggle.enabled || movement->AutoPeekPosition.null())
        return;
 
    constexpr float step = 3.141592654f * 2.0f / 20.0f;
    std::vector<ImVec2> points;
    for (float lat = 0.f; lat <= 3.141592654f * 2.0f; lat += step)
    {
        const auto& point3d = Vector{ std::sin(lat), std::cos(lat), 0.f } *15.f;
        ImVec2 point2d;
        if (worldToScreen(movement->AutoPeekPosition + point3d, point2d))
            points.push_back(point2d);
    }

    const ImU32 color = Helpers::calculateColor(visualsConfig.autoPeekToggle.asColor4());
    auto flags_backup = drawList->Flags;
    drawList->Flags |= ImDrawListFlags_AntiAliasedFill;
    drawList->AddConvexPolyFilled(points.data(), points.size(), color);
    drawList->AddPolyline(points.data(), points.size(), color, true, 2.f);
    drawList->Flags = flags_backup; 
}

float Visuals::aspectRatio() noexcept
{
    return visualsConfig.aspectratio;
}

void Visuals::ragdollForce(int force) noexcept
{
    if (lastRagdollForce != force) {
        const auto phys_pushscale = interfaces->cvar->findVar("phys_pushscale");
        if (phys_pushscale->getInt() != force)
            phys_pushscale->setValue(force);
        lastRagdollForce = force;
    }
}

void Visuals::fullBright(bool toggle) noexcept
{
    if (lastFullBrightState != toggle) {
        const auto mat_fullbright = interfaces->cvar->findVar("mat_fullbright");
        mat_fullbright->setValue(toggle);
        lastFullBrightState = toggle;
    }
}

void Visuals::updateEventListeners(bool forceRemove) noexcept
{
    class ImpactEventListener : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override { bulletTracer(*event); }
    };

    static ImpactEventListener listener;
    static bool listenerRegistered = false;

    if (visualsConfig.bulletTracers.enabled && !listenerRegistered) {
        interfaces->gameEventManager->addListener(&listener, "bullet_impact");
        listenerRegistered = true;
    } else if ((!visualsConfig.bulletTracers.enabled || forceRemove) && listenerRegistered) {
        interfaces->gameEventManager->removeListener(&listener);
        listenerRegistered = false;
    }
}

void Visuals::updateInput() noexcept
{
    visualsConfig.thirdpersonKey.handleToggle();
    visualsConfig.zoomKey.handleToggle();
    ragdollForce(visualsConfig.ragdollForce);
    fullBright(visualsConfig.fullBright);
}

void Visuals::drawGUI() noexcept
{
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 280.0f);
    ImGui::Checkbox("Disable post-processing", &visualsConfig.disablePostProcessing);
    ImGui::Checkbox("No fog", &visualsConfig.noFog);
    //ImGui::Checkbox("No 3d sky", &visualsConfig.no3dSky);
    ImGui::Checkbox("No aim punch", &visualsConfig.noAimPunch);
    ImGui::Checkbox("No view punch", &visualsConfig.noViewPunch);
    //ImGui::Checkbox("No hands", &visualsConfig.noHands);
    //ImGui::Checkbox("No sleeves", &visualsConfig.noSleeves);
    //ImGui::Checkbox("No weapons", &visualsConfig.noWeapons);
    ImGui::Checkbox("No smoke", &visualsConfig.noSmoke);
    ImGui::Checkbox("No blur", &visualsConfig.noBlur);
    ImGui::Checkbox("No scope overlay", &visualsConfig.noScopeOverlay);
    ImGui::Checkbox("No grass", &visualsConfig.noGrass);
    //ImGui::Checkbox("No shadows", &visualsConfig.noShadows);
    ImGui::Checkbox("Wireframe smoke", &visualsConfig.wireframeSmoke);
    
    //ImGuiCustom::colorPicker("Draw Bullet Tracers", visualsConfig.bulletTracers.asColor4().color.data(), &visualsConfig.bulletTracers.asColor4().color[3], nullptr, nullptr, &visualsConfig.bulletTracers.enabled);
    //ImGuiCustom::colorPicker("Draw Molotov Hull", visualsConfig.molotovHull);
    if (movement->config.autoPeek)
        ImGuiCustom::colorPicker("Draw Autopeek", visualsConfig.autoPeekToggle);

    ImGui::Checkbox("Smoke Timer", &visualsConfig.smokeTimer.enabled);
    if (visualsConfig.smokeTimer.enabled) {
        ImGui::SameLine();
        if (ImGui::Button("...##smoke_timer"))
            ImGui::OpenPopup("##popup_smokeTimer");

        if (ImGui::BeginPopup("##popup_smokeTimer"))
        {
            ImGuiCustom::colorPicker("Background color", visualsConfig.smokeTimer.backgroundColor);
            ImGuiCustom::colorPicker("Text color", visualsConfig.smokeTimer.textColor);
            ImGuiCustom::colorPicker("Timer color", visualsConfig.smokeTimer.timerColor, nullptr, &visualsConfig.smokeTimer.timerThickness);
            ImGui::EndPopup();
        }
    }

    /*ImGui::Checkbox("Color correction", &visualsConfig.colorCorrection.enabled);
    if (visualsConfig.colorCorrection.enabled) {
        ImGui::SameLine();
        if (bool ccPopup = ImGui::Button("Edit"))
            ImGui::OpenPopup("##popup");

        if (ImGui::BeginPopup("##popup")) {
            ImGui::VSliderFloat("##1", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.blue, 0.0f, 1.0f, "Blue\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##2", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.red, 0.0f, 1.0f, "Red\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##3", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.mono, 0.0f, 1.0f, "Mono\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##4", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.saturation, 0.0f, 1.0f, "Sat\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##5", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.ghost, 0.0f, 1.0f, "Ghost\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##6", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.green, 0.0f, 1.0f, "Green\n%.3f"); ImGui::SameLine();
            ImGui::VSliderFloat("##7", { 40.0f, 160.0f }, &visualsConfig.colorCorrection.yellow, 0.0f, 1.0f, "Yellow\n%.3f"); ImGui::SameLine();
            ImGui::EndPopup();
        }
    }*/

    ImGui::NextColumn();

    ImGui::Checkbox("Zoom", &visualsConfig.zoom);
    if (visualsConfig.zoom) {
        ImGui::SameLine();
        ImGui::PushID("Zoom Key");
        ImGui::hotkey("", visualsConfig.zoomKey);
        ImGui::PopID();
    }

    ImGui::Checkbox("Thirdperson", &visualsConfig.thirdperson);
    if (visualsConfig.thirdperson) {
        ImGui::SameLine();
        ImGui::PushID("Thirdperson Key");
        ImGui::hotkey("", visualsConfig.thirdpersonKey);
        ImGui::PopID();
        ImGui::PushItemWidth(290.0f);
        ImGui::PushID("Thirdperson Distance");
        ImGui::SliderInt("", &visualsConfig.thirdpersonDistance, 0, 1000, "Thirdperson distance: %d");
        ImGui::PopID();
        ImGui::PopItemWidth();
    }


    ImGui::PushItemWidth(290.0f);
    /*ImGui::PushID(1);
    ImGui::SliderInt("", &visualsConfig.viewmodelFov, -60, 60, "Viewmodel FOV: %d");
    ImGui::SameLine();
    if (ImGui::Button("...##visuals_5"))
        ImGui::OpenPopup("popup_viewModel");

    if (ImGui::BeginPopup("popup_viewModel"))
    {
        ImGui::Checkbox("Enabled", &visualsConfig.customPos);
        ImGui::SliderFloat("##visuals_7", &visualsConfig.x, -20.00f, 20.00f, "X: %.2f");
        ImGui::SliderFloat("##visuals_8", &visualsConfig.y, -20.00f, 20.00f, "Y: %.2f");
        ImGui::SliderFloat("##visuals_9", &visualsConfig.z, -20.00f, 20.00f, "Z: %.2f");
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PushID(2);
    ImGui::SliderInt("", &visualsConfig.fov, -60, 60, "FOV: %d");
    ImGui::PopID();
    ImGui::PushID(3);
    ImGui::SliderInt("", &visualsConfig.farZ, 0, 2000, "Far Z: %d");
    ImGui::PopID();*/
    ImGui::PushID(4);
    ImGui::SliderInt("", &visualsConfig.flashReduction, 0, 100, "Flash reduction: %d%%");
    ImGui::PopID(); 
    if (!visualsConfig.fullBright) {
        ImGui::PushID(5);
        ImGui::SliderFloat("", &visualsConfig.brightness, 0.0f, 1.0f, "Brightness: %.2f");
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::Checkbox(visualsConfig.fullBright ? "Max Brightness" : "Max", &visualsConfig.fullBright);
    ImGui::PushID(6);
    ImGui::SliderInt("", &visualsConfig.ragdollForce, 1, 800, visualsConfig.inverseRagdollGravity ? "Ragdoll Force: -%d%%" :"Ragdoll Force: %d%%");
    ImGui::PopID();
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Checkbox("Inverse", &visualsConfig.inverseRagdollGravity);

    //ImGui::Combo("Skybox", &visualsConfig.skybox, Visuals::skyboxList.data(), Visuals::skyboxList.size());
    //ImGuiCustom::colorPicker("World color", visualsConfig.world);
    //ImGuiCustom::colorPicker("Sky color", visualsConfig.sky);
    //ImGui::Checkbox("Deagle spinner", &visualsConfig.deagleSpinner);
    //ImGui::Combo("Screen effect", &visualsConfig.screenEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    //ImGui::Combo("Hit effect", &visualsConfig.hitEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    //ImGui::SliderFloat("Hit effect time", &visualsConfig.hitEffectTime, 0.1f, 1.5f, "%.2fs");
    
    ImGui::Combo("Hit marker", &visualsConfig.hitMarker, "None\0Default (Cross)\0");
    if (visualsConfig.hitMarker > 0)
        ImGui::SliderFloat("Hit marker time", &visualsConfig.hitMarkerTime, 0.1f, 1.5f, "%.2fs"); 
    //ImGui::SliderFloat("Aspect Ratio", &visualsConfig.aspectratio, 0.0f, 5.0f, "%.2f");

    
    ImGui::Columns(1);
}

static void from_json(const json& j, VisualsConfig::ColorCorrection& c)
{
    read(j, "Enabled", c.enabled);
    read(j, "Blue", c.blue);
    read(j, "Red", c.red);
    read(j, "Mono", c.mono);
    read(j, "Saturation", c.saturation);
    read(j, "Ghost", c.ghost);
    read(j, "Green", c.green);
    read(j, "Yellow", c.yellow);
}

static void from_json(const json& j, VisualsConfig::SmokeTimer& s)
{
    read(j, "Enabled", s.enabled);
    read<value_t::object>(j, "Background color", s.backgroundColor);
    read<value_t::object>(j, "Timer color", s.timerColor);
    read(j, "Timer thickness", s.timerThickness);
    read<value_t::object>(j, "Text color", s.textColor);
}

static void from_json(const json& j, BulletTracers& o)
{
    from_json(j, static_cast<ColorToggle&>(o));
}

static void from_json(const json& j, VisualsConfig& v)
{
    read(j, "Disable post-processing", v.disablePostProcessing);
    read(j, "Inverse ragdoll gravity", v.inverseRagdollGravity);
    read(j, "No fog", v.noFog);
    read(j, "No 3d sky", v.no3dSky);
    read(j, "No aim punch", v.noAimPunch);
    read(j, "No view punch", v.noViewPunch);
    read(j, "No hands", v.noHands);
    read(j, "No sleeves", v.noSleeves);
    read(j, "No weapons", v.noWeapons);
    read(j, "No smoke", v.noSmoke);
    read(j, "No blur", v.noBlur);
    read(j, "No scope overlay", v.noScopeOverlay);
    read(j, "No grass", v.noGrass);
    read(j, "No shadows", v.noShadows);
    read(j, "Wireframe smoke", v.wireframeSmoke);
    read(j, "Zoom", v.zoom);
    read(j, "Zoom key", v.zoomKey);
    read(j, "Thirdperson", v.thirdperson);
    read(j, "Thirdperson key", v.thirdpersonKey);
    read(j, "Thirdperson distance", v.thirdpersonDistance);
    read(j, "Ragdoll Force", v.ragdollForce);
    read(j, "Full Bright", v.fullBright);
    read(j, "Viewmodel FOV", v.viewmodelFov);
    read(j, "Custom Position", v.customPos);
    read(j, "Viewmodel X", v.x);
    read(j, "Viewmodel Y", v.y);
    read(j, "Viewmodel Z", v.z);
    read(j, "FOV", v.fov);
    read(j, "Far Z", v.farZ);
    read(j, "Flash reduction", v.flashReduction);
    read(j, "Brightness", v.brightness);
    read(j, "Skybox", v.skybox);
    read<value_t::object>(j, "World", v.world);
    read<value_t::object>(j, "Sky", v.sky);
    read(j, "Deagle spinner", v.deagleSpinner);
    read(j, "Screen effect", v.screenEffect);
    read(j, "Hit effect", v.hitEffect);
    read(j, "Hit effect time", v.hitEffectTime);
    read(j, "Hit marker", v.hitMarker);
    read(j, "Hit marker time", v.hitMarkerTime);
    read<value_t::object>(j, "Color correction", v.colorCorrection);
    read<value_t::object>(j, "Bullet Tracers", v.bulletTracers);
    read<value_t::object>(j, "Molotov Hull", v.molotovHull);
    read<value_t::object>(j, "Smoke timer", v.smokeTimer);
    read<value_t::object>(j, "Auto Peek Toggle", v.autoPeekToggle);
    read(j, "Aspect Ratio", v.aspectratio);
}

static void to_json(json& j, const VisualsConfig::ColorCorrection& o, const VisualsConfig::ColorCorrection& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Blue", blue);
    WRITE("Red", red);
    WRITE("Mono", mono);
    WRITE("Saturation", saturation);
    WRITE("Ghost", ghost);
    WRITE("Green", green);
    WRITE("Yellow", yellow);
}

static void to_json(json& j, const BulletTracers& o, const BulletTracers& dummy = {})
{
    to_json(j, static_cast<const ColorToggle&>(o), dummy);
}

static void to_json(json& j, const VisualsConfig::SmokeTimer& o, const VisualsConfig::SmokeTimer& dummy)
{
    WRITE("Enabled", enabled);
    WRITE("Background color", backgroundColor);
    WRITE("Timer color", timerColor);
    WRITE("Timer thickness", timerThickness);
    WRITE("Text color", textColor);
}

static void to_json(json& j, const VisualsConfig& o)
{
    const VisualsConfig dummy;

    WRITE("Disable post-processing", disablePostProcessing);
    WRITE("Inverse ragdoll gravity", inverseRagdollGravity);
    WRITE("No fog", noFog);
    WRITE("No 3d sky", no3dSky);
    WRITE("No aim punch", noAimPunch);
    WRITE("No view punch", noViewPunch);
    WRITE("No hands", noHands);
    WRITE("No sleeves", noSleeves);
    WRITE("No weapons", noWeapons);
    WRITE("No smoke", noSmoke);
    WRITE("No blur", noBlur);
    WRITE("No scope overlay", noScopeOverlay);
    WRITE("No grass", noGrass);
    WRITE("No shadows", noShadows);
    WRITE("Wireframe smoke", wireframeSmoke);
    WRITE("Zoom", zoom);
    WRITE("Zoom key", zoomKey);
    WRITE("Thirdperson", thirdperson);
    WRITE("Thirdperson key", thirdpersonKey);
    WRITE("Thirdperson distance", thirdpersonDistance);
    WRITE("Ragdoll Force", ragdollForce);
    WRITE("Full Bright", fullBright);
    WRITE("Viewmodel FOV", viewmodelFov);
    WRITE("Custom Position", customPos);
    WRITE("Viewmodel X", x);
    WRITE("Viewmodel Y", y);
    WRITE("Viewmodel Z", z);
    WRITE("FOV", fov);
    WRITE("Far Z", farZ);
    WRITE("Flash reduction", flashReduction);
    WRITE("Brightness", brightness);
    WRITE("Skybox", skybox);
    WRITE("World", world);
    WRITE("Sky", sky);
    WRITE("Deagle spinner", deagleSpinner);
    WRITE("Screen effect", screenEffect);
    WRITE("Hit effect", hitEffect);
    WRITE("Hit effect time", hitEffectTime);
    WRITE("Hit marker", hitMarker);
    WRITE("Hit marker time", hitMarkerTime);
    WRITE("Color correction", colorCorrection);
    WRITE("Bullet Tracers", bulletTracers);
    WRITE("Molotov Hull", molotovHull);
    WRITE("Smoke timer", smokeTimer);
    WRITE("Auto Peek Toggle", autoPeekToggle);
    WRITE("Aspect Ratio", aspectratio);
}

json Visuals::toJson() noexcept
{
    json j;
    to_json(j, visualsConfig);
    return j;
}

void Visuals::fromJson(const json& j) noexcept
{
    from_json(j, visualsConfig);
}

void Visuals::resetConfig() noexcept
{
    visualsConfig = {};
}