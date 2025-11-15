// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodJsonParser.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Implementation for parsing JSON text into ItemFoodData list.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "FoodJsonParser.h"
#include <iostream>

using nlohmann::json;

static int GetIntSafe(const json& j, const char* key, int defaultValue = 0)
{
    if (!j.contains(key))
        return defaultValue;
    if (j[key].is_number_integer())
        return j[key].get<int>();
    if (j[key].is_number())
        return static_cast<int>(j[key].get<double>());
    return defaultValue;
}

static bool GetBoolSafe(const json& j, const char* key, bool defaultValue = false)
{
    if (!j.contains(key))
        return defaultValue;
    if (j[key].is_boolean())
        return j[key].get<bool>();
    if (j[key].is_number_integer())
        return j[key].get<int>() != 0;
    return defaultValue;
}

static std::string GetStringSafe(const json& j, const char* key, const std::string& defaultValue = "")
{
    if (!j.contains(key))
        return defaultValue;
    if (j[key].is_string())
        return j[key].get<std::string>();
    return defaultValue;
}

bool FoodJsonParser::ParseFromJsonText(const std::string& jsonText,
    std::vector<ItemFoodData>& outItems)
{
    outItems.clear();

    json root;
    try
    {
        root = json::parse(jsonText);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[FoodJsonParser] JSON parse error: " << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[FoodJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[FoodJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemFoodData item;
        item.id = GetStringSafe(jItem, "id");
        item.displayName = GetStringSafe(jItem, "displayName");
        item.category = GetStringSafe(jItem, "category");
        item.rarity = GetStringSafe(jItem, "rarity");

        item.maxStack = GetIntSafe(jItem, "maxStack", 1);

        item.hungerRestore = GetIntSafe(jItem, "hungerRestore", 0);
        item.thirstRestore = GetIntSafe(jItem, "thirstRestore", 0);
        item.healthRestore = GetIntSafe(jItem, "healthRestore", 0);

        item.spoils = GetBoolSafe(jItem, "spoils", false);
        item.spoilTimeMinutes = GetIntSafe(jItem, "spoilTimeMinutes", 0);

        item.description = GetStringSafe(jItem, "description");

        // 간단 검증: id나 displayName이 비어 있으면 스킵
        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[FoodJsonParser] Skipping item at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        outItems.push_back(item);
    }

    std::cout << "[FoodJsonParser] Parsed " << outItems.size()
        << " items from JSON.\n";

    return !outItems.empty();
}
