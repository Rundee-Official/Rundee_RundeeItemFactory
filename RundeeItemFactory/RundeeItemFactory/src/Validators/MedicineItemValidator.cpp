// ===============================
// Project Name: RundeeItemFactory
// File Name: MedicineItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-20
// Description: Implementation of validation and balancing for ItemMedicineData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/MedicineItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>

static float ComputePower(const ItemMedicineData& item)
{
    return static_cast<float>(item.healthRestore);
}

static void EnsureMedicineShape(ItemMedicineData& item)
{
    // Default category: Medicine
    if (item.category.empty())
        item.category = "Medicine";

    // Convert lowercase to proper case
    if (item.category == "medicine" || item.category == "MEDICINE")
        item.category = "Medicine";

    // Medicine items prioritize healthRestore
    if (item.healthRestore < 10)
        item.healthRestore = 10;
}

static void EnsureSpoilage(ItemMedicineData& item)
{
    // If spoils == false, spoilTimeMinutes should be 0
    if (!item.spoils)
    {
        item.spoilTimeMinutes = 0;
        return;
    }

    // If spoils == true, set min/max range (60 minutes ~ 7 days)
    int minMinutes = 60;
    int maxMinutes = 7 * 24 * 60; // 7 days
    item.spoilTimeMinutes = JsonUtils::ClampInt(item.spoilTimeMinutes, minMinutes, maxMinutes);
}

static void EnsureRarity(ItemMedicineData& item)
{
    float power = ComputePower(item);

    // If rarity is empty or invalid, auto-classify based on power
    if (item.rarity.empty() ||
        (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare"))
    {
        if (power <= 30.0f)      item.rarity = "Common";
        else if (power <= 50.0f) item.rarity = "Uncommon";
        else                     item.rarity = "Rare";
        return;
    }

    // Adjust to match rarity range
    float maxPower = 999.0f;
    if (item.rarity == "Common")
    {
        maxPower = 30.0f;
    }
    else if (item.rarity == "Uncommon")
    {
        maxPower = 50.0f;
    }
    else if (item.rarity == "Rare")
    {
        maxPower = 80.0f;
    }

    if (power <= maxPower)
        return; // Already within range

    // If power is too high, scale down
    float scale = maxPower / power;
    if (scale <= 0.0f)
        return;

    int scaled = static_cast<int>(item.healthRestore * scale + 0.5f);
    item.healthRestore = JsonUtils::ClampInt(scaled, 0, 100);
}

void MedicineItemValidator::Validate(ItemMedicineData& item)
{
    std::string originalId = item.id;

    // 0) Normalize prefix: strip any leading medicine_ (any case, repeated), then add canonical "Medicine_"
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

        stripPrefix(item.id, "medicine_");
        item.id = "Medicine_" + item.id;
    }

    // Debug logging disabled for release

    // 1) Clamp basic values
    item.healthRestore = JsonUtils::ClampInt(item.healthRestore, 0, 100);

    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

    // 2) Ensure Medicine characteristics (healthRestore priority)
    EnsureMedicineShape(item);

    // 3) Handle spoilage
    EnsureSpoilage(item);

    // 4) Balance rarity
    EnsureRarity(item);

    // 5) Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that restores health.";
        std::cout << "[MedicineItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
    }
}

