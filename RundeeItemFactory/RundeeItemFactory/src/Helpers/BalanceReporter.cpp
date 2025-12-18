// ===============================
// Project Name: RundeeItemFactory
// File Name: BalanceReporter.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of balance report generator.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

// Project Includes
#include "Helpers/BalanceReporter.h"
#include "Data/ItemAmmoData.h"
#include "Data/ItemArmorData.h"
#include "Data/ItemClothingData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemFoodData.h"
#include "Data/ItemMaterialData.h"
#include "Data/ItemWeaponComponentData.h"
#include "Data/ItemWeaponData.h"
#include "Parsers/ItemJsonParser.h"

// ============================================================================
// SECTION 1: BalanceReporter Namespace - Internal Data Structures
// ============================================================================

namespace BalanceReporter
{
    struct FoodStats
    {
        int count = 0;
        int totalHunger = 0;
        int totalThirst = 0;
        int totalHealth = 0;
        int minHunger = 1000;
        int maxHunger = -1;
        int minThirst = 1000;
        int maxThirst = -1;
        int minHealth = 1000;
        int maxHealth = -1;
        int spoilingCount = 0;
        std::map<std::string, int> rarityCount;
    };

    struct MaterialStats
    {
        int count = 0;
        int totalHardness = 0;
        int totalFlammability = 0;
        int totalValue = 0;
        int minHardness = 1000;
        int maxHardness = -1;
        int minFlammability = 1000;
        int maxFlammability = -1;
        int minValue = 1000;
        int maxValue = -1;
        std::map<std::string, int> rarityCount;
        std::map<std::string, int> materialTypeCount;
    };

    struct WeaponStats
    {
        int count = 0;
        int totalMinDamage = 0;
        int totalMaxDamage = 0;
        int totalFireRate = 0;
        int totalAccuracy = 0;
        int totalRecoil = 0;
        int totalErgonomics = 0;
        int totalWeight = 0;
        int totalMuzzleVelocity = 0;
        int totalEffectiveRange = 0;
        int totalPenetrationPower = 0;
        int totalModdingSlots = 0;
        int minMinDamage = 1000;
        int maxMinDamage = -1;
        int minMaxDamage = 1000;
        int maxMaxDamage = -1;
        int minFireRate = 10000;
        int maxFireRate = -1;
        int minAccuracy = 1000;
        int maxAccuracy = -1;
        int minRecoil = 1000;
        int maxRecoil = -1;
        int minErgonomics = 1000;
        int maxErgonomics = -1;
        int minWeight = 100000;
        int maxWeight = -1;
        int minMuzzleVelocity = 10000;
        int maxMuzzleVelocity = -1;
        int minEffectiveRange = 10000;
        int maxEffectiveRange = -1;
        int minPenetrationPower = 1000;
        int maxPenetrationPower = -1;
        int minModdingSlots = 1000;
        int maxModdingSlots = -1;
        std::map<std::string, int> rarityCount;
        std::map<std::string, int> weaponTypeCount;
        std::map<std::string, int> weaponCategoryCount;
        std::map<std::string, int> caliberCount;
    };

    struct WeaponComponentStats
    {
        int count = 0;
        int totalDamageModifier = 0;
        int totalRecoilModifier = 0;
        int totalErgonomicsModifier = 0;
        int totalAccuracyModifier = 0;
        int totalWeightModifier = 0;
        int totalMuzzleVelocityModifier = 0;
        int totalEffectiveRangeModifier = 0;
        int totalPenetrationModifier = 0;
        int minDamageModifier = 1000;
        int maxDamageModifier = -1000;
        int minRecoilModifier = 1000;
        int maxRecoilModifier = -1000;
        int minErgonomicsModifier = 1000;
        int maxErgonomicsModifier = -1000;
        int minAccuracyModifier = 1000;
        int maxAccuracyModifier = -1000;
        int minWeightModifier = 100000;
        int maxWeightModifier = -100000;
        int minMuzzleVelocityModifier = 10000;
        int maxMuzzleVelocityModifier = -10000;
        int minEffectiveRangeModifier = 10000;
        int maxEffectiveRangeModifier = -10000;
        int minPenetrationModifier = 1000;
        int maxPenetrationModifier = -1000;
        int magazineCount = 0;
        int totalMagazineCapacity = 0;
        int minMagazineCapacity = 10000;
        int maxMagazineCapacity = -1;
        int magazinesWithLoadPlan = 0;
        int mixedAmmoMagazineCount = 0;
        int totalPlannedRounds = 0;
        std::map<std::string, int> rarityCount;
        std::map<std::string, int> componentTypeCount;
        std::map<std::string, int> magazineTypeCount;
        std::map<std::string, int> caliberCount;
    };

    struct AmmoStats
    {
        int count = 0;
        int totalDamageBonus = 0;
        int totalPenetration = 0;
        int totalAccuracyBonus = 0;
        int totalRecoilModifier = 0;
        int totalValue = 0;
        int minDamageBonus = 1000;
        int maxDamageBonus = -1000;
        int minPenetration = 1000;
        int maxPenetration = -1;
        int minAccuracyBonus = 1000;
        int maxAccuracyBonus = -1000;
        int minRecoilModifier = 1000;
        int maxRecoilModifier = -1000;
        int minValue = 1000;
        int maxValue = -1;
        int armorPiercingCount = 0;
        int hollowPointCount = 0;
        int tracerCount = 0;
        int incendiaryCount = 0;
        std::map<std::string, int> rarityCount;
        std::map<std::string, int> caliberCount;
    };

    struct ArmorStats
    {
        int count = 0;
        int totalArmorClass = 0;
        int totalDurability = 0;
        int totalMaterial = 0;
        int totalMovementSpeedPenalty = 0;
        int totalErgonomicsPenalty = 0;
        int totalTurnSpeedPenalty = 0;
        int totalWeight = 0;
        int totalCapacity = 0;
        int minArmorClass = 100;
        int maxArmorClass = -1;
        int minDurability = 100;
        int maxDurability = -1;
        int minMaterial = 100;
        int maxMaterial = -1;
        int minMovementSpeedPenalty = 100;
        int maxMovementSpeedPenalty = -1;
        int minErgonomicsPenalty = 100;
        int maxErgonomicsPenalty = -1;
        int minTurnSpeedPenalty = 100;
        int maxTurnSpeedPenalty = -1;
        int minWeight = 100000;
        int maxWeight = -1;
        int minCapacity = 10000;
        int maxCapacity = -1;
        int blocksHeadsetCount = 0;
        int blocksFaceCoverCount = 0;
        std::map<std::string, int> rarityCount;
        std::map<std::string, int> armorTypeCount;
    };

