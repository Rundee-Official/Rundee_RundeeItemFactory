// ===============================
// Project Name: RundeeItemFactory
// File Name: WeaponItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation and balancing for ItemWeaponData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/WeaponItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>

namespace WeaponItemValidator
{
    void Validate(ItemWeaponData& item)
    {
        // Add prefix to ID
        if (!item.id.empty() && item.id.find("Weapon_") != 0)
        {
            item.id = "Weapon_" + item.id;
        }

        // Default category
        if (item.category.empty())
            item.category = "Weapon";

        // Clamp stat values
        item.minDamage = JsonUtils::ClampInt(item.minDamage, 0, 100);
        item.maxDamage = JsonUtils::ClampInt(item.maxDamage, 0, 100);
        item.fireRate = JsonUtils::ClampInt(item.fireRate, 0, 1200);
        item.accuracy = JsonUtils::ClampInt(item.accuracy, 0, 100);
        item.recoil = JsonUtils::ClampInt(item.recoil, 0, 100);
        item.ergonomics = JsonUtils::ClampInt(item.ergonomics, 0, 100);
        item.durability = JsonUtils::ClampInt(item.durability, 0, 100);
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

        // Ensure maxDamage >= minDamage
        if (item.maxDamage < item.minDamage)
        {
            item.maxDamage = item.minDamage;
        }

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " weapon.";
            std::cout << "[WeaponItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}

