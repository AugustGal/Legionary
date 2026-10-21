#pragma once

class EquipPlacementManager
{
public:
    static EquipPlacementManager* GetSingleton();

    void Update(RE::Actor* a_actor);
};