    struct ClothingStats
    {
        int count = 0;
        int totalColdResistance = 0;
        int totalHeatResistance = 0;
        int totalWaterResistance = 0;
        int totalWindResistance = 0;
        int totalComfort = 0;
        int totalMobilityBonus = 0;
        int totalStaminaBonus = 0;
        int totalDurability = 0;
        int totalMaterial = 0;
        int totalWeight = 0;
        int minColdResistance = 100;
        int maxColdResistance = -1;
        int minHeatResistance = 100;
        int maxHeatResistance = -1;
        int minWaterResistance = 100;
        int maxWaterResistance = -1;
        int minWindResistance = 100;
        int maxWindResistance = -1;
        int minComfort = 100;
        int maxComfort = -1;
        int minMobilityBonus = 100;
        int maxMobilityBonus = -100;
        int minStaminaBonus = 100;
        int maxStaminaBonus = -100;
        int minDurability = 100;
        int maxDurability = -1;
        int minMaterial = 100;
        int maxMaterial = -1;
        int minWeight = 100000;
        int maxWeight = -1;
        int isInsulatedCount = 0;
        int isWaterproofCount = 0;
        int isWindproofCount = 0;
        std::map<std::string, int> rarityCount;
        std::map<std::string, int> clothingTypeCount;
    };

    // ============================================================================
    // SECTION 2: Report Printing Functions (by Item Type - Alphabetical Order)
    // ============================================================================

    static void PrintFoodReport(const std::vector<ItemFoodData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No food items to analyze.\n";
            return;
        }

        FoodStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalHunger += item.hungerRestore;
            stats.totalThirst += item.thirstRestore;
            stats.totalHealth += item.healthRestore;

            stats.minHunger = std::min(stats.minHunger, item.hungerRestore);
            stats.maxHunger = std::max(stats.maxHunger, item.hungerRestore);
            stats.minThirst = std::min(stats.minThirst, item.thirstRestore);
            stats.maxThirst = std::max(stats.maxThirst, item.thirstRestore);
            stats.minHealth = std::min(stats.minHealth, item.healthRestore);
            stats.maxHealth = std::max(stats.maxHealth, item.healthRestore);

            if (item.spoils)
                stats.spoilingCount++;

