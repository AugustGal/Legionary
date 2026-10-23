#include "Events.h"
#include "EquipPlacementManager.h"
#include "Settings.h"


void Listener(SKSE::MessagingInterface::Message* a_message) noexcept
{
    if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
        
        auto settings = Settings::GetSingleton();

        if (settings)
        {
            settings->LoadForms();
        }

        if (GetModuleHandleW(L"ImmersiveEquipmentDisplays.dll")) {
            logger::warn("IED detected. Skipped sink registration");
            return;
        }

        Player3DLoadedEventSink::Register();
        ActorEquipEventSink::Register();
        Actor3DLoadedEventSink::Register();
        ActorRaceSwitchEventSink::Register();
        LoadGameEventSink::Register();

        logger::info("Sinks registered");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    REL::Module::reset();

    Init(skse);

    const auto plugin{ SKSE::PluginDeclaration::GetSingleton() };
    const auto name{ plugin->GetName() };
    const auto version{ plugin->GetVersion() };

    logger::init();
    logger::info("{} {} is loading...", name, version);

    Settings::GetSingleton()->LoadSettings();

    if (const auto messaging{ SKSE::GetMessagingInterface() }; !messaging->RegisterListener(Listener)) {
        return false;
    }

    logger::info("{} has finished loading.", name);
    logger::info("");

    return true;
}