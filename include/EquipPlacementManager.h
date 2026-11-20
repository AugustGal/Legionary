#pragma once

class EquipPlacementManager
{
public:
    static EquipPlacementManager* GetSingleton();

    void QueueWeaponPlacementUpdate(RE::Actor* a_actor);
    bool VerifyPlayerSkeletonCompatbility();
};