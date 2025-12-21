/**
 * @file QualityChecker.h
 * @brief Quality and realism checking for generated items
 * @author Haneul Lee (Rundee)
 * @date 2025-11-17
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides quality checking functionality to validate generated items for
 * realism, balance, and consistency. Flags items with warnings or errors.
 */

#pragma once

#include <string>
#include <vector>

// Forward declarations
struct ItemWeaponData;
struct ItemWeaponComponentData;
struct ItemAmmoData;
struct ItemFoodData;
struct ItemDrinkData;
struct ItemMedicineData;
struct ItemMaterialData;

/**
 * @namespace QualityChecker
 * @brief Namespace for quality checking functions
 */
namespace QualityChecker
{
    /**
     * @struct QualityResult
     * @brief Result of a quality check operation
     * 
     * Contains validation results including validity status, warnings, errors,
     * and a quality score.
     */
    struct QualityResult
    {
        /**
         * @brief Whether the item is valid
         * 
         * If false, the item should be filtered out or regenerated.
         * Set to false when critical errors are found.
         */
        bool isValid = true;

        /**
         * @brief List of warning messages
         * 
         * Non-critical issues that don't prevent the item from being used.
         */
        std::vector<std::string> warnings;

        /**
         * @brief List of error messages
         * 
         * Critical issues that may cause the item to be invalid.
         */
        std::vector<std::string> errors;

        /**
         * @brief Quality score (0-100, higher is better)
         * 
         * Overall quality rating of the item. Used for filtering and ranking.
         */
        float qualityScore = 100.0f;
    };

    /**
     * @brief Check weapon item quality
     * 
     * Validates weapon items for realism, balance, and consistency.
     * Checks combat stats, weapon type compatibility, and attachment slots.
     * 
     * @param item Weapon item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckWeaponQuality(const ItemWeaponData& item);

    /**
     * @brief Check weapon component item quality
     * 
     * Validates weapon component items for compatibility and stat balance.
     * 
     * @param item Weapon component item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckWeaponComponentQuality(const ItemWeaponComponentData& item);

    /**
     * @brief Check ammo item quality
     * 
     * Validates ammo items for caliber consistency and stat balance.
     * 
     * @param item Ammo item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckAmmoQuality(const ItemAmmoData& item);

    /**
     * @brief Check food item quality
     * 
     * Validates food items for restoration values and spoilage logic.
     * 
     * @param item Food item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckFoodQuality(const ItemFoodData& item);

    /**
     * @brief Check drink item quality
     * 
     * Validates drink items for restoration values and spoilage logic.
     * 
     * @param item Drink item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckDrinkQuality(const ItemDrinkData& item);

    /**
     * @brief Check medicine item quality
     * 
     * Validates medicine items for health restoration and value balance.
     * 
     * @param item Medicine item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckMedicineQuality(const ItemMedicineData& item);

    /**
     * @brief Check material item quality
     * 
     * Validates material items for material properties and value balance.
     * 
     * @param item Material item to check
     * @return QualityResult with validation results
     */
    QualityResult CheckMaterialQuality(const ItemMaterialData& item);

    /**
     * @brief Print quality result to console
     * 
     * Outputs the quality check results in a human-readable format.
     * Shows warnings, errors, and quality score.
     * 
     * @param result QualityResult to print
     * @param itemId Item ID for identification in output
     */
    void PrintQualityResult(const QualityResult& result, const std::string& itemId);
}

