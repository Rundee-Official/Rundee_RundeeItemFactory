/**
 * @file WeaponItemValidator.h
 * @brief Validation and balancing for ItemWeaponData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for weapon items, including:
 * - ID prefix normalization (ensures "Weapon_" prefix)
 * - Combat stat validation (damage, accuracy, range, etc.)
 * - Weapon type validation
 * - Rarity balancing based on combat effectiveness
 */

#pragma once

#include "Data/ItemWeaponData.h"

/**
 * @namespace WeaponItemValidator
 * @brief Namespace for weapon item validation functions
 */
namespace WeaponItemValidator
{
    /**
     * @brief Validate and balance weapon item data
     * 
     * Performs comprehensive validation and balancing on a weapon item:
     * - Normalizes ID prefix to "Weapon_"
     * - Clamps combat stats to valid ranges
     * - Validates weapon type and caliber compatibility
     * - Balances rarity based on combat effectiveness
     * - Generates default description if empty
     * 
     * @param[in,out] item Weapon item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Combat stats are balanced relative to each other
     */
    void Validate(ItemWeaponData& item);
}











