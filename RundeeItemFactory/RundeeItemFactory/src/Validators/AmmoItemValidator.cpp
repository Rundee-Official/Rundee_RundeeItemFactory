// ===============================
// Project Name: RundeeItemFactory
// File Name: AmmoItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation for ItemAmmoData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/AmmoItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>

namespace AmmoItemValidator
{
    void Validate(ItemAmmoData& item)
    {
        // Add prefix to ID
        if (!item.id.empty() && item.id.find("Ammo_") != 0)
        {
            item.id = "Ammo_" + item.id;
        }

        // Default category
        if (item.category.empty())
            item.category = "Ammo";

        // Clamp stat values
        item.damageBonus = JsonUtils::ClampInt(item.damageBonus, -50, 50);
        item.penetration = JsonUtils::ClampInt(item.penetration, 0, 100);
        item.accuracyBonus = JsonUtils::ClampInt(item.accuracyBonus, -50, 50);
        item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, -50, 50);
        item.value = JsonUtils::ClampInt(item.value, 0, 100);
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

        // Special property logic
        if (item.armorPiercing)
        {
            // Armor piercing ammo should have high penetration
            if (item.penetration < 50)
                item.penetration = 50;
        }

        if (item.hollowPoint)
        {
            // Hollow point has higher damage but lower penetration
            if (item.damageBonus < 5)
                item.damageBonus = 5;
            if (item.penetration > 30)
                item.penetration = 30;
        }

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " ammunition.";
            std::cout << "[AmmoItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}



