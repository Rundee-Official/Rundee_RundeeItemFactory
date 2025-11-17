// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonParser.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of unified JSON parser.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Parsers/ItemJsonParser.h"
#include "Utils/StringUtils.h"
#include "Utils/JsonUtils.h"
#include "Validators/FoodItemValidator.h"
#include "Validators/DrinkItemValidator.h"
#include "Validators/MaterialItemValidator.h"
#include <iostream>

using nlohmann::json;

bool ItemJsonParser::ParseFoodFromJsonText(const std::string& jsonText, std::vector<ItemFoodData>& outItems)
{
    outItems.clear();

    // Clean LLM response string
    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemFoodData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");

        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.hungerRestore = JsonUtils::GetIntSafe(jItem, "hungerRestore", 0);
        item.thirstRestore = JsonUtils::GetIntSafe(jItem, "thirstRestore", 0);
        item.healthRestore = JsonUtils::GetIntSafe(jItem, "healthRestore", 0);
        item.spoils = JsonUtils::GetBoolSafe(jItem, "spoils", false);
        item.spoilTimeMinutes = JsonUtils::GetIntSafe(jItem, "spoilTimeMinutes", 0);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        // Minimum validation: id/displayName must exist
        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        // Validation/balancing
        FoodItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " food items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseDrinkFromJsonText(const std::string& jsonText, std::vector<ItemDrinkData>& outItems)
{
    outItems.clear();

    // Clean LLM response string
    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemDrinkData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");

        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.hungerRestore = JsonUtils::GetIntSafe(jItem, "hungerRestore", 0);
        item.thirstRestore = JsonUtils::GetIntSafe(jItem, "thirstRestore", 0);
        item.healthRestore = JsonUtils::GetIntSafe(jItem, "healthRestore", 0);
        item.spoils = JsonUtils::GetBoolSafe(jItem, "spoils", false);
        item.spoilTimeMinutes = JsonUtils::GetIntSafe(jItem, "spoilTimeMinutes", 0);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        // Minimum validation: id/displayName must exist
        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        // Validation/balancing
        DrinkItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " drink items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseMaterialFromJsonText(const std::string& jsonText, std::vector<ItemMaterialData>& outItems)
{
    outItems.clear();

    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error: " << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemMaterialData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");
        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.materialType = JsonUtils::GetStringSafe(jItem, "materialType");
        item.hardness = JsonUtils::GetIntSafe(jItem, "hardness", 0);
        item.flammability = JsonUtils::GetIntSafe(jItem, "flammability", 0);
        item.value = JsonUtils::GetIntSafe(jItem, "value", 0);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping item at index "
                << i << " (missing id/displayName)\n";
            continue;
        }

        MaterialItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " material items from JSON.\n";

    return !outItems.empty();
}


