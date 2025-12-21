/**
 * @file ArmorItemValidator.h
 * @brief Validation and balancing helpers for ItemArmorData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for armor items, including:
 * - ID prefix normalization (ensures "Armor_" prefix)
 * - Value clamping and range validation
 * - Armor-specific shape validation
 * - Rarity balancing
 */

#pragma once

#include "Data/ItemArmorData.h"

/**
 * @class ArmorItemValidator
 * @brief Static validator class for armor item data
 * 
 * Validates and balances armor item data to ensure consistency and game balance.
 * Modifies the item in-place to fix issues and normalize values.
 */
class ArmorItemValidator
{
public:
    /**
     * @brief Validate and balance armor item data
     * 
     * Performs comprehensive validation and balancing on an armor item:
     * - Normalizes ID prefix to "Armor_"
     * - Clamps values to valid ranges
     * - Ensures armor characteristics
     * - Balances rarity based on item stats
     * - Generates default description if empty
     * 
     * @param[in,out] item Armor item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Values are clamped to valid ranges automatically
     * @note Empty descriptions are replaced with default text
     */
    static void Validate(ItemArmorData& item);
};