            stats.rarityCount[item.rarity]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   FOOD ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        // Quick Summary
        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Hunger Restore: " << (stats.totalHunger / stats.count) << "\n";
        std::cout << "  Average Thirst Restore: " << (stats.totalThirst / stats.count) << "\n";
        std::cout << "  Average Health Restore: " << (stats.totalHealth / stats.count) << "\n";
        float avgPower = (stats.totalHunger + stats.totalThirst + stats.totalHealth) / (float)stats.count;
        std::cout << "  Average Total Power: " << std::fixed << std::setprecision(1) << avgPower << "\n";
        std::cout << "  Spoiling Items: " << stats.spoilingCount << " (" 
            << (stats.spoilingCount * 100.0f / stats.count) << "%)\n";
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Hunger Restore Stats ---\n";
        std::cout << "  Average: " << (stats.totalHunger / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minHunger << "\n";
        std::cout << "  Max:     " << stats.maxHunger << "\n";

        std::cout << "\n--- Thirst Restore Stats ---\n";
        std::cout << "  Average: " << (stats.totalThirst / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minThirst << "\n";
        std::cout << "  Max:     " << stats.maxThirst << "\n";

        std::cout << "\n--- Health Restore Stats ---\n";
        std::cout << "  Average: " << (stats.totalHealth / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minHealth << "\n";
        std::cout << "  Max:     " << stats.maxHealth << "\n";

        std::cout << "\n--- Spoilage ---\n";
        std::cout << "  Items that spoil: " << stats.spoilingCount
            << " (" << (stats.spoilingCount * 100.0f / stats.count) << "%)\n";

        // Balance warnings
        std::cout << "\n--- Balance Warnings ---\n";
        bool hasWarnings = false;

        if (stats.maxHunger > 80)
        {
            std::cout << "  [WARNING] High hunger restore detected (max: " << stats.maxHunger << ")\n";
            hasWarnings = true;
        }
        if (stats.maxThirst > 80)
        {
            std::cout << "  [WARNING] High thirst restore detected (max: " << stats.maxThirst << ")\n";
            hasWarnings = true;
        }
        if (stats.maxHealth > 50)
        {
            std::cout << "  [WARNING] High health restore detected (max: " << stats.maxHealth << ")\n";
            hasWarnings = true;
        }

        if (avgPower > 60)
        {
            std::cout << "  [WARNING] High average power detected (avg: " << std::fixed << std::setprecision(1) << avgPower << ")\n";
            hasWarnings = true;
        }

        if (!hasWarnings)
        {
            std::cout << "  [OK] No balance issues detected.\n";
        }

        std::cout << "========================================\n\n";
    }

    static void PrintDrinkReport(const std::vector<ItemDrinkData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No drink items to analyze.\n";
            return;
        }

        FoodStats stats; // Reuse FoodStats structure
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalHunger += item.hungerRestore;
            stats.totalThirst += item.thirstRestore;
            stats.totalHealth += item.healthRestore;

            stats.minHunger = std::min(stats.minHunger, item.hungerRestore);
            stats.maxHunger = std::max(stats.maxHunger, item.hungerRestore);
            stats.minThirst = std::min(stats.minThirst, item.thirstRestore);
            stats.maxThirst = std::max(stats.maxThirst, item.thirstRestore);
            stats.minHealth = std::min(stats.minHealth, item.healthRestore);
            stats.maxHealth = std::max(stats.maxHealth, item.healthRestore);

            if (item.spoils)
                stats.spoilingCount++;

            stats.rarityCount[item.rarity]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   DRINK ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        // Quick Summary
        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Hunger Restore: " << (stats.totalHunger / stats.count) << "\n";
        std::cout << "  Average Thirst Restore: " << (stats.totalThirst / stats.count) << "\n";
        std::cout << "  Average Health Restore: " << (stats.totalHealth / stats.count) << "\n";
        float avgPower = (stats.totalHunger + stats.totalThirst + stats.totalHealth) / (float)stats.count;
        std::cout << "  Average Total Power: " << std::fixed << std::setprecision(1) << avgPower << "\n";
        std::cout << "  Spoiling Items: " << stats.spoilingCount << " (" 
            << (stats.spoilingCount * 100.0f / stats.count) << "%)\n";
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Hunger Restore Stats ---\n";
        std::cout << "  Average: " << (stats.totalHunger / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minHunger << "\n";
        std::cout << "  Max:     " << stats.maxHunger << "\n";

        std::cout << "\n--- Thirst Restore Stats ---\n";
        std::cout << "  Average: " << (stats.totalThirst / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minThirst << "\n";
        std::cout << "  Max:     " << stats.maxThirst << "\n";

        std::cout << "\n--- Health Restore Stats ---\n";
        std::cout << "  Average: " << (stats.totalHealth / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minHealth << "\n";
        std::cout << "  Max:     " << stats.maxHealth << "\n";

        std::cout << "\n--- Spoilage ---\n";
        std::cout << "  Items that spoil: " << stats.spoilingCount
            << " (" << (stats.spoilingCount * 100.0f / stats.count) << "%)\n";

        // Balance warnings
        std::cout << "\n--- Balance Warnings ---\n";
        bool hasWarnings = false;

        if (stats.maxThirst < 15)
        {
            std::cout << "  [WARNING] Low thirst restore for drinks (max: " << stats.maxThirst << ")\n";
            hasWarnings = true;
        }
        if (stats.maxHunger > 20)
        {
            std::cout << "  [WARNING] High hunger restore for drinks (max: " << stats.maxHunger << ")\n";
            hasWarnings = true;
        }

        if (!hasWarnings)
        {
            std::cout << "  [OK] No balance issues detected.\n";
        }

        std::cout << "========================================\n\n";
    }

    static void PrintMaterialReport(const std::vector<ItemMaterialData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No material items to analyze.\n";
            return;
        }

        MaterialStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalHardness += item.hardness;
            stats.totalFlammability += item.flammability;
            stats.totalValue += item.value;

            stats.minHardness = std::min(stats.minHardness, item.hardness);
            stats.maxHardness = std::max(stats.maxHardness, item.hardness);
            stats.minFlammability = std::min(stats.minFlammability, item.flammability);
            stats.maxFlammability = std::max(stats.maxFlammability, item.flammability);
            stats.minValue = std::min(stats.minValue, item.value);
            stats.maxValue = std::max(stats.maxValue, item.value);

            stats.rarityCount[item.rarity]++;
            stats.materialTypeCount[item.materialType]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   MATERIAL ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        // Quick Summary
        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Hardness: " << (stats.totalHardness / stats.count) << "/100\n";
        std::cout << "  Average Flammability: " << (stats.totalFlammability / stats.count) << "/100\n";
        std::cout << "  Average Value: " << (stats.totalValue / stats.count) << "\n";
        std::cout << "  Material Types: " << stats.materialTypeCount.size() << " unique types\n";
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Material Type Distribution ---\n";
        for (const auto& pair : stats.materialTypeCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(15) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Hardness Stats ---\n";
        std::cout << "  Average: " << (stats.totalHardness / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minHardness << "\n";
        std::cout << "  Max:     " << stats.maxHardness << "\n";

        std::cout << "\n--- Flammability Stats ---\n";
        std::cout << "  Average: " << (stats.totalFlammability / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minFlammability << "\n";
        std::cout << "  Max:     " << stats.maxFlammability << "\n";

        std::cout << "\n--- Value Stats ---\n";
        std::cout << "  Average: " << (stats.totalValue / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minValue << "\n";
        std::cout << "  Max:     " << stats.maxValue << "\n";

        // Balance warnings
        std::cout << "\n--- Balance Warnings ---\n";
        bool hasWarnings = false;

        if (stats.maxValue > 90)
        {
            std::cout << "  [WARNING] Very high value detected (max: " << stats.maxValue << ")\n";
            hasWarnings = true;
        }

        if (!hasWarnings)
        {
            std::cout << "  [OK] No balance issues detected.\n";
        }

        std::cout << "========================================\n\n";
    }

    static void PrintWeaponReport(const std::vector<ItemWeaponData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No weapon items to analyze.\n";
            return;
        }

        WeaponStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalMinDamage += item.minDamage;
            stats.totalMaxDamage += item.maxDamage;
            stats.totalFireRate += item.fireRate;
            stats.totalAccuracy += item.accuracy;
            stats.totalRecoil += item.recoil;
            stats.totalErgonomics += item.ergonomics;
            stats.totalWeight += item.weight;
            stats.totalMuzzleVelocity += item.muzzleVelocity;
            stats.totalEffectiveRange += item.effectiveRange;
            stats.totalPenetrationPower += item.penetrationPower;
            stats.totalModdingSlots += item.moddingSlots;

            stats.minMinDamage = std::min(stats.minMinDamage, item.minDamage);
            stats.maxMinDamage = std::max(stats.maxMinDamage, item.minDamage);
            stats.minMaxDamage = std::min(stats.minMaxDamage, item.maxDamage);
            stats.maxMaxDamage = std::max(stats.maxMaxDamage, item.maxDamage);
            stats.minFireRate = std::min(stats.minFireRate, item.fireRate);
            stats.maxFireRate = std::max(stats.maxFireRate, item.fireRate);
            stats.minAccuracy = std::min(stats.minAccuracy, item.accuracy);
            stats.maxAccuracy = std::max(stats.maxAccuracy, item.accuracy);
            stats.minRecoil = std::min(stats.minRecoil, item.recoil);
            stats.maxRecoil = std::max(stats.maxRecoil, item.recoil);
            stats.minErgonomics = std::min(stats.minErgonomics, item.ergonomics);
            stats.maxErgonomics = std::max(stats.maxErgonomics, item.ergonomics);
            stats.minWeight = std::min(stats.minWeight, item.weight);
            stats.maxWeight = std::max(stats.maxWeight, item.weight);
            stats.minMuzzleVelocity = std::min(stats.minMuzzleVelocity, item.muzzleVelocity);
            stats.maxMuzzleVelocity = std::max(stats.maxMuzzleVelocity, item.muzzleVelocity);
            stats.minEffectiveRange = std::min(stats.minEffectiveRange, item.effectiveRange);
            stats.maxEffectiveRange = std::max(stats.maxEffectiveRange, item.effectiveRange);
            stats.minPenetrationPower = std::min(stats.minPenetrationPower, item.penetrationPower);
            stats.maxPenetrationPower = std::max(stats.maxPenetrationPower, item.penetrationPower);
            stats.minModdingSlots = std::min(stats.minModdingSlots, item.moddingSlots);
            stats.maxModdingSlots = std::max(stats.maxModdingSlots, item.moddingSlots);

            stats.rarityCount[item.rarity]++;
            stats.weaponTypeCount[item.weaponType]++;
            stats.weaponCategoryCount[item.weaponCategory]++;
            if (!item.caliber.empty())
            {
                stats.caliberCount[item.caliber]++;
            }
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   WEAPON ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        // Quick Summary
        std::cout << "--- Quick Summary ---\n";
        float avgDamage = (stats.totalMinDamage + stats.totalMaxDamage) / (2.0f * stats.count);
        std::cout << "  Average Damage: " << std::fixed << std::setprecision(1) << avgDamage << "\n";
        std::cout << "  Average Fire Rate: " << (stats.totalFireRate / stats.count) << " RPM\n";
        std::cout << "  Average Accuracy: " << (stats.totalAccuracy / stats.count) << "/100\n";
        std::cout << "  Average Weight: " << (stats.totalWeight / stats.count) << "g (" 
            << std::fixed << std::setprecision(1) << ((stats.totalWeight / stats.count) / 1000.0f) << " kg)\n";
        
        // Ranged vs Melee breakdown
        int rangedCount = 0, meleeCount = 0;
        for (const auto& pair : stats.weaponCategoryCount)
        {
            if (pair.first == "Ranged") rangedCount = pair.second;
            else if (pair.first == "Melee") meleeCount = pair.second;
        }
        if (rangedCount > 0 || meleeCount > 0)
        {
            std::cout << "  Weapon Categories: " << rangedCount << " Ranged, " << meleeCount << " Melee\n";
        }
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Weapon Category Distribution ---\n";
        for (const auto& pair : stats.weaponCategoryCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Weapon Type Distribution ---\n";
        for (const auto& pair : stats.weaponTypeCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(15) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        if (!stats.caliberCount.empty())
        {
            std::cout << "\n--- Caliber Distribution (Ranged Only) ---\n";
            for (const auto& pair : stats.caliberCount)
            {
                float percentage = (pair.second * 100.0f) / stats.caliberCount.size();
                std::cout << "  " << std::setw(10) << std::left << pair.first
                    << ": " << std::setw(3) << pair.second
                    << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
            }
        }

        std::cout << "\n--- Damage Stats ---\n";
        std::cout << "  Min Damage - Average: " << (stats.totalMinDamage / stats.count)
            << ", Min: " << stats.minMinDamage << ", Max: " << stats.maxMinDamage << "\n";
        std::cout << "  Max Damage - Average: " << (stats.totalMaxDamage / stats.count)
            << ", Min: " << stats.minMaxDamage << ", Max: " << stats.maxMaxDamage << "\n";

        std::cout << "\n--- Fire Rate Stats ---\n";
        std::cout << "  Average: " << (stats.totalFireRate / stats.count) << " RPM\n";
        std::cout << "  Min:     " << stats.minFireRate << " RPM\n";
        std::cout << "  Max:     " << stats.maxFireRate << " RPM\n";

        std::cout << "\n--- Accuracy Stats ---\n";
        std::cout << "  Average: " << (stats.totalAccuracy / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minAccuracy << "\n";
        std::cout << "  Max:     " << stats.maxAccuracy << "\n";

        std::cout << "\n--- Recoil Stats ---\n";
        std::cout << "  Average: " << (stats.totalRecoil / stats.count) << " (lower is better)\n";
        std::cout << "  Min:     " << stats.minRecoil << "\n";
        std::cout << "  Max:     " << stats.maxRecoil << "\n";

        std::cout << "\n--- Ergonomics Stats ---\n";
        std::cout << "  Average: " << (stats.totalErgonomics / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minErgonomics << "\n";
        std::cout << "  Max:     " << stats.maxErgonomics << "\n";

        std::cout << "\n--- Weight Stats ---\n";
        std::cout << "  Average: " << (stats.totalWeight / stats.count) << " grams\n";
        std::cout << "  Min:     " << stats.minWeight << " grams\n";
        std::cout << "  Max:     " << stats.maxWeight << " grams\n";

        std::cout << "\n--- Muzzle Velocity Stats ---\n";
        std::cout << "  Average: " << (stats.totalMuzzleVelocity / stats.count) << " m/s\n";
        std::cout << "  Min:     " << stats.minMuzzleVelocity << " m/s\n";
        std::cout << "  Max:     " << stats.maxMuzzleVelocity << " m/s\n";

        std::cout << "\n--- Effective Range Stats ---\n";
        std::cout << "  Average: " << (stats.totalEffectiveRange / stats.count) << " meters\n";
        std::cout << "  Min:     " << stats.minEffectiveRange << " meters\n";
        std::cout << "  Max:     " << stats.maxEffectiveRange << " meters\n";

        std::cout << "\n--- Penetration Power Stats ---\n";
        std::cout << "  Average: " << (stats.totalPenetrationPower / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minPenetrationPower << "\n";
        std::cout << "  Max:     " << stats.maxPenetrationPower << "\n";

        std::cout << "\n--- Modding Slots Stats ---\n";
        std::cout << "  Average: " << (stats.totalModdingSlots / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minModdingSlots << "\n";
        std::cout << "  Max:     " << stats.maxModdingSlots << "\n";

        // Balance warnings
        std::cout << "\n--- Balance Warnings ---\n";
        bool hasWarnings = false;

        if (stats.maxMaxDamage > 90)
        {
            std::cout << "  [WARNING] Very high damage detected (max: " << stats.maxMaxDamage << ")\n";
            hasWarnings = true;
        }
        if (stats.maxFireRate > 1000)
        {
            std::cout << "  [WARNING] Very high fire rate detected (max: " << stats.maxFireRate << " RPM)\n";
            hasWarnings = true;
        }
        if (stats.maxRecoil > 80)
        {
            std::cout << "  [WARNING] Very high recoil detected (max: " << stats.maxRecoil << ")\n";
            hasWarnings = true;
        }
        if (stats.maxWeight > 5000)
        {
            std::cout << "  [WARNING] Very heavy weapons detected (max: " << stats.maxWeight << " grams)\n";
            hasWarnings = true;
        }

        if (!hasWarnings)
        {
            std::cout << "  [OK] No balance issues detected.\n";
        }

        std::cout << "========================================\n\n";
    }

    static void PrintWeaponComponentReport(const std::vector<ItemWeaponComponentData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No weapon component items to analyze.\n";
            return;
        }

        WeaponComponentStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalDamageModifier += item.damageModifier;
            stats.totalRecoilModifier += item.recoilModifier;
            stats.totalErgonomicsModifier += item.ergonomicsModifier;
            stats.totalAccuracyModifier += item.accuracyModifier;
            stats.totalWeightModifier += item.weightModifier;
            stats.totalMuzzleVelocityModifier += item.muzzleVelocityModifier;
            stats.totalEffectiveRangeModifier += item.effectiveRangeModifier;
            stats.totalPenetrationModifier += item.penetrationModifier;

            stats.minDamageModifier = std::min(stats.minDamageModifier, item.damageModifier);
            stats.maxDamageModifier = std::max(stats.maxDamageModifier, item.damageModifier);
            stats.minRecoilModifier = std::min(stats.minRecoilModifier, item.recoilModifier);
            stats.maxRecoilModifier = std::max(stats.maxRecoilModifier, item.recoilModifier);
            stats.minErgonomicsModifier = std::min(stats.minErgonomicsModifier, item.ergonomicsModifier);
            stats.maxErgonomicsModifier = std::max(stats.maxErgonomicsModifier, item.ergonomicsModifier);
            stats.minAccuracyModifier = std::min(stats.minAccuracyModifier, item.accuracyModifier);
            stats.maxAccuracyModifier = std::max(stats.maxAccuracyModifier, item.accuracyModifier);
            stats.minWeightModifier = std::min(stats.minWeightModifier, item.weightModifier);
            stats.maxWeightModifier = std::max(stats.maxWeightModifier, item.weightModifier);
            stats.minMuzzleVelocityModifier = std::min(stats.minMuzzleVelocityModifier, item.muzzleVelocityModifier);
            stats.maxMuzzleVelocityModifier = std::max(stats.maxMuzzleVelocityModifier, item.muzzleVelocityModifier);
            stats.minEffectiveRangeModifier = std::min(stats.minEffectiveRangeModifier, item.effectiveRangeModifier);
            stats.maxEffectiveRangeModifier = std::max(stats.maxEffectiveRangeModifier, item.effectiveRangeModifier);
            stats.minPenetrationModifier = std::min(stats.minPenetrationModifier, item.penetrationModifier);
            stats.maxPenetrationModifier = std::max(stats.maxPenetrationModifier, item.penetrationModifier);

            if (item.componentType == "Magazine" || item.componentType == "magazine")
            {
                stats.magazineCount++;
                stats.totalMagazineCapacity += item.magazineCapacity;
                stats.minMagazineCapacity = std::min(stats.minMagazineCapacity, item.magazineCapacity);
                stats.maxMagazineCapacity = std::max(stats.maxMagazineCapacity, item.magazineCapacity);
                stats.magazineTypeCount[item.magazineType]++;
                stats.caliberCount[item.caliber]++;

                if (!item.loadedRounds.empty())
                {
                    stats.magazinesWithLoadPlan++;
                    int plannedRounds = 0;
                    std::string firstAmmoId;
                    bool hasMixed = false;
                    for (const auto& segment : item.loadedRounds)
                    {
                        plannedRounds += segment.roundCount;
                        if (firstAmmoId.empty() && !segment.ammoId.empty())
                        {
                            firstAmmoId = segment.ammoId;
                        }
                        else if (!segment.ammoId.empty() && segment.ammoId != firstAmmoId)
                        {
                            hasMixed = true;
                        }
                    }
                    stats.totalPlannedRounds += plannedRounds;
                    if (hasMixed)
                        stats.mixedAmmoMagazineCount++;
                }
            }

            stats.rarityCount[item.rarity]++;
            stats.componentTypeCount[item.componentType]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   WEAPON COMPONENT ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        // Quick Summary
        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Damage Modifier: " << std::fixed << std::setprecision(1) 
            << (stats.totalDamageModifier / (float)stats.count) << "\n";
        std::cout << "  Average Recoil Reduction: " << (stats.totalRecoilModifier / stats.count) << "\n";
        std::cout << "  Average Ergonomics Bonus: " << (stats.totalErgonomicsModifier / stats.count) << "\n";
        std::cout << "  Average Accuracy Bonus: " << (stats.totalAccuracyModifier / stats.count) << "\n";
        std::cout << "  Average Weight Change: " << (stats.totalWeightModifier / stats.count) << "g\n";
        
        if (stats.magazineCount > 0)
        {
            std::cout << "  Magazines: " << stats.magazineCount << " (" 
                << (stats.magazineCount * 100.0f / stats.count) << "% of all components)\n";
            std::cout << "  Average Magazine Capacity: " << (stats.totalMagazineCapacity / stats.magazineCount) << " rounds\n";
        }
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Component Type Distribution ---\n";
        for (const auto& pair : stats.componentTypeCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(15) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        if (stats.magazineCount > 0)
        {
            std::cout << "\n--- Magazine Type Distribution ---\n";
            for (const auto& pair : stats.magazineTypeCount)
            {
                float percentage = (pair.second * 100.0f) / stats.magazineCount;
                std::cout << "  " << std::setw(15) << std::left << pair.first
                    << ": " << std::setw(3) << pair.second
                    << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
            }

            std::cout << "\n--- Magazine Caliber Distribution ---\n";
            for (const auto& pair : stats.caliberCount)
            {
                float percentage = (pair.second * 100.0f) / stats.magazineCount;
                std::cout << "  " << std::setw(10) << std::left << pair.first
                    << ": " << std::setw(3) << pair.second
                    << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
            }

            std::cout << "\n--- Magazine Capacity Stats ---\n";
            std::cout << "  Average: " << (stats.totalMagazineCapacity / stats.magazineCount) << " rounds\n";
            std::cout << "  Min:     " << stats.minMagazineCapacity << " rounds\n";
            std::cout << "  Max:     " << stats.maxMagazineCapacity << " rounds\n";

            if (stats.magazinesWithLoadPlan > 0)
            {
                std::cout << "\n--- Magazine Load Plans ---\n";
                std::cout << "  Magazines with load order: " << stats.magazinesWithLoadPlan
                    << " / " << stats.magazineCount << "\n";
                std::cout << "  Mixed-ammo loadouts:      " << stats.mixedAmmoMagazineCount
                    << " (" << (stats.mixedAmmoMagazineCount * 100.0f / stats.magazineCount) << "% of all magazines)\n";
                std::cout << "  Avg planned rounds stored: "
                    << (stats.totalPlannedRounds / stats.magazinesWithLoadPlan) << "\n";
            }
        }

        std::cout << "\n--- Damage Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalDamageModifier / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minDamageModifier << "\n";
        std::cout << "  Max:     " << stats.maxDamageModifier << "\n";

        std::cout << "\n--- Recoil Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalRecoilModifier / stats.count) << " (positive = less recoil)\n";
        std::cout << "  Min:     " << stats.minRecoilModifier << "\n";
        std::cout << "  Max:     " << stats.maxRecoilModifier << "\n";

        std::cout << "\n--- Ergonomics Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalErgonomicsModifier / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minErgonomicsModifier << "\n";
        std::cout << "  Max:     " << stats.maxErgonomicsModifier << "\n";

        std::cout << "\n--- Accuracy Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalAccuracyModifier / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minAccuracyModifier << "\n";
        std::cout << "  Max:     " << stats.maxAccuracyModifier << "\n";

        std::cout << "\n--- Weight Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalWeightModifier / stats.count) << " grams\n";
        std::cout << "  Min:     " << stats.minWeightModifier << " grams\n";
        std::cout << "  Max:     " << stats.maxWeightModifier << " grams\n";

        std::cout << "\n--- Muzzle Velocity Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalMuzzleVelocityModifier / stats.count) << " m/s\n";
        std::cout << "  Min:     " << stats.minMuzzleVelocityModifier << " m/s\n";
        std::cout << "  Max:     " << stats.maxMuzzleVelocityModifier << " m/s\n";

        std::cout << "\n--- Effective Range Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalEffectiveRangeModifier / stats.count) << " meters\n";
        std::cout << "  Min:     " << stats.minEffectiveRangeModifier << " meters\n";
        std::cout << "  Max:     " << stats.maxEffectiveRangeModifier << " meters\n";

        std::cout << "\n--- Penetration Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalPenetrationModifier / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minPenetrationModifier << "\n";
        std::cout << "  Max:     " << stats.maxPenetrationModifier << "\n";

        // Balance warnings
        std::cout << "\n--- Balance Warnings ---\n";
        bool hasWarnings = false;

        if (stats.maxDamageModifier > 40)
        {
            std::cout << "  [WARNING] Very high damage modifier detected (max: +" << stats.maxDamageModifier << ")\n";
            hasWarnings = true;
        }
        if (stats.maxWeightModifier > 1000)
        {
            std::cout << "  [WARNING] Very high weight modifier detected (max: +" << stats.maxWeightModifier << " grams)\n";
            hasWarnings = true;
        }
        if (stats.minWeightModifier < -1000)
        {
            std::cout << "  [WARNING] Very high weight reduction detected (min: " << stats.minWeightModifier << " grams)\n";
            hasWarnings = true;
        }

        if (!hasWarnings)
        {
            std::cout << "  [OK] No balance issues detected.\n";
        }

        std::cout << "========================================\n\n";
    }

    static void PrintAmmoReport(const std::vector<ItemAmmoData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No ammo items to analyze.\n";
            return;
        }

        AmmoStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalDamageBonus += item.damageBonus;
            stats.totalPenetration += item.penetration;
            stats.totalAccuracyBonus += item.accuracyBonus;
            stats.totalRecoilModifier += item.recoilModifier;
            stats.totalValue += item.value;

            stats.minDamageBonus = std::min(stats.minDamageBonus, item.damageBonus);
            stats.maxDamageBonus = std::max(stats.maxDamageBonus, item.damageBonus);
            stats.minPenetration = std::min(stats.minPenetration, item.penetration);
            stats.maxPenetration = std::max(stats.maxPenetration, item.penetration);
            stats.minAccuracyBonus = std::min(stats.minAccuracyBonus, item.accuracyBonus);
            stats.maxAccuracyBonus = std::max(stats.maxAccuracyBonus, item.accuracyBonus);
            stats.minRecoilModifier = std::min(stats.minRecoilModifier, item.recoilModifier);
            stats.maxRecoilModifier = std::max(stats.maxRecoilModifier, item.recoilModifier);
            stats.minValue = std::min(stats.minValue, item.value);
            stats.maxValue = std::max(stats.maxValue, item.value);

            if (item.armorPiercing) stats.armorPiercingCount++;
            if (item.hollowPoint) stats.hollowPointCount++;
            if (item.tracer) stats.tracerCount++;
            if (item.incendiary) stats.incendiaryCount++;

            stats.rarityCount[item.rarity]++;
            stats.caliberCount[item.caliber]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   AMMO ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        // Quick Summary
        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Damage Bonus: " << std::fixed << std::setprecision(1) 
            << (stats.totalDamageBonus / (float)stats.count) << "\n";
        std::cout << "  Average Penetration: " << (stats.totalPenetration / stats.count) << "/100\n";
        std::cout << "  Average Accuracy Bonus: " << (stats.totalAccuracyBonus / stats.count) << "\n";
        std::cout << "  Average Value: " << (stats.totalValue / stats.count) << "\n";
        
        int specialCount = stats.armorPiercingCount + stats.hollowPointCount + stats.tracerCount + stats.incendiaryCount;
        std::cout << "  Special Rounds: " << specialCount << " (" 
            << (specialCount * 100.0f / stats.count) << "% of all ammo)\n";
        std::cout << "    - AP: " << stats.armorPiercingCount 
            << ", HP: " << stats.hollowPointCount 
            << ", Tracer: " << stats.tracerCount 
            << ", Incendiary: " << stats.incendiaryCount << "\n";
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Caliber Distribution ---\n";
        for (const auto& pair : stats.caliberCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Special Properties Distribution ---\n";
        std::cout << "  Armor Piercing: " << stats.armorPiercingCount
            << " (" << (stats.armorPiercingCount * 100.0f / stats.count) << "%)\n";
        std::cout << "  Hollow Point:   " << stats.hollowPointCount
            << " (" << (stats.hollowPointCount * 100.0f / stats.count) << "%)\n";
        std::cout << "  Tracer:         " << stats.tracerCount
            << " (" << (stats.tracerCount * 100.0f / stats.count) << "%)\n";
        std::cout << "  Incendiary:     " << stats.incendiaryCount
            << " (" << (stats.incendiaryCount * 100.0f / stats.count) << "%)\n";

        std::cout << "\n--- Damage Bonus Stats ---\n";
        std::cout << "  Average: " << (stats.totalDamageBonus / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minDamageBonus << "\n";
        std::cout << "  Max:     " << stats.maxDamageBonus << "\n";

        std::cout << "\n--- Penetration Stats ---\n";
        std::cout << "  Average: " << (stats.totalPenetration / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minPenetration << "\n";
        std::cout << "  Max:     " << stats.maxPenetration << "\n";

        std::cout << "\n--- Accuracy Bonus Stats ---\n";
        std::cout << "  Average: " << (stats.totalAccuracyBonus / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minAccuracyBonus << "\n";
        std::cout << "  Max:     " << stats.maxAccuracyBonus << "\n";

        std::cout << "\n--- Recoil Modifier Stats ---\n";
        std::cout << "  Average: " << (stats.totalRecoilModifier / stats.count) << " (positive = less recoil)\n";
        std::cout << "  Min:     " << stats.minRecoilModifier << "\n";
        std::cout << "  Max:     " << stats.maxRecoilModifier << "\n";

        std::cout << "\n--- Value Stats ---\n";
        std::cout << "  Average: " << (stats.totalValue / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minValue << "\n";
        std::cout << "  Max:     " << stats.maxValue << "\n";

        // Balance warnings
        std::cout << "\n--- Balance Warnings ---\n";
        bool hasWarnings = false;

        if (stats.maxDamageBonus > 30)
        {
            std::cout << "  [WARNING] Very high damage bonus detected (max: +" << stats.maxDamageBonus << ")\n";
            hasWarnings = true;
        }
        if (stats.maxPenetration > 90)
        {
            std::cout << "  [WARNING] Very high penetration detected (max: " << stats.maxPenetration << ")\n";
            hasWarnings = true;
        }
        if (stats.maxValue > 90)
        {
            std::cout << "  [WARNING] Very high value detected (max: " << stats.maxValue << ")\n";
            hasWarnings = true;
        }
        if (stats.armorPiercingCount > stats.count * 0.5f)
        {
            std::cout << "  [WARNING] Too many armor piercing rounds (" << stats.armorPiercingCount << " / " << stats.count << ")\n";
            hasWarnings = true;
        }

        if (!hasWarnings)
        {
            std::cout << "  [OK] No balance issues detected.\n";
        }

        std::cout << "========================================\n\n";
    }

    static void PrintArmorReport(const std::vector<ItemArmorData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No armor items to analyze.\n";
            return;
        }

        ArmorStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalArmorClass += item.armorClass;
            stats.totalDurability += item.durability;
            stats.totalMaterial += item.material;
            stats.totalMovementSpeedPenalty += item.movementSpeedPenalty;
            stats.totalErgonomicsPenalty += item.ergonomicsPenalty;
            stats.totalTurnSpeedPenalty += item.turnSpeedPenalty;
            stats.totalWeight += item.weight;
            stats.totalCapacity += item.capacity;

            stats.minArmorClass = std::min(stats.minArmorClass, item.armorClass);
            stats.maxArmorClass = std::max(stats.maxArmorClass, item.armorClass);
            stats.minDurability = std::min(stats.minDurability, item.durability);
            stats.maxDurability = std::max(stats.maxDurability, item.durability);
            stats.minMaterial = std::min(stats.minMaterial, item.material);
            stats.maxMaterial = std::max(stats.maxMaterial, item.material);
            stats.minMovementSpeedPenalty = std::min(stats.minMovementSpeedPenalty, item.movementSpeedPenalty);
            stats.maxMovementSpeedPenalty = std::max(stats.maxMovementSpeedPenalty, item.movementSpeedPenalty);
            stats.minErgonomicsPenalty = std::min(stats.minErgonomicsPenalty, item.ergonomicsPenalty);
            stats.maxErgonomicsPenalty = std::max(stats.maxErgonomicsPenalty, item.ergonomicsPenalty);
            stats.minTurnSpeedPenalty = std::min(stats.minTurnSpeedPenalty, item.turnSpeedPenalty);
            stats.maxTurnSpeedPenalty = std::max(stats.maxTurnSpeedPenalty, item.turnSpeedPenalty);
            stats.minWeight = std::min(stats.minWeight, item.weight);
            stats.maxWeight = std::max(stats.maxWeight, item.weight);
            stats.minCapacity = std::min(stats.minCapacity, item.capacity);
            stats.maxCapacity = std::max(stats.maxCapacity, item.capacity);

            if (item.blocksHeadset) stats.blocksHeadsetCount++;
            if (item.blocksFaceCover) stats.blocksFaceCoverCount++;

            stats.rarityCount[item.rarity]++;
            stats.armorTypeCount[item.armorType]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   ARMOR ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Armor Class: " << std::fixed << std::setprecision(1)
            << (stats.totalArmorClass / (float)stats.count) << "/6\n";
        std::cout << "  Average Durability: " << (stats.totalDurability / stats.count) << "/100\n";
        std::cout << "  Average Weight: " << (stats.totalWeight / stats.count) << " grams\n";
        std::cout << "  Average Movement Speed Penalty: " << (stats.totalMovementSpeedPenalty / stats.count) << "%\n";
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Armor Type Distribution ---\n";
        for (const auto& pair : stats.armorTypeCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(15) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Armor Class Stats ---\n";
        std::cout << "  Average: " << (stats.totalArmorClass / stats.count) << "\n";
        std::cout << "  Min:     " << stats.minArmorClass << "\n";
        std::cout << "  Max:     " << stats.maxArmorClass << "\n";

        std::cout << "\n--- Weight Stats ---\n";
        std::cout << "  Average: " << (stats.totalWeight / stats.count) << " grams\n";
        std::cout << "  Min:     " << stats.minWeight << " grams\n";
        std::cout << "  Max:     " << stats.maxWeight << " grams\n";

        std::cout << "\n--- Penalty Stats ---\n";
        std::cout << "  Movement Speed Penalty: " << (stats.totalMovementSpeedPenalty / stats.count) << "% (avg)\n";
        std::cout << "  Ergonomics Penalty: " << (stats.totalErgonomicsPenalty / stats.count) << "% (avg)\n";
        std::cout << "  Turn Speed Penalty: " << (stats.totalTurnSpeedPenalty / stats.count) << "% (avg)\n";

        std::cout << "\n--- Special Properties ---\n";
        std::cout << "  Blocks Headset: " << stats.blocksHeadsetCount
            << " (" << (stats.blocksHeadsetCount * 100.0f / stats.count) << "%)\n";
        std::cout << "  Blocks Face Cover: " << stats.blocksFaceCoverCount
            << " (" << (stats.blocksFaceCoverCount * 100.0f / stats.count) << "%)\n";

        std::cout << "========================================\n\n";
    }

    static void PrintClothingReport(const std::vector<ItemClothingData>& items)
    {
        if (items.empty())
        {
            std::cout << "[BalanceReporter] No clothing items to analyze.\n";
            return;
        }

        ClothingStats stats;
        for (const auto& item : items)
        {
            stats.count++;
            stats.totalColdResistance += item.coldResistance;
            stats.totalHeatResistance += item.heatResistance;
            stats.totalWaterResistance += item.waterResistance;
            stats.totalWindResistance += item.windResistance;
            stats.totalComfort += item.comfort;
            stats.totalMobilityBonus += item.mobilityBonus;
            stats.totalStaminaBonus += item.staminaBonus;
            stats.totalDurability += item.durability;
            stats.totalMaterial += item.material;
            stats.totalWeight += item.weight;

            stats.minColdResistance = std::min(stats.minColdResistance, item.coldResistance);
            stats.maxColdResistance = std::max(stats.maxColdResistance, item.coldResistance);
            stats.minHeatResistance = std::min(stats.minHeatResistance, item.heatResistance);
            stats.maxHeatResistance = std::max(stats.maxHeatResistance, item.heatResistance);
            stats.minWaterResistance = std::min(stats.minWaterResistance, item.waterResistance);
            stats.maxWaterResistance = std::max(stats.maxWaterResistance, item.waterResistance);
            stats.minWindResistance = std::min(stats.minWindResistance, item.windResistance);
            stats.maxWindResistance = std::max(stats.maxWindResistance, item.windResistance);
            stats.minComfort = std::min(stats.minComfort, item.comfort);
            stats.maxComfort = std::max(stats.maxComfort, item.comfort);
            stats.minMobilityBonus = std::min(stats.minMobilityBonus, item.mobilityBonus);
            stats.maxMobilityBonus = std::max(stats.maxMobilityBonus, item.mobilityBonus);
            stats.minStaminaBonus = std::min(stats.minStaminaBonus, item.staminaBonus);
            stats.maxStaminaBonus = std::max(stats.maxStaminaBonus, item.staminaBonus);
            stats.minDurability = std::min(stats.minDurability, item.durability);
            stats.maxDurability = std::max(stats.maxDurability, item.durability);
            stats.minMaterial = std::min(stats.minMaterial, item.material);
            stats.maxMaterial = std::max(stats.maxMaterial, item.material);
            stats.minWeight = std::min(stats.minWeight, item.weight);
            stats.maxWeight = std::max(stats.maxWeight, item.weight);

            if (item.isInsulated) stats.isInsulatedCount++;
            if (item.isWaterproof) stats.isWaterproofCount++;
            if (item.isWindproof) stats.isWindproofCount++;

            stats.rarityCount[item.rarity]++;
            stats.clothingTypeCount[item.clothingType]++;
        }

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   CLOTHING ITEMS BALANCE REPORT\n";
        std::cout << "========================================\n";
        std::cout << "Total Items: " << stats.count << "\n\n";

        std::cout << "--- Quick Summary ---\n";
        std::cout << "  Average Cold Resistance: " << (stats.totalColdResistance / stats.count) << "/100\n";
        std::cout << "  Average Heat Resistance: " << (stats.totalHeatResistance / stats.count) << "/100\n";
        std::cout << "  Average Water Resistance: " << (stats.totalWaterResistance / stats.count) << "/100\n";
        std::cout << "  Average Comfort: " << (stats.totalComfort / stats.count) << "/100\n";
        std::cout << "  Average Weight: " << (stats.totalWeight / stats.count) << " grams\n";
        std::cout << "\n";

        std::cout << "--- Rarity Distribution ---\n";
        for (const auto& pair : stats.rarityCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(10) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Clothing Type Distribution ---\n";
        for (const auto& pair : stats.clothingTypeCount)
        {
            float percentage = (pair.second * 100.0f) / stats.count;
            std::cout << "  " << std::setw(15) << std::left << pair.first
                << ": " << std::setw(3) << pair.second
                << " (" << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }

        std::cout << "\n--- Environmental Protection Stats ---\n";
        std::cout << "  Cold Resistance: " << (stats.totalColdResistance / stats.count) << " (avg), "
            << stats.minColdResistance << "-" << stats.maxColdResistance << " (range)\n";
        std::cout << "  Heat Resistance: " << (stats.totalHeatResistance / stats.count) << " (avg), "
            << stats.minHeatResistance << "-" << stats.maxHeatResistance << " (range)\n";
        std::cout << "  Water Resistance: " << (stats.totalWaterResistance / stats.count) << " (avg), "
            << stats.minWaterResistance << "-" << stats.maxWaterResistance << " (range)\n";
        std::cout << "  Wind Resistance: " << (stats.totalWindResistance / stats.count) << " (avg), "
            << stats.minWindResistance << "-" << stats.maxWindResistance << " (range)\n";

        std::cout << "\n--- Comfort and Mobility Stats ---\n";
        std::cout << "  Comfort: " << (stats.totalComfort / stats.count) << " (avg), "
            << stats.minComfort << "-" << stats.maxComfort << " (range)\n";
        std::cout << "  Mobility Bonus: " << (stats.totalMobilityBonus / stats.count) << " (avg), "
            << stats.minMobilityBonus << "-" << stats.maxMobilityBonus << " (range)\n";
        std::cout << "  Stamina Bonus: " << (stats.totalStaminaBonus / stats.count) << " (avg), "
            << stats.minStaminaBonus << "-" << stats.maxStaminaBonus << " (range)\n";

        std::cout << "\n--- Special Properties ---\n";
        std::cout << "  Insulated: " << stats.isInsulatedCount
            << " (" << (stats.isInsulatedCount * 100.0f / stats.count) << "%)\n";
        std::cout << "  Waterproof: " << stats.isWaterproofCount
            << " (" << (stats.isWaterproofCount * 100.0f / stats.count) << "%)\n";
        std::cout << "  Windproof: " << stats.isWindproofCount
            << " (" << (stats.isWindproofCount * 100.0f / stats.count) << "%)\n";

        std::cout << "========================================\n\n";
    }

    int GenerateReport(const std::string& jsonFilePath, ItemType itemType)
    {
        // Check if file exists
        std::ifstream file(jsonFilePath);
        if (!file.is_open())
        {
            std::cerr << "[BalanceReporter] Error: Failed to open file: " << jsonFilePath << "\n";
            std::cerr << "[BalanceReporter] Please check if the file exists and is accessible.\n";
            return 1;
        }

        std::string jsonText((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        // Check if file is empty
        if (jsonText.empty() || jsonText.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            std::cerr << "[BalanceReporter] Error: File is empty or contains only whitespace: " << jsonFilePath << "\n";
            return 1;
        }

        if (itemType == ItemType::Food)
        {
            std::vector<ItemFoodData> items;
            if (!ItemJsonParser::ParseFoodFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse food JSON.\n";
                return 1;
            }
            PrintFoodReport(items);
        }
        else if (itemType == ItemType::Drink)
        {
            std::vector<ItemDrinkData> items;
            if (!ItemJsonParser::ParseDrinkFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse drink JSON.\n";
                return 1;
            }
            PrintDrinkReport(items);
        }
        else if (itemType == ItemType::Material)
        {
            std::vector<ItemMaterialData> items;
            if (!ItemJsonParser::ParseMaterialFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse material JSON.\n";
                return 1;
            }
            PrintMaterialReport(items);
        }
        else if (itemType == ItemType::Weapon)
        {
            std::vector<ItemWeaponData> items;
            if (!ItemJsonParser::ParseWeaponFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse weapon JSON.\n";
                return 1;
            }
            PrintWeaponReport(items);
        }
        else if (itemType == ItemType::WeaponComponent)
        {
            std::vector<ItemWeaponComponentData> items;
            if (!ItemJsonParser::ParseWeaponComponentFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse weapon component JSON.\n";
                return 1;
            }
            PrintWeaponComponentReport(items);
        }
        else if (itemType == ItemType::Ammo)
        {
            std::vector<ItemAmmoData> items;
            if (!ItemJsonParser::ParseAmmoFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse ammo JSON.\n";
                return 1;
            }
            PrintAmmoReport(items);
        }
        else if (itemType == ItemType::Armor)
        {
            std::vector<ItemArmorData> items;
            if (!ItemJsonParser::ParseArmorFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse armor JSON.\n";
                return 1;
            }
            PrintArmorReport(items);
        }
        else if (itemType == ItemType::Clothing)
        {
            std::vector<ItemClothingData> items;
            if (!ItemJsonParser::ParseClothingFromJsonText(jsonText, items))
            {
                std::cerr << "[BalanceReporter] Failed to parse clothing JSON.\n";
                return 1;
            }
            PrintClothingReport(items);
        }
        else
        {
            std::cerr << "[BalanceReporter] Unknown item type.\n";
            return 1;
        }

        return 0;
    }
}

