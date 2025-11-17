// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonWriter.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Unified JSON writer for all item types.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include <set>
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMaterialData.h"
#include "Data/ItemWeaponData.h"
#include "Data/ItemWeaponComponentData.h"
#include "Data/ItemAmmoData.h"

class ItemJsonWriter
{
public:
    // Write food items to JSON file (overwrites existing file)
    static bool WriteFoodToFile(const std::vector<ItemFoodData>& items, const std::string& path);

    // Write drink items to JSON file (overwrites existing file)
    static bool WriteDrinkToFile(const std::vector<ItemDrinkData>& items, const std::string& path);

    // Write material items to JSON file (overwrites existing file)
    static bool WriteMaterialToFile(const std::vector<ItemMaterialData>& items, const std::string& path);

    // Write weapon items to JSON file (overwrites existing file)
    static bool WriteWeaponToFile(const std::vector<ItemWeaponData>& items, const std::string& path);

    // Write weapon component items to JSON file (overwrites existing file)
    static bool WriteWeaponComponentToFile(const std::vector<ItemWeaponComponentData>& items, const std::string& path);

    // Write ammo items to JSON file (overwrites existing file)
    static bool WriteAmmoToFile(const std::vector<ItemAmmoData>& items, const std::string& path);

    // Merge items with existing file (keeps existing items, adds new ones, skips duplicates by ID)
    static bool MergeFoodToFile(const std::vector<ItemFoodData>& items, const std::string& path);
    static bool MergeDrinkToFile(const std::vector<ItemDrinkData>& items, const std::string& path);
    static bool MergeMaterialToFile(const std::vector<ItemMaterialData>& items, const std::string& path);
    static bool MergeWeaponToFile(const std::vector<ItemWeaponData>& items, const std::string& path);
    static bool MergeWeaponComponentToFile(const std::vector<ItemWeaponComponentData>& items, const std::string& path);
    static bool MergeAmmoToFile(const std::vector<ItemAmmoData>& items, const std::string& path);

    // Get existing item IDs from file (for avoiding duplicates)
    static std::set<std::string> GetExistingFoodIds(const std::string& path);
    static std::set<std::string> GetExistingDrinkIds(const std::string& path);
    static std::set<std::string> GetExistingMaterialIds(const std::string& path);
    static std::set<std::string> GetExistingWeaponIds(const std::string& path);
    static std::set<std::string> GetExistingWeaponComponentIds(const std::string& path);
    static std::set<std::string> GetExistingAmmoIds(const std::string& path);
};


