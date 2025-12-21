/**
 * @file ItemClothingData.h
 * @brief Data structure for clothing items
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for clothing items that provide warmth and style.
 * Inherits common item properties from ItemDataBase.
 */

#pragma once

#include "Data/ItemDataBase.h"

/**
 * @struct ItemClothingData
 * @brief Data structure representing a clothing item
 * 
 * Contains all properties specific to clothing items, including warmth
 * and style information.
 */
struct ItemClothingData : public ItemDataBase
{
    /**
     * @brief Warmth value (0-100)
     * 
     * Amount of warmth provided. Higher values indicate better insulation.
     */
    int warmth = 0;

    /**
     * @brief Style value (0-100)
     * 
     * Aesthetic appeal of the clothing. Affects social interactions.
     */
    int style = 0;

    /**
     * @brief Clothing durability (0-100)
     * 
     * Current condition of the clothing. Decreases with use.
     */
    int durability = 100;

    /**
     * @brief Weight in grams
     * 
     * How much the clothing weighs.
     */
    int weight = 0;

    /**
     * @brief Clothing type
     * 
     * Examples: "Shirt", "Pants", "Jacket", "Hat", "Shoes"
     */
    std::string clothingType;

    /**
     * @brief Item value
     * 
     * Base value for trading and economy.
     */
    int value = 0;

    /**
     * @brief Get the item type name
     * @return Always returns "Clothing"
     */
    std::string GetItemTypeName() const override { return "Clothing"; }
};





