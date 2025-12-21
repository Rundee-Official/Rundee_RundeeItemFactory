// ===============================
// Project Name: RundeeItemFactory
// File Name: ArmorItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation and balancing for ItemArmorData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/ArmorItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <cctype>

static float ComputePower(const ItemArmorData& item)
{
    return static_cast<float>(
        item.armorValue * 2.0f +
        item.durability * 0.5f +
        item.value * 0.1f);
}

static void EnsureArmorShape(ItemArmorData& item)
{
    // Default category: Armor
    if (item.category.empty())
        item.category = "Armor";

    // Convert lowercase to proper case
    if (item.category == "armor" || item.category == "ARMOR")
        item.category = "Armor";

    // Armor items should have minimum armor value
    if (item.armorValue < 1)
        item.armorValue = 1;
}

static void EnsureRarity(ItemArmorData& item)
{
    float power = ComputePower(item);

    // If rarity is empty or invalid, auto-classify based on power
    if (item.rarity.empty() ||
        (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare"))
    {
        if (power <= 50.0f)      item.rarity = "Common";
        else if (power <= 150.0f) item.rarity = "Uncommon";
        else                     item.rarity = "Rare";
        return;
    }

    // Adjust to match rarity range
    float maxPower = 999.0f;
    if (item.rarity == "Common")
    {
        maxPower = 80.0f;
    }
    else if (item.rarity == "Uncommon")
    {
        maxPower = 200.0f;
    }
    else if (item.rarity == "Rare")
    {
        maxPower = 300.0f;
    }

    if (power <= maxPower)
        return; // Already within range

    // If power is too high, scale down while maintaining ratio
    float scale = maxPower / power;
    if (scale <= 0.0f)
        return;

    item.armorValue = static_cast<int>(item.armorValue * scale + 0.5f);
    item.armorValue = JsonUtils::ClampInt(item.armorValue, 1, 100);
}

void ArmorItemValidator::Validate(ItemArmorData& item)
{
    // 0) Normalize prefix: strip any leading armor_ (any case, repeated), then add canonical "Armor_"
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

    // 1) Clamp basic values
    item.armorValue = JsonUtils::ClampInt(item.armorValue, 0, 100);
    item.durability = JsonUtils::ClampInt(item.durability, 0, 100);
    item.weight = JsonUtils::ClampInt(item.weight, 0, 100000);
    item.value = JsonUtils::ClampInt(item.value, 0, 999999);
    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

    // 2) Ensure Armor characteristics
    EnsureArmorShape(item);

    // 3) Balance rarity
    EnsureRarity(item);

    // 4) Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that provides protection.";
        std::cout << "[ArmorItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
    }
}





