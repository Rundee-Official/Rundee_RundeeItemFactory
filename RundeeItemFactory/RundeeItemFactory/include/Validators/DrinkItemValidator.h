/**
 * @file DrinkItemValidator.h
 * @brief Validation and balancing helpers for ItemDrinkData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for drink items, including:
 * - ID prefix normalization (ensures "Drink_" prefix)
 * - Value clamping and range validation
 * - Drink-specific shape validation (thirstRestore priority)
 * - Spoilage handling
 * - Rarity balancing
 */

#pragma once

#include "Data/ItemDrinkData.h"

/**
 * @class DrinkItemValidator
 * @brief Static validator class for drink item data
 * 
 * Validates and balances drink item data to ensure consistency and game balance.
 * Modifies the item in-place to fix issues and normalize values.
 */
class DrinkItemValidator
{
public:
    /**
     * @brief Validate and balance drink item data
     * 
     * Performs comprehensive validation and balancing on a drink item:
     * - Normalizes ID prefix to "Drink_"
     * - Clamps values to valid ranges (hungerRestore, thirstRestore, healthRestore: 0-100)
     * - Ensures drink characteristics (thirstRestore takes priority over hungerRestore)
     * - Handles spoilage logic
     * - Balances rarity based on item stats
     * - Generates default description if empty
     * 
     * @param[in,out] item Drink item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Values are clamped to valid ranges automatically
     * @note Empty descriptions are replaced with default text
     */
    static void Validate(ItemDrinkData& item);
};












