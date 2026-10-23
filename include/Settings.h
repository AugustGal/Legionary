#pragma once

class Settings
{
public:
    static Settings* GetSingleton();
    
    void LoadSettings() noexcept;

    void LoadForms();

    inline static bool debug_logging{};

    RE::BGSKeyword* SpearKeyword;

    static RE::FormID ParseFormID(const std::string& str);

    template<typename T>
    T* LoadFormPointerFromIni(std::string formId, std::string filename)
    {
        if (!formId.empty()) {
            auto objectFormId = ParseFormID(formId);
            auto form = RE::TESDataHandler::GetSingleton()->LookupForm(objectFormId, filename)->As<T>();
            return form;
        }
        else {
            return nullptr;
        }

    }
};