/**
 * @file ItemMedicineData.h
 * @brief Data structure for survival game medicine items
 * @author Haneul Lee (Rundee)
 * @date 2025-12-20
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for medicine items that restore health.
 * Inherits common item properties from ItemDataBase.
 */

#pragma once

#include "Data/ItemDataBase.h"

/**
 * @struct ItemMedicineData
 * @brief Data structure representing a medicine item
 * 
 * Contains all properties specific to medicine items, including health restoration
 * and spoilage information. Medicine items prioritize health restoration.
 */
struct ItemMedicineData : public ItemDataBase
{
    /**
     * @brief Health restoration value (0-100)
     * 
     * Amount of health restored when consumed. Medicine items typically have
     * higher healthRestore values than food or drink items.
     */
    int healthRestore = 0;

    /**
     * @brief Whether this medicine item spoils over time
     * 
     * If true, the item will spoil after spoilTimeMinutes.
     */
    bool spoils = false;

    /**
     * @brief Time until spoilage in minutes
     * 
     * Only used if spoils is true. Represents how long the item remains effective.
     */
    int spoilTimeMinutes = 0;

    /**
     * @brief Get the item type name
     * @return Always returns "Medicine"
     */
    std::string GetItemTypeName() const override { return "Medicine"; }
};

