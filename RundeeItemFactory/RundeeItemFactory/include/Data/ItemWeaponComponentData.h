// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemWeaponComponentData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Data structure for weapon attachment components.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "Data/ItemDataBase.h"
#include <vector>
#include <string>

// Sub-slot definition (for components that can have other components attached)
struct ComponentAttachmentSlot
{
    std::string slotType;        // "Muzzle", "Grip", "Sight", "Rail", etc.
    int slotIndex = 0;          // For multiple slots of same type
    bool hasBuiltInRail = false; // If true, this slot already has a rail (e.g., handguard with integrated rail)
};

struct ItemWeaponComponentData : public ItemDataBase
{
    // Component type
    std::string componentType;   // "Muzzle", "Grip", "Sight", "Scope", "Stock", "Barrel", "Handguard", "Rail", "Flashlight", "Laser", etc.

    // Compatibility: which weapon slots can this component attach to
    std::vector<std::string> compatibleSlots;  // e.g., ["Muzzle", "Barrel"] for a muzzle device

    // Sub-slots: what can be attached to this component
    std::vector<ComponentAttachmentSlot> subSlots;  // e.g., Handguard can have Rail slots, Barrel can have Muzzle slot

    // Stat modifiers (applied when attached to weapon)
    int damageModifier = 0;      // Damage bonus/penalty
    int recoilModifier = 0;     // Recoil reduction (positive = less recoil)
    int ergonomicsModifier = 0; // Ergonomics bonus/penalty
    int accuracyModifier = 0;    // Accuracy bonus/penalty
    int weightModifier = 0;     // Weight change in grams

    // Special properties
    bool hasBuiltInRail = false; // If true, this component has an integrated rail (e.g., handguard with rail)
    std::string railType;        // "Picatinny", "M-LOK", "KeyMod", etc. (if hasBuiltInRail is true)

    // Override base class method
    std::string GetItemTypeName() const override { return "WeaponComponent"; }
};

