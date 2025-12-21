/**
 * @file ItemArmorData.h
 * @brief Data structure for armor items
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for armor items that provide protection.
 * Inherits common item properties from ItemDataBase.
 */

#pragma once

#include "Data/ItemDataBase.h"

/**
 * @struct ItemArmorData
 * @brief Data structure representing an armor item
 * 
 * Contains all properties specific to armor items, including protection values
 * and durability information.
 */
struct ItemArmorData : public ItemDataBase
{
    /**
     * @brief Armor protection value (0-100)
     * 
     * Amount of damage reduction provided. Higher values indicate better protection.
     */
    int armorValue = 0;

    /**
     * @brief Armor durability (0-100)
     * 
     * Current condition of the armor. Decreases with use.
     */
    int durability = 100;

    /**
     * @brief Weight in grams
     * 
     * How much the armor weighs. Affects movement speed.
     */
    int weight = 0;

    /**
     * @brief Armor type
     * 
     * Examples: "Helmet", "Vest", "Plate", "Light", "Heavy"
     */
    std::string armorType;

    /**
     * @brief Item value
     * 
     * Base value for trading and economy.
     */
    int value = 0;

    /**
     * @brief Get the item type name
     * @return Always returns "Armor"
     */
    std::string GetItemTypeName() const override { return "Armor"; }
};





