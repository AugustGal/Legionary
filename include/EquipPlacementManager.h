#pragma once

class EquipPlacementManager
{
public:
    static EquipPlacementManager* GetSingleton();

    void RequestNodeUpdate(RE::Actor* a_actor);

    bool PlayerHasRequiredNodes();
};
