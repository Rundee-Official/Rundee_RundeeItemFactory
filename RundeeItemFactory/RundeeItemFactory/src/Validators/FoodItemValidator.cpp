// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Implementation of validation and balancing for ItemFoodData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/FoodItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>

static float ComputePower(const ItemFoodData& item)
{
    return static_cast<float>(
        item.hungerRestore +
        item.thirstRestore +
        item.healthRestore);
}

static void EnsureFoodShape(ItemFoodData& item)
{
    // Default category: Food
    if (item.category.empty())
        item.category = "Food";

    // Convert lowercase to proper case
    if (item.category == "food" || item.category == "FOOD")
        item.category = "Food";

    // Food items prioritize hungerRestore
    if (item.hungerRestore < 5)
        item.hungerRestore = 5;

    // Food items should have hungerRestore greater than thirstRestore
    if (item.hungerRestore < item.thirstRestore)
    {
        item.hungerRestore = item.thirstRestore + 5;
    }
}

static void EnsureSpoilage(ItemFoodData& item)
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

static void EnsureRarity(ItemFoodData& item)
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

void FoodItemValidator::Validate(ItemFoodData& item)
{
    std::string originalId = item.id;

    // 0) Normalize prefix: strip any leading food_ (any case, repeated), then add canonical "Food_"
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

        stripPrefix(item.id, "food_");
        item.id = "Food_" + item.id;
    }

    // #region agent log
    static int dbgCount = 0;
    if (dbgCount < 50)
    {
        ++dbgCount;
        auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
        if (dbg.is_open())
        {
            dbg << R"({"sessionId":"debug-session","runId":"prefix-debug","hypothesisId":"H2","location":"FoodItemValidator.cpp:Validate","message":"id prefix normalization","data":{"before":")"
                << originalId << R"(","after":")" << item.id << R"("},"timestamp":)" << ts << "})" << "\n";
        }
    }
    // #endregion

    // 1) Clamp basic values
    item.hungerRestore = JsonUtils::ClampInt(item.hungerRestore, 0, 100);
    item.thirstRestore = JsonUtils::ClampInt(item.thirstRestore, 0, 100);
    item.healthRestore = JsonUtils::ClampInt(item.healthRestore, 0, 100);

    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

    // 2) Ensure Food characteristics (hungerRestore priority)
    EnsureFoodShape(item);

    // 3) Handle spoilage
    EnsureSpoilage(item);

    // 4) Balance rarity
    EnsureRarity(item);

    // 5) Ensure description is not empty
    if (item.description.empty())
    {
        item.description = "A " + item.displayName + " that restores hunger.";
        std::cout << "[FoodItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
    }
}


