/**
 * @file ItemGenerateParams.h
 * @brief Parameters structure for item generation
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines parameters used for item generation, including generation count,
 * player stat maximums for balance calculations, and output file path.
 */

#pragma once

#include <string>

/**
 * @struct FoodGenerateParams
 * @brief Parameters for item generation requests
 * 
 * Contains all parameters needed to generate items, including:
 * - Generation count (number of items to generate)
 * - Player stat maximums (used for item balance calculations)
 * - Output file path
 * 
 * @note Despite the name "FoodGenerateParams", this structure is used
 *       for all item types, not just food items.
 */
struct FoodGenerateParams
{
    /**
     * @brief Number of items to generate
     * 
     * Default: 1
     */
    int count = 1;
    
    /**
     * @brief Maximum hunger value for player stats
     * 
     * Used for balancing item effects. Items that restore hunger
     * are balanced relative to this value.
     * 
     * Default: 100
     */
    int maxHunger = 100;
    
    /**
     * @brief Maximum thirst value for player stats
     * 
     * Used for balancing item effects. Items that restore thirst
     * are balanced relative to this value.
     * 
     * Default: 100
     */
    int maxThirst = 100;
    
    /**
     * @brief Maximum health value for player stats
     * 
     * Used for balancing item effects. Items that restore health
     * are balanced relative to this value.
     * 
     * Default: 100
     */
    int maxHealth = 100;
    
    /**
     * @brief Maximum stamina value for player stats
     * 
     * Used for balancing item effects. Items that affect stamina
     * are balanced relative to this value.
     * 
     * Default: 100
     */
    int maxStamina = 100;
    
    /**
     * @brief Maximum weight capacity for player inventory (in grams)
     * 
     * Used for balancing item weight values. Items are balanced
     * so their weights are reasonable relative to this capacity.
     * 
     * Default: 50000 (50 kg)
     */
    int maxWeight = 50000;  // grams (50kg default)
    
    /**
     * @brief Maximum energy value for player stats
     * 
     * Used for balancing item effects. Items that affect energy
     * are balanced relative to this value.
     * 
     * Default: 100
     */
    int maxEnergy = 100;
    
    /**
     * @brief Output file path for generated items
     * 
     * Path where the generated JSON items will be saved.
     * Can be relative (to executable directory) or absolute.
     * 
     * Default: "ItemJson/items_food.json"
     */
    std::string outputPath = "ItemJson/items_food.json";
};
