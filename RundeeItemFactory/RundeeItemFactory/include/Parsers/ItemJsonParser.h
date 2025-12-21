/**
 * @file ItemJsonParser.h
 * @brief Unified JSON parser for all item types
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * This class provides static methods to parse JSON text into item data structures
 * for all supported item types: Food, Drink, Material, Weapon, WeaponComponent, and Ammo.
 * Handles JSON cleaning, validation, and error reporting.
 */

#pragma once

#include <vector>
#include <string>
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMedicineData.h"
#include "Data/ItemMaterialData.h"
#include "Data/ItemWeaponData.h"
#include "Data/ItemWeaponComponentData.h"
#include "Data/ItemAmmoData.h"
#include "Data/ItemArmorData.h"
#include "Data/ItemClothingData.h"

/**
 * @class ItemJsonParser
 * @brief Static parser class for converting JSON text to item data structures
 * 
 * Provides type-specific parsing methods for each item type. All methods follow
 * the same pattern: take JSON text as input, parse into vector of item data,
 * and return success/failure status.
 */
class ItemJsonParser
{
public:
    /**
     * @brief Parse food items from JSON text
     * 
     * Parses a JSON array of food items from the provided JSON text string.
     * Handles JSON cleaning, validation, and error reporting.
     * 
     * @param jsonText JSON text string containing an array of food items
     * @param[out] outItems Vector to store parsed food items
     * @return true if parsing succeeded, false otherwise
     * 
     * @note The JSON text is automatically cleaned (trailing commas, whitespace)
     * @note Items with missing required fields (id, displayName) are skipped
     * @note Only items with category "Food" are parsed
     */
    static bool ParseFoodFromJsonText(const std::string& jsonText, std::vector<ItemFoodData>& outItems);

    /**
     * @brief Parse drink items from JSON text
     * 
     * @param jsonText JSON text string containing an array of drink items
     * @param[out] outItems Vector to store parsed drink items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseDrinkFromJsonText(const std::string& jsonText, std::vector<ItemDrinkData>& outItems);

    /**
     * @brief Parse medicine items from JSON text
     * 
     * @param jsonText JSON text string containing an array of medicine items
     * @param[out] outItems Vector to store parsed medicine items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseMedicineFromJsonText(const std::string& jsonText, std::vector<ItemMedicineData>& outItems);

    /**
     * @brief Parse material items from JSON text
     * 
     * @param jsonText JSON text string containing an array of material items
     * @param[out] outItems Vector to store parsed material items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseMaterialFromJsonText(const std::string& jsonText, std::vector<ItemMaterialData>& outItems);

    /**
     * @brief Parse weapon items from JSON text
     * 
     * @param jsonText JSON text string containing an array of weapon items
     * @param[out] outItems Vector to store parsed weapon items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseWeaponFromJsonText(const std::string& jsonText, std::vector<ItemWeaponData>& outItems);

    /**
     * @brief Parse weapon component items from JSON text
     * 
     * @param jsonText JSON text string containing an array of weapon component items
     * @param[out] outItems Vector to store parsed weapon component items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseWeaponComponentFromJsonText(const std::string& jsonText, std::vector<ItemWeaponComponentData>& outItems);

    /**
     * @brief Parse ammo items from JSON text
     * 
     * @param jsonText JSON text string containing an array of ammo items
     * @param[out] outItems Vector to store parsed ammo items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseAmmoFromJsonText(const std::string& jsonText, std::vector<ItemAmmoData>& outItems);

    /**
     * @brief Parse armor items from JSON text
     * 
     * @param jsonText JSON text string containing an array of armor items
     * @param[out] outItems Vector to store parsed armor items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseArmorFromJsonText(const std::string& jsonText, std::vector<ItemArmorData>& outItems);

    /**
     * @brief Parse clothing items from JSON text
     * 
     * @param jsonText JSON text string containing an array of clothing items
     * @param[out] outItems Vector to store parsed clothing items
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseClothingFromJsonText(const std::string& jsonText, std::vector<ItemClothingData>& outItems);
};


