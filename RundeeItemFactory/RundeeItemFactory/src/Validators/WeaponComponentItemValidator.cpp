// ===============================
// Project Name: RundeeItemFactory
// File Name: WeaponComponentItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation for ItemWeaponComponentData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/WeaponComponentItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>

namespace WeaponComponentItemValidator
{
    void Validate(ItemWeaponComponentData& item)
    {
        // Add prefix to ID
        if (!item.id.empty() && item.id.find("WeaponComponent_") != 0)
        {
            item.id = "WeaponComponent_" + item.id;
        }

        // Default category
        if (item.category.empty())
            item.category = "WeaponComponent";

        // Clamp stat modifiers (can be negative, but reasonable range)
        item.damageModifier = JsonUtils::ClampInt(item.damageModifier, -50, 50);
        item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, -50, 50);
        item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -50, 50);
        item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, -50, 50);
        item.weightModifier = JsonUtils::ClampInt(item.weightModifier, -2000, 2000);
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " weapon component.";
            std::cout << "[WeaponComponentItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}

