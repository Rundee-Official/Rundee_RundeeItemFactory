/**
 * @file MedicineItemValidator.h
 * @brief Validation and balancing for medicine items
 * @author Haneul Lee (Rundee)
 * @date 2025-12-20
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides validation and balancing logic for medicine items.
 * Ensures medicine items have proper health restoration values,
 * spoilage settings, and rarity classification.
 */

#pragma once

#include "Data/ItemMedicineData.h"

/**
 * @class MedicineItemValidator
 * @brief Validates and balances medicine items
 * 
 * Ensures medicine items have proper health restoration values,
 * spoilage settings, and rarity classification.
 */
class MedicineItemValidator
{
public:
    /**
     * @brief Validate and balance a medicine item
     * 
     * Performs the following operations:
     * - Normalizes item ID prefix
     * - Clamps health restoration values
     * - Ensures medicine characteristics (healthRestore priority)
     * - Handles spoilage settings
     * - Balances rarity based on health restoration
     * - Ensures description is not empty
     * 
     * @param item Medicine item to validate (modified in-place)
     */
    static void Validate(ItemMedicineData& item);
};

