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

#include "Data/ItemDataBase.h"

struct ItemFoodData : public ItemDataBase
{
    // Food-specific properties
    int hungerRestore = 0;
    int thirstRestore = 0;
    int healthRestore = 0;

    bool spoils = false;
    int spoilTimeMinutes = 0;

    // Override base class method
    std::string GetItemTypeName() const override { return "Food"; }
};




