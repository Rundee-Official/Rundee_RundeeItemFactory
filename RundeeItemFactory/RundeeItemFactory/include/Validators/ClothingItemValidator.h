/**
 * @file ClothingItemValidator.h
 * @brief Validation and balancing helpers for ItemClothingData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for clothing items, including:
 * - ID prefix normalization (ensures "Clothing_" prefix)
 * - Value clamping and range validation
 * - Clothing-specific shape validation
 * - Rarity balancing
 */

#pragma once

#include "Data/ItemClothingData.h"

/**
 * @class ClothingItemValidator
 * @brief Static validator class for clothing item data
 * 
 * Validates and balances clothing item data to ensure consistency and game balance.
 * Modifies the item in-place to fix issues and normalize values.
 */
class ClothingItemValidator
{
public:
    /**
     * @brief Validate and balance clothing item data
     * 
     * Performs comprehensive validation and balancing on a clothing item:
     * - Normalizes ID prefix to "Clothing_"
     * - Clamps values to valid ranges
     * - Ensures clothing characteristics
     * - Balances rarity based on item stats
     * - Generates default description if empty
     * 
     * @param[in,out] item Clothing item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Values are clamped to valid ranges automatically
     * @note Empty descriptions are replaced with default text
     */
    static void Validate(ItemClothingData& item);
};





