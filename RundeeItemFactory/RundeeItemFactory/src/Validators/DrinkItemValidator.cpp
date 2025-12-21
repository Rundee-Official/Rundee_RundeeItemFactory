// ===============================
// Project Name: RundeeItemFactory
// File Name: DrinkItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation and balancing for ItemDrinkData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/DrinkItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <fstream>
#include <chrono>

static float ComputePower(const ItemDrinkData& item)
{
    return static_cast<float>(
        item.hungerRestore +
        item.thirstRestore +
        item.healthRestore);
}

static void EnsureDrinkShape(ItemDrinkData& item)
{
    // Default category: Drink
    if (item.category.empty())
        item.category = "Drink";

    // Convert lowercase to proper case
    if (item.category == "drink" || item.category == "DRINK")
        item.category = "Drink";

    // Drink items prioritize thirstRestore
    if (item.thirstRestore < 5)
        item.thirstRestore = 5;

    // Drink items should have thirstRestore greater than hungerRestore
    if (item.thirstRestore < item.hungerRestore)
    {
        item.thirstRestore = item.hungerRestore + 5;
    }
}

static void EnsureSpoilage(ItemDrinkData& item)
{
    // If spoils == false, spoilTimeMinutes should be 0
    if (!item.spoils)
    {
        item.spoilTimeMinutes = 0;
        return;
    }

    // If spoils == true, set min/max range (5 minutes ~ 7 days)
    int minMinutes = 5;
    int maxMinutes = 7 * 24 * 60; // 7 days
    item.spoilTimeMinutes = JsonUtils::ClampInt(item.spoilTimeMinutes, minMinutes, maxMinutes);
}

static void EnsureRarity(ItemDrinkData& item)
{
    float power = ComputePower(item);

    // If rarity is empty or invalid, auto-classify based on power
    if (item.rarity.empty() ||
        (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare"))
    {
        if (power <= 25.0f)      item.rarity = "Common";
        else if (power <= 55.0f) item.rarity = "Uncommon";
        else                     item.rarity = "Rare";
        return;
    }

    // Adjust to match rarity range
    float maxPower = 999.0f;
    if (item.rarity == "Common")
    {
        maxPower = 40.0f;
    }
    else if (item.rarity == "Uncommon")
    {
        maxPower = 65.0f;
    }
    else if (item.rarity == "Rare")
    {
        maxPower = 100.0f;
    }

    if (power <= maxPower)
        return; // Already within range

    // If power is too high, scale down while maintaining ratio
    float scale = maxPower / power;
    if (scale <= 0.0f)
        return;

    auto scaleAndClamp = [&](int& v)
        {
            int scaled = static_cast<int>(v * scale + 0.5f);
            v = JsonUtils::ClampInt(scaled, 0, 100);
        };

    scaleAndClamp(item.hungerRestore);
    scaleAndClamp(item.thirstRestore);
    scaleAndClamp(item.healthRestore);
}

void DrinkItemValidator::Validate(ItemDrinkData& item)
{
    std::string originalId = item.id;

    // 0) Normalize prefix: strip any leading drink_ (any case, repeated), then add canonical "Drink_"
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

        stripPrefix(item.id, "drink_");
        item.id = "Drink_" + item.id;
    }

    // Debug logging disabled for release

    // 1) Clamp basic values
    item.hungerRestore = JsonUtils::ClampInt(item.hungerRestore, 0, 100);
    item.thirstRestore = JsonUtils::ClampInt(item.thirstRestore, 0, 100);
    item.healthRestore = JsonUtils::ClampInt(item.healthRestore, 0, 100);

    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

    // 2) Ensure Drink characteristics (thirstRestore priority)
    EnsureDrinkShape(item);

    // 3) Handle spoilage
    EnsureSpoilage(item);

    // 4) Balance rarity
    EnsureRarity(item);

    // 5) Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that restores thirst.";
        std::cout << "[DrinkItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
    }
}


