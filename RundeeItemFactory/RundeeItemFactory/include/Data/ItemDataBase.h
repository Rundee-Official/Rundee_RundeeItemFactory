// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemDataBase.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Base class for all item data types.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>

// Base class for all item types
struct ItemDataBase
{
    // Identity
    std::string id;              // unique, lowercase, underscore
    std::string displayName;     // user-facing name

    // Classification
    std::string category;        // "Food", "Drink", "Material", "Junk", "Component", etc.
    std::string rarity;          // "Common", "Uncommon", "Rare"

    // Inventory
    int maxStack = 1;            // how many can stack in one slot

    // Flavor
    std::string description;

    // Virtual destructor for polymorphism
    virtual ~ItemDataBase() = default;

    // Virtual method to get item type name (for logging/debugging)
    virtual std::string GetItemTypeName() const = 0;
};






