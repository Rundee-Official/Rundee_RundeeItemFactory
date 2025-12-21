/**
 * @file MaterialItemValidator.h
 * @brief Validation and balancing helpers for ItemMaterialData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for material items, including:
 * - ID prefix normalization (ensures "Material_" prefix)
 * - Material property validation (hardness, flammability, value)
 * - Material type validation
 * - Rarity balancing
 */

#pragma once

#include "Data/ItemMaterialData.h"

/**
 * @namespace MaterialItemValidator
 * @brief Namespace for material item validation functions
 */
namespace MaterialItemValidator
{
    /**
     * @brief Validate and balance material item data
     * 
     * Performs comprehensive validation and balancing on a material item:
     * - Normalizes ID prefix to "Material_"
     * - Clamps material properties to valid ranges (hardness, flammability, value: 0-100)
     * - Validates material type
     * - Balances rarity based on item stats
     * - Generates default description if empty
     * 
     * @param[in,out] item Material item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Values are clamped to valid ranges automatically
     */
    void Validate(ItemMaterialData& item);
}












