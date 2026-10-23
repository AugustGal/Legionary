#pragma once

#include "EquipPlacementManager.h"

static inline RE::Actor* AsActor(RE::TESObjectREFR* a_reference)
{
    return a_reference ? a_reference->As<RE::Actor>() : nullptr;
}

class ActorEquipEventSink final : public EventSingleton<ActorEquipEventSink, RE::TESEquipEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>*) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* reference = a_event->actor.get();
        EquipPlacementManager::GetSingleton()->RequestNodeUpdate(AsActor(reference));

        return RE::BSEventNotifyControl::kContinue;
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
        EquipPlacementManager::GetSingleton()->RequestNodeUpdate(form ? form->As<RE::Actor>() : nullptr);

        return RE::BSEventNotifyControl::kContinue;
    }
};

class ActorRaceSwitchEventSink final : public EventSingleton<ActorRaceSwitchEventSink, RE::TESSwitchRaceCompleteEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* a_event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override
    {
        if (a_event) {
            EquipPlacementManager::GetSingleton()->RequestNodeUpdate(AsActor(a_event->subject.get()));
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

class LoadGameEventSink final : public EventSingleton<LoadGameEventSink, RE::TESLoadGameEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>*) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto player = RE::PlayerCharacter::GetSingleton();
        auto manager = EquipPlacementManager::GetSingleton();
        manager->RequestNodeUpdate(player);

        auto processLists = RE::ProcessLists::GetSingleton();
        if (!processLists) {
            return RE::BSEventNotifyControl::kContinue;
        }

        processLists->ForAllActors([&](RE::Actor* a_actor) {
            if (a_actor) {
                manager->RequestNodeUpdate(a_actor);
            }

            return RE::BSContainer::ForEachResult::kContinue;
        });

        return RE::BSEventNotifyControl::kContinue;
    }
};

inline void UnregisterAllSinks()
{
    auto source = RE::ScriptEventSourceHolder::GetSingleton();
    if (source) {
        source->RemoveEventSink<RE::TESEquipEvent>(ActorEquipEventSink::GetSingleton());

        source->RemoveEventSink<RE::TESObjectLoadedEvent>(Actor3DLoadedEventSink::GetSingleton());

        source->RemoveEventSink<RE::TESSwitchRaceCompleteEvent>(ActorRaceSwitchEventSink::GetSingleton());

        source->RemoveEventSink<RE::TESLoadGameEvent>(LoadGameEventSink::GetSingleton());

        logger::info("Sinks unregistered");
    }
}

class Player3DLoadedEventSink final : public EventSingleton<Player3DLoadedEventSink, RE::TESObjectLoadedEvent>
{
public:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_source) override
    {
        if (!a_event || !a_event->loaded) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || a_event->formID != player->GetFormID()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (a_source) {
            a_source->RemoveEventSink(this);
        }

        if (!EquipPlacementManager::GetSingleton()->PlayerHasRequiredNodes()) {
            UnregisterAllSinks();
            return RE::BSEventNotifyControl::kContinue;
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};