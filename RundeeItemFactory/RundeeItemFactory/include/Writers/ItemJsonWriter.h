// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonWriter.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Unified JSON writer for all item types.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMaterialData.h"

class ItemJsonWriter
{
public:
    // Write food items to JSON file
    static bool WriteFoodToFile(const std::vector<ItemFoodData>& items, const std::string& path);

    // Write drink items to JSON file
    static bool WriteDrinkToFile(const std::vector<ItemDrinkData>& items, const std::string& path);

    // Write material items to JSON file
    static bool WriteMaterialToFile(const std::vector<ItemMaterialData>& items, const std::string& path);
};


