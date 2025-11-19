// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemAmmoData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Data structure for ammunition items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "Data/ItemDataBase.h"
#include <string>

struct ItemAmmoData : public ItemDataBase
{
    // Ammo type
    std::string caliber;         // "9mm", "5.56mm", "7.62mm", "12gauge", ".45ACP", etc. (must match weapon caliber)

    // Combat stats
    int damageBonus = 0;         // Damage bonus/penalty (can be negative)
    int penetration = 0;         // 0-100, armor penetration (higher is better)
    int accuracyBonus = 0;       // Accuracy bonus/penalty (can be negative)
    int recoilModifier = 0;      // Recoil modifier (positive = less recoil, can be negative)

    // Special properties
    bool armorPiercing = false;  // If true, has high penetration
    bool hollowPoint = false;    // If true, higher damage but lower penetration
    bool tracer = false;         // If true, visible trajectory
    bool incendiary = false;     // If true, can cause fire damage

    // Value
    int value = 0;               // 0-100, trade/scrap value

    // Override base class method
    std::string GetItemTypeName() const override { return "Ammo"; }
};



