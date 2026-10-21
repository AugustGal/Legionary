#include "Events.h"
#include "Settings.h"


void Listener(SKSE::MessagingInterface::Message* a_message)
{
    if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {

        if (GetModuleHandleW(L"ImmersiveEquipmentDisplays.dll")) {
            logger::info("IED detected. Skipping equipment placement registration.");
            return;
        }

        EquipEventSink::Register();
        Actor3DLoadedEventSink::Register();
        ActorInitScriptEventSink::Register();
        ActorRaceSwitchEventSink::Register();
        ActorLocationChangeEventSink::Register();
        ActorNodeUpdateEventSink::Register();
        CameraEventSink::Register();
        MenuCloseEventSink::Register();

        logger::info("Equipment placement sinks registered.");
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

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(Listener)) {
        return false;
    }

    logger::info("{} has finished loading.", name);
    logger::info("");

    return true;
}
