/**
 * @file WeaponComponentItemValidator.h
 * @brief Validation for ItemWeaponComponentData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for weapon component items, including:
 * - ID prefix normalization (ensures "WeaponComponent_" prefix)
 * - Component type validation (scope, grip, magazine, etc.)
 * - Stat modifier validation
 * - Compatibility validation
 */

#pragma once

#include "Data/ItemWeaponComponentData.h"

/**
 * @namespace WeaponComponentItemValidator
 * @brief Namespace for weapon component item validation functions
 */
namespace WeaponComponentItemValidator
{
    /**
     * @brief Validate and balance weapon component item data
     * 
     * Performs comprehensive validation and balancing on a weapon component item:
     * - Normalizes ID prefix to "WeaponComponent_"
     * - Validates component type
     * - Clamps stat modifiers to valid ranges
     * - Generates default description if empty
     * 
     * @param[in,out] item Weapon component item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     */
    void Validate(ItemWeaponComponentData& item);
}











