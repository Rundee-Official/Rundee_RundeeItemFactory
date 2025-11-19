// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemDrinkData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Data structure for survival game drink items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "Data/ItemDataBase.h"

struct ItemDrinkData : public ItemDataBase
{
    // Drink-specific properties
    int hungerRestore = 0;
    int thirstRestore = 0;
    int healthRestore = 0;

    bool spoils = false;
    int spoilTimeMinutes = 0;

    // Override base class method
    std::string GetItemTypeName() const override { return "Drink"; }
};




