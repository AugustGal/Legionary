#include "Settings.h"
#include "SimpleIni.h"

Settings* Settings::GetSingleton()
{
    static Settings settings;
    return &settings;
}

void Settings::LoadSettings() noexcept
{
    logger::info("Loading settings");

    CSimpleIniA ini;

    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\Legionary.ini)");

    debug_logging = ini.GetBoolValue("Log", "Debug");

    if (debug_logging) {
        spdlog::set_level(spdlog::level::debug);
        logger::debug("Debug logging enabled");
    }

    logger::info("Loaded settings");
}

void Settings::LoadForms()
{
    CSimpleIniA ini;

    ini.SetUnicode();
    ini.LoadFile(R"(.\Data\SKSE\Plugins\Legionary.ini)");

    std::string spearKeywordFormId((ini.GetValue("Settings", "SpearKeywordFormId", "")));
    std::string spearKeywordPlugin((ini.GetValue("Settings", "SpearKeywordPlugin", "")));
    SpearKeyword = LoadFormPointerFromIni<RE::BGSKeyword>(spearKeywordFormId, spearKeywordPlugin);
}

RE::FormID Settings::ParseFormID(const std::string& str)
{
    RE::FormID result;
    std::istringstream ss{ str };
    ss >> std::hex >> result;
    return result;
}