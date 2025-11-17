// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemFoodGenerator.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Helper class for generating food items (currently dummy data).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include "Data/ItemFoodData.h"

// Parameters for item generation
struct FoodGenerateParams
{
    int count = 1;
    int maxHunger = 100;
    int maxThirst = 100;

    std::string outputPath = "items_food.json";
};

class ItemFoodGenerator
{
public:
    std::vector<ItemFoodData> GenerateDummy(const FoodGenerateParams& params);
};


