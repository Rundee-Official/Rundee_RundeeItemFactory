// ===============================
// Project Name: RundeeItemFactory
// File Name: ClothingItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation and balancing for ItemClothingData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/ClothingItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <cctype>

static float ComputePower(const ItemClothingData& item)
{
    return static_cast<float>(
        item.warmth * 1.5f +
        item.style * 1.0f +
        item.value * 0.1f);
}

static void EnsureClothingShape(ItemClothingData& item)
{
    // Default category: Clothing
    if (item.category.empty())
        item.category = "Clothing";

    // Convert lowercase to proper case
    if (item.category == "clothing" || item.category == "CLOTHING")
        item.category = "Clothing";

    // Clothing items should have minimum warmth or style
    if (item.warmth < 1 && item.style < 1)
        item.warmth = 1;
}

static void EnsureRarity(ItemClothingData& item)
{
    float power = ComputePower(item);

    // If rarity is empty or invalid, auto-classify based on power
    if (item.rarity.empty() ||
        (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare"))
    {
        if (power <= 30.0f)      item.rarity = "Common";
        else if (power <= 100.0f) item.rarity = "Uncommon";
        else                     item.rarity = "Rare";
        return;
    }

    // Adjust to match rarity range
    float maxPower = 999.0f;
    if (item.rarity == "Common")
    {
        maxPower = 50.0f;
    }
    else if (item.rarity == "Uncommon")
    {
        maxPower = 150.0f;
    }
    else if (item.rarity == "Rare")
    {
        maxPower = 250.0f;
    }

    if (power <= maxPower)
        return; // Already within range

    // If power is too high, scale down while maintaining ratio
    float scale = maxPower / power;
    if (scale <= 0.0f)
        return;

    item.warmth = static_cast<int>(item.warmth * scale + 0.5f);
    item.warmth = JsonUtils::ClampInt(item.warmth, 0, 100);
    item.style = static_cast<int>(item.style * scale + 0.5f);
    item.style = JsonUtils::ClampInt(item.style, 0, 100);
}

void ClothingItemValidator::Validate(ItemClothingData& item)
{
    // 0) Normalize prefix: strip any leading clothing_ (any case, repeated), then add canonical "Clothing_"
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

    // 1) Clamp basic values
    item.warmth = JsonUtils::ClampInt(item.warmth, 0, 100);
    item.style = JsonUtils::ClampInt(item.style, 0, 100);
    item.durability = JsonUtils::ClampInt(item.durability, 0, 100);
    item.weight = JsonUtils::ClampInt(item.weight, 0, 100000);
    item.value = JsonUtils::ClampInt(item.value, 0, 999999);
    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

    // 2) Ensure Clothing characteristics
    EnsureClothingShape(item);

    // 3) Balance rarity
    EnsureRarity(item);

    // 4) Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that provides warmth and style.";
        std::cout << "[ClothingItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
    }
}





