#include <cassert>
#include <utility>
 
#include <Encryption/fnv.h>
#include <Encryption/xorstr.hpp>

#include "EventListener.h"
#include "GameData.h"
#include "Hacks/Misc.h"
#include "InventoryChanger/InventoryChanger.h"
#include "Hacks/Visuals.h"
#include "Interfaces.h"
#include "Memory.h"
#include "SDK/GameEvent.h"
#include "SDK/UtlVector.h"
#include "SDK/Engine.h"
#include "SDK/LocalPlayer.h"
#include "SDK/Entity.h"
#include <Hacks/Troll.h>
#include <Hacks/Sound.h>

namespace
{
    class EventListenerImpl : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override
        {
            switch (fnv::hashRuntime(event->getName())) 
            {
                case fnv::hash("round_start"):          GameData::clearProjectileList(); Misc::preserveKillfeed(true); [[fallthrough]];
                case fnv::hash("round_freeze_end"):     Misc::purchaseList(event); break;
                case fnv::hash("player_death"):         EventListener::OnKilledEvent(*event); break;
                case fnv::hash("player_hurt"):          EventListener::OnDamgeEvent(*event);  break;
                case fnv::hash("vote_cast"):            Misc::voteRevealer(*event); break;
                case fnv::hash("round_mvp"):            InventoryChanger::onRoundMVP(*event); Troll::chatSpam(ChatSpamEvents::OnMVP); break;
                case fnv::hash("cs_win_panel_match"):   Misc::autoDisconnect(); break;
            }
        }

        static EventListenerImpl& instance() noexcept
        {
            static EventListenerImpl impl;
            return impl;
        }
    };
}

void EventListener::init() noexcept
{
    assert(interfaces);

    // If you add here listeners which aren't used by client.dll (e.g., item_purchase, bullet_impact), the cheat will be detected by AntiDLL (community anticheat).
    // Instead, register listeners dynamically and only when certain functions are enabled - see Misc::updateEventListeners(), Visuals::updateEventListeners()

    const auto gameEventManager = interfaces->gameEventManager;
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("round_start"));
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("round_freeze_end"));
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("player_hurt"));
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("player_death"));
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("vote_cast"));
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("round_mvp"));
    gameEventManager->addListener(&EventListenerImpl::instance(), xorstr_("cs_win_panel_match"));
    
    // Move our player_death listener to the first position to override killfeed icons (InventoryChanger::overrideHudIcon()) before HUD gets them
    if (const auto desc = memory->getEventDescriptor(gameEventManager, xorstr_("player_death"), nullptr))
        std::swap(desc->listeners[0], desc->listeners[desc->listeners.size - 1]);
    else
        assert(false);

    // Move our round_mvp listener to the first position to override event data (InventoryChanger::onRoundMVP()) before HUD gets them
    if (const auto desc = memory->getEventDescriptor(gameEventManager, xorstr_("round_mvp"), nullptr))
        std::swap(desc->listeners[0], desc->listeners[desc->listeners.size - 1]);
    else
        assert(false);
}

void EventListener::remove() noexcept
{
    assert(interfaces);

    interfaces->gameEventManager->removeListener(&EventListenerImpl::instance());
}

void EventListener::OnKilledEvent(GameEvent &event) noexcept
{
    if (!localPlayer) return;

    const auto localUserId = localPlayer->getUserId();
    auto DeathByPlayer = event.getInt("attacker") != localUserId;
    auto DeathByBySuicde = event.getInt("userid") == localUserId;
    
    InventoryChanger::overrideHudIcon(event);

    if (DeathByPlayer) {
        Misc::killMessage(event);
        Sound::playKillSound(event);
        InventoryChanger::updateStatTrak(event); 
        Troll::chatSpam(ChatSpamEvents::OnKill);
    }
    else if (DeathByBySuicde) {
        Troll::chatSpam(ChatSpamEvents::OnDeath);
    }
}

void EventListener::OnDamgeEvent(GameEvent &event) noexcept
{
    Sound::playHitSound(event);
    Visuals::hitEffect(&event);
    Visuals::hitMarker(&event);
    Troll::chatSpam(ChatSpamEvents::OnDMG);
}