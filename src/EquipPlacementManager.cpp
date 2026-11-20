#include "EquipPlacementManager.h"
#include "Settings.h"

namespace
{
    enum class WeaponPlacement
    {
        kHip,
        kBack,
        kNone,
    };

    struct HandNodes
    {
        std::string_view weaponNode;
        std::string_view hipNode;
        std::string_view backNode;
    };

    constexpr HandNodes kRightHand{
        .weaponNode = "WeaponSword",
        .hipNode = "MOV WeaponSwordDefault",
        .backNode = "MOV WeaponSwordOnBack",
    };

    constexpr HandNodes kLeftHand{
        .weaponNode = "WeaponSwordLeft",
        .hipNode = "MOV WeaponSwordLeftDefault",
        .backNode = "MOV WeaponSwordLeftOnBack",
    };

    RE::NiNode* AsNiNode(RE::NiAVObject* a_object)
    {
        return a_object ? a_object->AsNode() : nullptr;
    }

    WeaponPlacement GetWeaponPlacement(RE::TESForm* a_form)
    {
        auto weapon = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;

        if (!weapon || !weapon->IsOneHandedSword()) {
            return WeaponPlacement::kNone;
        }

        return weapon->HasKeyword(Settings::GetSingleton()->SpearKeyword) ? WeaponPlacement::kBack : WeaponPlacement::kHip;
    }

    void ReparentWeaponNode(RE::NiNode* a_root, const HandNodes& a_nodes, WeaponPlacement a_placement)
    {
        if (!a_root) {
            return;
        }

        auto weaponNode = AsNiNode(a_root->GetObjectByName(a_nodes.weaponNode));
        auto targetNode = AsNiNode(a_root->GetObjectByName(a_placement == WeaponPlacement::kBack ? a_nodes.backNode : a_nodes.hipNode));

        if (!weaponNode || !targetNode || weaponNode->parent == targetNode) {
            return;
        }

        targetNode->AttachChild(weaponNode, true);

        RE::NiUpdateData updateData{};
        updateData.flags = RE::NiUpdateData::Flag::kDisableCollision;

        weaponNode->UpdateDownwardPass(updateData, 0);
        weaponNode->UpdateWorldBound();

        logger::debug("Reparented weapon node");
        return;
    }

    void ApplyWeaponPlacements(RE::NiNode* a_root, WeaponPlacement a_rightHand, WeaponPlacement a_leftHand)
    {
        if (!a_root) {
            return;
        }

        if (a_rightHand != WeaponPlacement::kNone) {
            ReparentWeaponNode(a_root, kRightHand, a_rightHand);
            logger::debug("Checking right hand nodes");
        }

        if (a_leftHand != WeaponPlacement::kNone) {
            ReparentWeaponNode(a_root, kLeftHand, a_leftHand);
            logger::debug("Checking left hand nodes");
        }
    }
}

EquipPlacementManager* EquipPlacementManager::GetSingleton()
{
    static EquipPlacementManager equipPlacementManager;
    return &equipPlacementManager;
}

void EquipPlacementManager::QueueWeaponPlacementUpdate(RE::Actor* a_actor)
{
    if (!a_actor) {
        return;
    }

    auto handle = a_actor->GetHandle();

    SKSE::GetTaskInterface()->AddTask([handle]() {
        auto actor = handle.get().get();
        if (!actor || !actor->Is3DLoaded()) {
            return;
        }

        auto root3p = AsNiNode(actor->Get3D(false));
        auto root1p = AsNiNode(actor->Get3D(true));

        const auto rightHand = GetWeaponPlacement(actor->GetEquippedObject(false));
        const auto leftHand = GetWeaponPlacement(actor->GetEquippedObject(true));

        if (root3p) {
            ApplyWeaponPlacements(root3p, rightHand, leftHand);
        }

        if (root1p) {
            ApplyWeaponPlacements(root1p, rightHand, leftHand);
        }
    });
}

bool EquipPlacementManager::VerifyPlayerSkeletonCompatbility()
{
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        logger::warn("Player wasn't loaded before checking the skeleton");
        return false;
    }

    auto root = AsNiNode(player->Get3D(false));
    if (!root) {
        logger::warn("Player's 3D wasn't loaded before checking the skeleton");
        return false;
    }

    for (auto hand : { &kRightHand, &kLeftHand }) {
        for (auto node : { hand->weaponNode, hand->hipNode, hand->backNode }) {
            if (!AsNiNode(root->GetObjectByName(node))) {
                logger::warn("Player has no compatible skeleton");
                return false;
            }
        }
    }

    logger::info("Player has compatible skeleton");
    return true;
}