// ===============================
// Project Name: RundeeItemFactory
// File Name: QualityChecker.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-17
// Description: Quality and realism checking for generated items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include <vector>

// Forward declarations
struct ItemWeaponData;
struct ItemWeaponComponentData;
struct ItemAmmoData;
struct ItemFoodData;
struct ItemDrinkData;
struct ItemMaterialData;

namespace QualityChecker
{
    // Quality check result
    struct QualityResult
    {
        bool isValid = true;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
        float qualityScore = 100.0f; // 0-100, higher is better
    };

    // ========================================================================
    // Quality Check Functions (by Item Type)
    // ========================================================================
    
    // Check weapon quality
    QualityResult CheckWeaponQuality(const ItemWeaponData& item);
    
    // Check weapon component quality
    QualityResult CheckWeaponComponentQuality(const ItemWeaponComponentData& item);
    
    // Check ammo quality
    QualityResult CheckAmmoQuality(const ItemAmmoData& item);
    
    // Check food quality
    QualityResult CheckFoodQuality(const ItemFoodData& item);
    
    // Check drink quality
    QualityResult CheckDrinkQuality(const ItemDrinkData& item);
    
    // Check material quality
    QualityResult CheckMaterialQuality(const ItemMaterialData& item);
    
    // ========================================================================
    // Utility Functions
    // ========================================================================
    
    // Print quality result
    void PrintQualityResult(const QualityResult& result, const std::string& itemId);
}

