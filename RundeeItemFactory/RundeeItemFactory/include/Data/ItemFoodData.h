/**
 * @file ItemFoodData.h
 * @brief Data structure for survival game food items
 * @author Haneul Lee (Rundee)
 * @date 2025-11-14
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for food items that restore hunger, thirst, and health.
 * Inherits common item properties from ItemDataBase.
 */

#pragma once

#include "Data/ItemDataBase.h"

/**
 * @struct ItemFoodData
 * @brief Data structure representing a food item
 * 
 * Contains all properties specific to food items, including restoration values
 * and spoilage information. Food items prioritize hunger restoration over thirst.
 */
struct ItemFoodData : public ItemDataBase
{
    /**
     * @brief Hunger restoration value (0-100)
     * 
     * Amount of hunger restored when consumed. Food items typically have
     * higher hungerRestore than thirstRestore.
     */
    int hungerRestore = 0;

    /**
     * @brief Thirst restoration value (0-100)
     * 
     * Amount of thirst restored when consumed. Usually lower than hungerRestore
     * for food items.
     */
    int thirstRestore = 0;

    /**
     * @brief Health restoration value (0-100)
     * 
     * Amount of health restored when consumed. Optional healing property.
     */
    int healthRestore = 0;

    /**
     * @brief Whether this food item spoils over time
     * 
     * If true, the item will spoil after spoilTimeMinutes.
     */
    bool spoils = false;

    /**
     * @brief Time until spoilage in minutes
     * 
     * Only used if spoils is true. Represents how long the item remains fresh.
     */
    int spoilTimeMinutes = 0;

    /**
     * @brief Get the item type name
     * @return Always returns "Food"
     */
    std::string GetItemTypeName() const override { return "Food"; }
};












