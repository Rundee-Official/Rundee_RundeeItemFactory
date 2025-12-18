// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemArmorData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Data structure for armor items (combat protection gear).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "Data/ItemDataBase.h"

struct ItemArmorData : public ItemDataBase
{
    // Armor category
    std::string armorType;      // "Helmet", "Vest", "Armor", "Backpack", "Rig", "FaceCover", "EarProtection", etc.
    
    // Protection stats
    int armorClass = 0;          // 0-6, armor class (higher = better protection)
    int durability = 100;        // 0-100, armor condition
    int material = 0;            // 0-100, material quality (affects durability and protection)
    
    // Protection zones (what body parts are protected)
    std::string protectionZones; // Comma-separated: "Head", "Thorax", "Stomach", "Arms", "Legs"
    
    // Mobility impact
    int movementSpeedPenalty = 0;  // 0-100, percentage penalty to movement speed
    int ergonomicsPenalty = 0;     // 0-100, penalty to weapon handling
    int turnSpeedPenalty = 0;      // 0-100, penalty to turning speed
    
    // Weight and capacity
    int weight = 0;              // Weight in grams
    int capacity = 0;             // Storage capacity (for backpacks/rigs, in slots or liters)
    
    // Special properties
    bool blocksHeadset = false;  // Whether this armor blocks headset/ear protection
    bool blocksFaceCover = false; // Whether this armor blocks face cover
    
    // Override base class method
    std::string GetItemTypeName() const override { return "Armor"; }
};

