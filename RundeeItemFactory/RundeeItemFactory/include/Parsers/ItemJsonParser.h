// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonParser.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Unified JSON parser for all item types.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMaterialData.h"
#include "Data/ItemWeaponData.h"
#include "Data/ItemWeaponComponentData.h"
#include "Data/ItemAmmoData.h"

class ItemJsonParser
{
public:
    // Parse food items from JSON text
    static bool ParseFoodFromJsonText(const std::string& jsonText, std::vector<ItemFoodData>& outItems);

    // Parse drink items from JSON text
    static bool ParseDrinkFromJsonText(const std::string& jsonText, std::vector<ItemDrinkData>& outItems);

    // Parse material items from JSON text
    static bool ParseMaterialFromJsonText(const std::string& jsonText, std::vector<ItemMaterialData>& outItems);

    // Parse weapon items from JSON text
    static bool ParseWeaponFromJsonText(const std::string& jsonText, std::vector<ItemWeaponData>& outItems);

    // Parse weapon component items from JSON text
    static bool ParseWeaponComponentFromJsonText(const std::string& jsonText, std::vector<ItemWeaponComponentData>& outItems);

    // Parse ammo items from JSON text
    static bool ParseAmmoFromJsonText(const std::string& jsonText, std::vector<ItemAmmoData>& outItems);
};


