#pragma once

class Settings
{
public:
    static Settings* GetSingleton();
    
    static void LoadSettings() noexcept;

    inline static bool debug_logging{};
};