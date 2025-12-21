/**
 * @file ItemDrinkData.h
 * @brief Data structure for survival game drink items
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for drink items that restore thirst, hunger, and health.
 * Inherits common item properties from ItemDataBase.
 */

#pragma once

#include "Data/ItemDataBase.h"

/**
 * @struct ItemDrinkData
 * @brief Data structure representing a drink item
 * 
 * Contains all properties specific to drink items, including restoration values
 * and spoilage information. Drink items prioritize thirst restoration over hunger.
 */
struct ItemDrinkData : public ItemDataBase
{
    /**
     * @brief Hunger restoration value (0-100)
     * 
     * Amount of hunger restored when consumed. Usually lower than thirstRestore
     * for drink items.
     */
    int hungerRestore = 0;

    /**
     * @brief Thirst restoration value (0-100)
     * 
     * Amount of thirst restored when consumed. Drink items typically have
     * higher thirstRestore than hungerRestore.
     */
    int thirstRestore = 0;

    /**
     * @brief Health restoration value (0-100)
     * 
     * Amount of health restored when consumed. Optional healing property.
     */
    int healthRestore = 0;

    /**
     * @brief Whether this drink item spoils over time
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
     * @return Always returns "Drink"
     */
    std::string GetItemTypeName() const override { return "Drink"; }
};












