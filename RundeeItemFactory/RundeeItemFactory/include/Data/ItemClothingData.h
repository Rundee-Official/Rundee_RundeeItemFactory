// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemClothingData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Data structure for clothing items (apparel and outfits).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "Data/ItemDataBase.h"

struct ItemClothingData : public ItemDataBase
{
    // Clothing category
    std::string clothingType;    // "Shirt", "Pants", "Jacket", "Hat", "Shoes", "Gloves", "Outfit", etc.
    
    // Environmental protection
    int coldResistance = 0;       // 0-100, protection against cold weather
    int heatResistance = 0;      // 0-100, protection against hot weather
    int waterResistance = 0;     // 0-100, protection against water/rain
    int windResistance = 0;      // 0-100, protection against wind
    
    // Comfort and mobility
    int comfort = 0;              // 0-100, overall comfort level
    int mobilityBonus = 0;        // -50 to 50, bonus/penalty to movement speed
    int staminaBonus = 0;         // -50 to 50, bonus/penalty to stamina regeneration
    
    // Durability
    int durability = 100;         // 0-100, clothing condition
    int material = 0;             // 0-100, material quality (affects durability)
    
    // Weight
    int weight = 0;               // Weight in grams
    
    // Special properties
    bool isInsulated = false;    // Whether clothing provides insulation
    bool isWaterproof = false;   // Whether clothing is waterproof
    bool isWindproof = false;    // Whether clothing is windproof
    
    // Override base class method
    std::string GetItemTypeName() const override { return "Clothing"; }
};

