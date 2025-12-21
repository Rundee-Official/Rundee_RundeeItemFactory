/**
 * @file ItemJsonWriter.h
 * @brief Unified JSON writer for all item types
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides methods to write item data to JSON files and merge with existing files.
 * Supports all item types: Food, Drink, Material, Weapon, WeaponComponent, Ammo.
 */

#pragma once

#include <vector>
#include <string>
#include <set>
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
 * @class ItemJsonWriter
 * @brief Static class for writing item data to JSON files
 * 
 * Provides type-specific methods for writing and merging items to JSON files.
 * All methods handle file I/O, JSON serialization, and duplicate detection.
 */
class ItemJsonWriter
{
public:
    /**
     * @brief Write food items to JSON file (overwrites existing file)
     * @param items Vector of food items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteFoodToFile(const std::vector<ItemFoodData>& items, const std::string& path);

    /**
     * @brief Write drink items to JSON file (overwrites existing file)
     * @param items Vector of drink items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteDrinkToFile(const std::vector<ItemDrinkData>& items, const std::string& path);

    /**
     * @brief Write medicine items to JSON file (overwrites existing file)
     * @param items Vector of medicine items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteMedicineToFile(const std::vector<ItemMedicineData>& items, const std::string& path);

    /**
     * @brief Write material items to JSON file (overwrites existing file)
     * @param items Vector of material items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteMaterialToFile(const std::vector<ItemMaterialData>& items, const std::string& path);

    /**
     * @brief Write weapon items to JSON file (overwrites existing file)
     * @param items Vector of weapon items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteWeaponToFile(const std::vector<ItemWeaponData>& items, const std::string& path);

    /**
     * @brief Write weapon component items to JSON file (overwrites existing file)
     * @param items Vector of weapon component items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteWeaponComponentToFile(const std::vector<ItemWeaponComponentData>& items, const std::string& path);

    /**
     * @brief Write ammo items to JSON file (overwrites existing file)
     * @param items Vector of ammo items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteAmmoToFile(const std::vector<ItemAmmoData>& items, const std::string& path);

    /**
     * @brief Write armor items to JSON file (overwrites existing file)
     * @param items Vector of armor items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteArmorToFile(const std::vector<ItemArmorData>& items, const std::string& path);

    /**
     * @brief Write clothing items to JSON file (overwrites existing file)
     * @param items Vector of clothing items to write
     * @param path Output file path
     * @return true on success, false on failure
     */
    static bool WriteClothingToFile(const std::vector<ItemClothingData>& items, const std::string& path);

    /**
     * @brief Merge food items with existing file (keeps existing, adds new, skips duplicates by ID)
     * @param items Vector of new food items to merge
     * @param path JSON file path (created if doesn't exist)
     * @return true on success, false on failure
     */
    static bool MergeFoodToFile(const std::vector<ItemFoodData>& items, const std::string& path);

    /**
     * @brief Merge drink items with existing file
     * @param items Vector of new drink items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeDrinkToFile(const std::vector<ItemDrinkData>& items, const std::string& path);

    /**
     * @brief Merge medicine items with existing file
     * @param items Vector of new medicine items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeMedicineToFile(const std::vector<ItemMedicineData>& items, const std::string& path);

    /**
     * @brief Merge material items with existing file
     * @param items Vector of new material items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeMaterialToFile(const std::vector<ItemMaterialData>& items, const std::string& path);

    /**
     * @brief Merge weapon items with existing file
     * @param items Vector of new weapon items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeWeaponToFile(const std::vector<ItemWeaponData>& items, const std::string& path);

    /**
     * @brief Merge weapon component items with existing file
     * @param items Vector of new weapon component items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeWeaponComponentToFile(const std::vector<ItemWeaponComponentData>& items, const std::string& path);

    /**
     * @brief Merge ammo items with existing file
     * @param items Vector of new ammo items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeAmmoToFile(const std::vector<ItemAmmoData>& items, const std::string& path);

    /**
     * @brief Merge armor items with existing file
     * @param items Vector of new armor items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeArmorToFile(const std::vector<ItemArmorData>& items, const std::string& path);

    /**
     * @brief Merge clothing items with existing file
     * @param items Vector of new clothing items to merge
     * @param path JSON file path
     * @return true on success, false on failure
     */
    static bool MergeClothingToFile(const std::vector<ItemClothingData>& items, const std::string& path);

    /**
     * @brief Get existing food item IDs from file (for avoiding duplicates)
     * @param path JSON file path
     * @return Set of existing item IDs (empty set if file doesn't exist or is invalid)
     */
    static std::set<std::string> GetExistingFoodIds(const std::string& path);

    /**
     * @brief Get existing drink item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingDrinkIds(const std::string& path);

    /**
     * @brief Get existing medicine item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingMedicineIds(const std::string& path);

    /**
     * @brief Get existing material item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingMaterialIds(const std::string& path);

    /**
     * @brief Get existing weapon item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingWeaponIds(const std::string& path);

    /**
     * @brief Get existing weapon component item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingWeaponComponentIds(const std::string& path);

    /**
     * @brief Get existing ammo item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingAmmoIds(const std::string& path);

    /**
     * @brief Get existing armor item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingArmorIds(const std::string& path);

    /**
     * @brief Get existing clothing item IDs from file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingClothingIds(const std::string& path);
};


