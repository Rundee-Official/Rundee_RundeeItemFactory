// ===============================
// Project Name: RundeeItemFactory
// File Name: ArmorItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Implementation of validation and balancing for ItemArmorData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <algorithm>
#include <cctype>
#include <iostream>

// Project Includes
#include "Validators/ArmorItemValidator.h"
#include "Utils/JsonUtils.h"

// ============================================================================
// SECTION 1: ArmorItemValidator Class Implementation
// ============================================================================

void ArmorItemValidator::Validate(ItemArmorData& item)
{
    std::string originalId = item.id;

    // Normalize prefix: strip any leading armor_ (any case, repeated), then add canonical "Armor_"
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

        stripPrefix(item.id, "armor_");
        item.id = "Armor_" + item.id;
    }

    // Default category
    if (item.category.empty())
        item.category = "Armor";

    // Clamp stat values
    item.armorClass = JsonUtils::ClampInt(item.armorClass, 0, 6);
    item.durability = JsonUtils::ClampInt(item.durability, 0, 100);
    item.material = JsonUtils::ClampInt(item.material, 0, 100);
    item.movementSpeedPenalty = JsonUtils::ClampInt(item.movementSpeedPenalty, 0, 100);
    item.ergonomicsPenalty = JsonUtils::ClampInt(item.ergonomicsPenalty, 0, 100);
    item.turnSpeedPenalty = JsonUtils::ClampInt(item.turnSpeedPenalty, 0, 100);
    item.weight = JsonUtils::ClampInt(item.weight, 0, 50000); // Max 50kg
    item.capacity = JsonUtils::ClampInt(item.capacity, 0, 100);
    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 10); // Armor typically doesn't stack

    // Default armorType if empty
    if (item.armorType.empty())
    {
        item.armorType = "Vest"; // Default to vest
    }

    // Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that provides protection.";
    }
}

