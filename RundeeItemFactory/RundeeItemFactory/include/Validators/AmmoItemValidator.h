/**
 * @file AmmoItemValidator.h
 * @brief Validation for ItemAmmoData
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for ammo items, including:
 * - ID prefix normalization (ensures "Ammo_" prefix)
 * - Caliber validation
 * - Combat stat validation (damage bonus, penetration, accuracy, recoil)
 * - Special property validation (armor piercing, hollow point, tracer, incendiary)
 */

#pragma once

#include "Data/ItemAmmoData.h"

/**
 * @namespace AmmoItemValidator
 * @brief Namespace for ammo item validation functions
 */
namespace AmmoItemValidator
{
    /**
     * @brief Validate and balance ammo item data
     * 
     * Performs comprehensive validation and balancing on an ammo item:
     * - Normalizes ID prefix to "Ammo_"
     * - Validates caliber format
     * - Clamps combat stats to valid ranges (damage bonus, penetration, accuracy, recoil)
     * - Validates special properties
     * - Balances rarity based on combat effectiveness
     * - Generates default description if empty
     * 
     * @param[in,out] item Ammo item to validate and balance (modified in-place)
     * 
     * @note This function modifies the item parameter directly
     * @note Combat stats are balanced relative to each other
     */
    void Validate(ItemAmmoData& item);
}











