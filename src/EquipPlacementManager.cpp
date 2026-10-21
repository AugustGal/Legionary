#include "EquipPlacementManager.h"

constexpr std::string_view kSpearKeyword = "AUG_WeapTypeSpear";

struct HandNodes
{
    std::string_view weaponNode;
    std::string_view hipNode;
    std::string_view backNode;
};

constexpr HandNodes kRightHand{
    .weaponNode = "WeaponSword",
    .hipNode    = "MOV WeaponSwordDefault",
    .backNode   = "MOV WeaponSwordOnBack",
};

constexpr HandNodes kLeftHand{
    .weaponNode = "WeaponSwordLeft",
    .hipNode    = "MOV WeaponSwordLeftDefault",
    .backNode   = "MOV WeaponSwordLeftOnBack",
};

RE::NiNode* AsNode(RE::NiAVObject* a_object)
{
    return a_object ? a_object->AsNode() : nullptr;
}

bool IsSpear(RE::TESForm* a_form)
{
    auto* weapon = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
    return weapon && weapon->IsOneHandedSword() && weapon->HasKeywordByEditorID(kSpearKeyword);
}

void SetWeaponPlacement(RE::NiNode* a_root, const HandNodes& a_nodes, bool a_onBack)
{
    if (!a_root) {
        return;
    }

    auto* weaponNode = AsNode(a_root->GetObjectByName(a_nodes.weaponNode.data()));
    const auto& targetName = a_onBack ? a_nodes.backNode : a_nodes.hipNode;
    auto* targetNode = AsNode(a_root->GetObjectByName(targetName.data()));

    if (!weaponNode || !targetNode || weaponNode == targetNode || !weaponNode->parent || weaponNode->parent == targetNode) {
        return;
    }

    targetNode->AttachChild(weaponNode, true);

    RE::NiUpdateData updateData{};
    updateData.flags = RE::NiUpdateData::Flag::kDisableCollision;

    weaponNode->UpdateDownwardPass(updateData, 0);
    weaponNode->UpdateWorldBound();
}

void ApplyPlacements(RE::NiNode* a_root, bool a_rightHandSpear, bool a_leftHandSpear)
{
    if (!a_root) {
        return;
    }

    SetWeaponPlacement(a_root, kRightHand, a_rightHandSpear);
    SetWeaponPlacement(a_root, kLeftHand, a_leftHandSpear);
}

EquipPlacementManager* EquipPlacementManager::GetSingleton()
{
    static EquipPlacementManager equipPlacementManager;
    return &equipPlacementManager;
}

void EquipPlacementManager::Update(RE::Actor* a_actor)
{
    if (!a_actor) {
        return;
    }

    const auto handle = a_actor->GetHandle();

    SKSE::GetTaskInterface()->AddTask([handle]() {
        auto actor = handle.get();

        if (!actor || !actor->Is3DLoaded()) {
            return;
        }

        auto* root3p = AsNode(actor->Get3D(false));
        auto* root1p = AsNode(actor->Get3D(true));

        if (!root3p && !root1p) {
            return;
        }

        const bool rightHandSpear = IsSpear(actor->GetEquippedObject(false));
        const bool leftHandSpear = IsSpear(actor->GetEquippedObject(true));

        if (root3p) {
            ApplyPlacements(root3p, rightHandSpear, leftHandSpear);
        }

        if (root1p && root1p != root3p) {
            ApplyPlacements(root1p, rightHandSpear, leftHandSpear);
        }
        });
}