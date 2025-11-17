// ===============================
// Project Name: RundeeItemFactory
// File Name: BalanceReporter.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of balance report generator.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Helpers/BalanceReporter.h"
#include "Parsers/ItemJsonParser.h"
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMaterialData.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <iomanip>

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

        float avgPower = (stats.totalHunger + stats.totalThirst + stats.totalHealth) / (float)stats.count;
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

    int GenerateReport(const std::string& jsonFilePath, ItemType itemType)
    {
        std::ifstream file(jsonFilePath);
        if (!file.is_open())
        {
            std::cerr << "[BalanceReporter] Failed to open file: " << jsonFilePath << "\n";
            return 1;
        }

        std::string jsonText((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

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
        else
        {
            std::cerr << "[BalanceReporter] Unknown item type.\n";
            return 1;
        }

        return 0;
    }
}

