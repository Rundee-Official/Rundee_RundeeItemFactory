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

// Standard Library Includes
#include <string>
#include <vector>

// Project Includes
#include "Data/ItemAmmoData.h"
#include "Data/ItemArmorData.h"
#include "Data/ItemClothingData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemFoodData.h"
#include "Data/ItemMaterialData.h"
#include "Data/ItemWeaponComponentData.h"
#include "Data/ItemWeaponData.h"

// ============================================================================
// SECTION 1: ItemJsonParser Class
// ============================================================================

class ItemJsonParser
{
public:
    // ========================================================================
    // Parse Functions (by Item Type - Alphabetical Order)
    // ========================================================================
    
    // Parse ammo items from JSON text
    static bool ParseAmmoFromJsonText(const std::string& jsonText, std::vector<ItemAmmoData>& outItems);
    
    // Parse armor items from JSON text
    static bool ParseArmorFromJsonText(const std::string& jsonText, std::vector<ItemArmorData>& outItems);
    
    // Parse clothing items from JSON text
    static bool ParseClothingFromJsonText(const std::string& jsonText, std::vector<ItemClothingData>& outItems);
    
    // Parse drink items from JSON text
    static bool ParseDrinkFromJsonText(const std::string& jsonText, std::vector<ItemDrinkData>& outItems);
    
    // Parse food items from JSON text
    static bool ParseFoodFromJsonText(const std::string& jsonText, std::vector<ItemFoodData>& outItems);
    
    // Parse material items from JSON text
    static bool ParseMaterialFromJsonText(const std::string& jsonText, std::vector<ItemMaterialData>& outItems);
    
    // Parse weapon component items from JSON text
    static bool ParseWeaponComponentFromJsonText(const std::string& jsonText, std::vector<ItemWeaponComponentData>& outItems);
    
    // Parse weapon items from JSON text
    static bool ParseWeaponFromJsonText(const std::string& jsonText, std::vector<ItemWeaponData>& outItems);
};


