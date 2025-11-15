// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemFoodData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Data structure for survival game food items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once
#include <string>

struct ItemFoodData
{
    std::string id;
    std::string displayName;
    std::string category;
    std::string rarity;

    int maxStack = 1;

    int hungerRestore = 0;
    int thirstRestore = 0;
    int healthRestore = 0;

    bool spoils = false;
    int spoilTimeMinutes = 0;

    std::string description;
};
