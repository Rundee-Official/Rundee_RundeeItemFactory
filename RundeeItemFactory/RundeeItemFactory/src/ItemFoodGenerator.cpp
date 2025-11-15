// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemFoodGenerator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Implementation of dummy food item generation.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "ItemFoodGenerator.h"
#include <iostream>

std::vector<ItemFoodData> ItemFoodGenerator::GenerateDummy(const FoodGenerateParams& params)
{
    std::vector<ItemFoodData> items;
    items.reserve(params.count);

    for (int i = 0; i < params.count; ++i)
    {
        ItemFoodData item;
        item.id = "item_dummy_food_" + std::to_string(i);
        item.displayName = "Dummy Food " + std::to_string(i);
        item.category = "Food";
        item.rarity = "Common";
        item.maxStack = 10;

        item.hungerRestore = 10 + i;   // 테스트용
        item.thirstRestore = 5;
        item.healthRestore = 0;

        item.spoils = true;
        item.spoilTimeMinutes = 30;

        item.description = "Placeholder food item generated without LLM.";

        items.push_back(item);
    }

    std::cout << "[ItemFoodGenerator] Generated " << items.size()
        << " dummy food items.\n";

    return items;
}
