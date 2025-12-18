// ===============================
// Project Name: RundeeItemFactory
// File Name: ClothingItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Implementation of validation and balancing for ItemClothingData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <algorithm>
#include <cctype>
#include <iostream>

// Project Includes
#include "Validators/ClothingItemValidator.h"
#include "Utils/JsonUtils.h"

// ============================================================================
// SECTION 1: ClothingItemValidator Class Implementation
// ============================================================================

void ClothingItemValidator::Validate(ItemClothingData& item)
{
    std::string originalId = item.id;

    // Normalize prefix: strip any leading clothing_ (any case, repeated), then add canonical "Clothing_"
    if (!item.id.empty())
    {
        auto stripPrefix = [](std::string& value, const std::string& prefixLower)
        {
            std::string lower = value;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            while (lower.rfind(prefixLower, 0) == 0)
            {
                value = value.substr(prefixLower.size());
                lower = value;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            }
        };

        stripPrefix(item.id, "clothing_");
        item.id = "Clothing_" + item.id;
    }

    // Default category
    if (item.category.empty())
        item.category = "Clothing";

    // Clamp stat values
    item.coldResistance = JsonUtils::ClampInt(item.coldResistance, 0, 100);
    item.heatResistance = JsonUtils::ClampInt(item.heatResistance, 0, 100);
    item.waterResistance = JsonUtils::ClampInt(item.waterResistance, 0, 100);
    item.windResistance = JsonUtils::ClampInt(item.windResistance, 0, 100);
    item.comfort = JsonUtils::ClampInt(item.comfort, 0, 100);
    item.mobilityBonus = JsonUtils::ClampInt(item.mobilityBonus, -50, 50);
    item.staminaBonus = JsonUtils::ClampInt(item.staminaBonus, -50, 50);
    item.durability = JsonUtils::ClampInt(item.durability, 0, 100);
    item.material = JsonUtils::ClampInt(item.material, 0, 100);
    item.weight = JsonUtils::ClampInt(item.weight, 0, 10000); // Max 10kg
    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 10); // Clothing typically doesn't stack

    // Default clothingType if empty
    if (item.clothingType.empty())
    {
        item.clothingType = "Shirt"; // Default to shirt
    }

    // Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that provides comfort and protection.";
    }
}

