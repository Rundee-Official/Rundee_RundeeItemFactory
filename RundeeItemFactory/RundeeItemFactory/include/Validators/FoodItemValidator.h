/**
 * @file FoodItemValidator.h
 * @brief Validation and balancing helpers for ItemFoodData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-14
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for food items, including:
 * - ID prefix normalization (ensures "Food_" prefix)
 * - Value clamping and range validation
 * - Food-specific shape validation (hungerRestore priority)
 * - Spoilage handling
 * - Rarity balancing
 */

#pragma once

#include "Data/ItemFoodData.h"

/**
 * @class FoodItemValidator
 * @brief Static validator class for food item data
 * 
 * Validates and balances food item data to ensure consistency and game balance.
 * Modifies the item in-place to fix issues and normalize values.
 */
class FoodItemValidator
{
public:
    /**
     * @brief Validate and balance food item data
     * 
     * Performs comprehensive validation and balancing on a food item:
     * - Normalizes ID prefix to "Food_"
     * - Clamps values to valid ranges (hungerRestore, thirstRestore, healthRestore: 0-100)
     * - Ensures food characteristics (hungerRestore takes priority over thirstRestore)
     * - Handles spoilage logic
     * - Balances rarity based on item stats
     * - Generates default description if empty
     * 
     * @param[in,out] item Food item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Values are clamped to valid ranges automatically
     * @note Empty descriptions are replaced with default text
     */
    static void Validate(ItemFoodData& item);
};












