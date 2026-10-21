#pragma once

#include "EquipPlacementManager.h"

static inline RE::Actor* AsActor(RE::TESObjectREFR* a_reference)
{
    return a_reference ? a_reference->As<RE::Actor>() : nullptr;
}

class EquipEventSink final : public EventSingleton<EquipEventSink, RE::TESEquipEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*) override
    {
        if (!a_event || !IsOneHandedSword(RE::TESForm::LookupByID(a_event->baseObject))) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* reference = a_event->actor.get();
        EquipPlacementManager::GetSingleton()->Update(AsActor(reference));

        return RE::BSEventNotifyControl::kContinue;
    }

private:
    static bool IsOneHandedSword(RE::TESForm* a_form)
    {
        auto* weapon = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
        return weapon && weapon->IsOneHandedSword();
    }
};

class Actor3DLoadedEventSink final : public EventSingleton<Actor3DLoadedEventSink, RE::TESObjectLoadedEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override
    {
        if (!a_event || !a_event->loaded) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* form = RE::TESForm::LookupByID(a_event->formID);
        EquipPlacementManager::GetSingleton()->Update(form ? form->As<RE::Actor>() : nullptr);

        return RE::BSEventNotifyControl::kContinue;
    }
};

class ActorInitScriptEventSink final : public EventSingleton<ActorInitScriptEventSink, RE::TESInitScriptEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESInitScriptEvent* a_event, RE::BSTEventSource<RE::TESInitScriptEvent>*) override
    {
        if (a_event) {
            EquipPlacementManager::GetSingleton()->Update(AsActor(a_event->objectInitialized.get()));
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

class ActorRaceSwitchEventSink final : public EventSingleton<ActorRaceSwitchEventSink, RE::TESSwitchRaceCompleteEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* a_event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override
    {
        if (a_event) {
            EquipPlacementManager::GetSingleton()->Update(AsActor(a_event->subject.get()));
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

class ActorLocationChangeEventSink final : public EventSingleton<ActorLocationChangeEventSink, RE::TESActorLocationChangeEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* a_event, RE::BSTEventSource<RE::TESActorLocationChangeEvent>*) override
    {
        if (a_event) {
            EquipPlacementManager::GetSingleton()->Update(AsActor(a_event->actor.get()));
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

class ActorNodeUpdateEventSink final : public EventSingleton<ActorNodeUpdateEventSink, SKSE::NiNodeUpdateEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const SKSE::NiNodeUpdateEvent* a_event, RE::BSTEventSource<SKSE::NiNodeUpdateEvent>*) override
    {
        if (a_event) {
            EquipPlacementManager::GetSingleton()->Update(AsActor(a_event->reference));
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

class CameraEventSink final : public EventSingleton<CameraEventSink, SKSE::CameraEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent* a_event, RE::BSTEventSource<SKSE::CameraEvent>*) override
    {
        if (a_event) {
            EquipPlacementManager::GetSingleton()->Update(RE::PlayerCharacter::GetSingleton());
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

class MenuCloseEventSink final : public EventSingleton<MenuCloseEventSink, RE::MenuOpenCloseEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        if (!a_event || a_event->opening || !IsRelevantMenu(a_event->menuName)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        EquipPlacementManager::GetSingleton()->Update(RE::PlayerCharacter::GetSingleton());
        return RE::BSEventNotifyControl::kContinue;
    }

private:
    static bool IsRelevantMenu(const RE::BSFixedString& a_menuName)
    {
        return a_menuName == "InventoryMenu" ||
            a_menuName == "ContainerMenu" ||
            a_menuName == "GiftMenu" ||
            a_menuName == "BarterMenu" ||
            a_menuName == "FavoritesMenu" ||
            a_menuName == "Console";
    }
};