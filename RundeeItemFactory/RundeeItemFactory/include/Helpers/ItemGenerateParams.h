// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemGenerateParams.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Parameters for item generation.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>

// Parameters for item generation
struct FoodGenerateParams
{
    int count = 1;
    int maxHunger = 100;
    int maxThirst = 100;
    int maxHealth = 100;
    int maxStamina = 100;
    int maxWeight = 50000;  // grams (50kg default)
    int maxEnergy = 100;
    std::string outputPath = "ItemJson/items_food.json";
};








