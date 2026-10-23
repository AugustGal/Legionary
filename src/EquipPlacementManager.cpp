#include "EquipPlacementManager.h"
#include "Settings.h"

namespace
{
    enum class WeaponType
    {
        kSword,
        kSpear,
        kNotSwordOrSpear,
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

    RE::NiNode* AsNode(RE::NiAVObject* a_object)
    {
        return a_object ? a_object->AsNode() : nullptr;
    }

    WeaponType GetWeaponType(RE::TESForm* a_form)
    {
        auto weapon = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;

        if (!weapon || !weapon->IsOneHandedSword()) {
            return WeaponType::kNotSwordOrSpear;
        }

        return weapon->HasKeyword(Settings::GetSingleton()->SpearKeyword) ? WeaponType::kSpear : WeaponType::kSword;
    }

    bool MoveWeaponNode(RE::NiNode* a_root, const HandNodes& a_nodes, bool a_onBack)
    {
        if (!a_root) {
            return false;
        }

        auto weaponNode = AsNode(a_root->GetObjectByName(a_nodes.weaponNode));
        auto targetNode = AsNode(a_root->GetObjectByName(a_onBack ? a_nodes.backNode : a_nodes.hipNode));

        if (!weaponNode || !targetNode || weaponNode->parent == targetNode) {
            return false ;
        }

        targetNode->AttachChild(weaponNode, true);

        RE::NiUpdateData updateData{};
        updateData.flags = RE::NiUpdateData::Flag::kDisableCollision;

        weaponNode->UpdateDownwardPass(updateData, 0);
        weaponNode->UpdateWorldBound();

        logger::debug("Moved weapon node");
        return true;
    }

    void VerifyPlacements(RE::NiNode* a_root, WeaponType a_rightHand, WeaponType a_leftHand)
    {
        if (!a_root) {
            return;
        }

        if (a_rightHand != WeaponType::kNotSwordOrSpear) {
            MoveWeaponNode(a_root, kRightHand, a_rightHand == WeaponType::kSpear);
            logger::debug("Checking right hand nodes");
        }

        if (a_leftHand != WeaponType::kNotSwordOrSpear) {
            MoveWeaponNode(a_root, kLeftHand, a_leftHand == WeaponType::kSpear);
            logger::debug("Checking left hand nodes");
        }
    }
}

EquipPlacementManager* EquipPlacementManager::GetSingleton()
{
    static EquipPlacementManager equipPlacementManager;
    return &equipPlacementManager;
}

void EquipPlacementManager::RequestNodeUpdate(RE::Actor* a_actor)
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

        auto root3p = AsNode(actor->Get3D(false));
        auto root1p = AsNode(actor->Get3D(true));

        if (!root3p && !root1p) {
            return;
        }

        const auto rightHand = GetWeaponType(actor->GetEquippedObject(false));
        const auto leftHand = GetWeaponType(actor->GetEquippedObject(true));

        if (root3p) {
            VerifyPlacements(root3p, rightHand, leftHand);
        }

        if (root1p) {
            VerifyPlacements(root1p, rightHand, leftHand);
        }
    });
}

bool EquipPlacementManager::PlayerHasRequiredNodes()
{
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        logger::warn("Player wasn't loaded before checking the skeleton");
        return false;
    }

    auto root = AsNode(player->Get3D(false));
    if (!root) {
        logger::warn("Player's 3D wasn't loaded before checking the skeleton");
        return false;
    }

    for (auto hand : { &kRightHand, &kLeftHand }) {
        for (auto node : { hand->weaponNode, hand->hipNode, hand->backNode }) {
            if (!AsNode(root->GetObjectByName(node))) {
                logger::warn("Player has no compatible skeleton");
                return false;
            }
        }
    }

    logger::info("Player has compatible skeleton");
    return true;
}