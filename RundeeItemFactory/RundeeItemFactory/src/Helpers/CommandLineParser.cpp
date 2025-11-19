// ===============================
// Project Name: RundeeItemFactory
// File Name: CommandLineParser.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of command line argument parsing.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Helpers/CommandLineParser.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <algorithm>

namespace CommandLineParser
{
    CommandLineArgs ParseArguments(int argc, char** argv)
    {
        CommandLineArgs args;
        args.params.count = 5;
        args.params.maxHunger = 100;
        args.params.maxThirst = 100;
        args.params.outputPath = "items_food.json";

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "--model" && i + 1 < argc)
            {
                args.modelName = argv[++i];
            }
            else if (arg == "--mode" && i + 1 < argc)
            {
                std::string m = argv[++i];
                if (m == "llm")
                {
                    args.mode = RunMode::LLM;
                }
                else if (m == "dummy")
                {
                    args.mode = RunMode::Dummy;
                }
                else if (m == "batch")
                {
                    args.mode = RunMode::Batch;
                }
                else
                {
                    std::cout << "[Warning] Unknown mode: " << m
                        << " (use 'dummy', 'llm', or 'batch')\n";
                }
            }
            else if (arg == "--batch" && i + 1 < argc)
            {
                std::string batchStr = argv[++i];
                args.batchItems = ParseBatchString(batchStr);
                if (!args.batchItems.empty())
                {
                    args.mode = RunMode::Batch;
                }
            }
            else if (arg == "--preset" && i + 1 < argc)
            {
                std::string p = argv[++i];
                if (p == "forest")
                    args.preset = PresetType::Forest;
                else if (p == "desert")
                    args.preset = PresetType::Desert;
                else if (p == "coast")
                    args.preset = PresetType::Coast;
                else if (p == "city")
                    args.preset = PresetType::City;
                else if (p == "default")
                    args.preset = PresetType::Default;
                else
                    std::cout << "[Warning] Unknown preset: " << p
                    << " (use default/forest/desert/coast/city)\n";
            }
            else if (arg == "--itemType" && i + 1 < argc)
            {
                std::string t = argv[++i];
                if (t == "food")
                    args.itemType = ItemType::Food;
                else if (t == "drink")
                    args.itemType = ItemType::Drink;
                else if (t == "material")
                    args.itemType = ItemType::Material;
                else if (t == "weapon")
                    args.itemType = ItemType::Weapon;
                else if (t == "weaponcomponent")
                    args.itemType = ItemType::WeaponComponent;
                else if (t == "ammo")
                    args.itemType = ItemType::Ammo;
                else
                    std::cout << "[Warning] Unknown itemType: " << t
                    << " (use 'food', 'drink', 'material', 'weapon', 'weaponcomponent', or 'ammo')\n";
            }
            else if (arg == "--count" && i + 1 < argc)
            {
                args.params.count = std::atoi(argv[++i]);
            }
            else if (arg == "--maxHunger" && i + 1 < argc)
            {
                args.params.maxHunger = std::atoi(argv[++i]);
            }
            else if (arg == "--maxThirst" && i + 1 < argc)
            {
                args.params.maxThirst = std::atoi(argv[++i]);
            }
            else if (arg == "--out" && i + 1 < argc)
            {
                args.params.outputPath = argv[++i];
            }
            else if (arg == "--report" && i + 1 < argc)
            {
                args.reportPath = argv[++i];
                args.mode = RunMode::Dummy; // Report mode doesn't need LLM
            }
            else if (arg == "--customPreset" && i + 1 < argc)
            {
                args.customPresetPath = argv[++i];
            }
            else
            {
                std::cout << "[Warning] Unknown or incomplete argument: " << arg << "\n";
            }
        }

        return args;
    }

    std::string GetPresetName(PresetType preset)
    {
        switch (preset)
        {
        case PresetType::Forest:  return "forest";
        case PresetType::Desert:  return "desert";
        case PresetType::Coast:   return "coast";
        case PresetType::City:    return "city";
        default:                  return "default";
        }
    }

    std::string GetItemTypeName(ItemType itemType)
    {
        switch (itemType)
        {
        case ItemType::Drink:         return "drink";
        case ItemType::Material:       return "material";
        case ItemType::Weapon:         return "weapon";
        case ItemType::WeaponComponent: return "weaponcomponent";
        case ItemType::Ammo:           return "ammo";
        case ItemType::Food:
        default:                       return "food";
        }
    }

    std::string GetRunModeName(RunMode mode)
    {
        switch (mode)
        {
        case RunMode::LLM: return "LLM";
        case RunMode::Batch: return "Batch";
        case RunMode::Dummy:
        default: return "Dummy";
        }
    }

    ItemType ParseItemType(const std::string& typeStr)
    {
        std::string lower = typeStr;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        if (lower == "food")
            return ItemType::Food;
        else if (lower == "drink")
            return ItemType::Drink;
        else if (lower == "material")
            return ItemType::Material;
        else if (lower == "weapon")
            return ItemType::Weapon;
        else if (lower == "weaponcomponent" || lower == "weapon_component")
            return ItemType::WeaponComponent;
        else if (lower == "ammo")
            return ItemType::Ammo;
        else
            return ItemType::Food; // Default
    }

    std::vector<BatchItem> ParseBatchString(const std::string& batchStr)
    {
        std::vector<BatchItem> result;
        
        if (batchStr.empty())
        {
            std::cerr << "[CommandLineParser] Error: Batch string is empty.\n";
            return result;
        }

        std::istringstream iss(batchStr);
        std::string token;
        
        while (std::getline(iss, token, ','))
        {
            // Trim whitespace
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            
            if (token.empty())
                continue;

            // Parse format: "itemtype:count" or "itemtype:count:outputpath"
            size_t colon1 = token.find(':');
            if (colon1 == std::string::npos)
            {
                std::cerr << "[CommandLineParser] Warning: Invalid batch item format: " << token
                    << " (expected 'itemtype:count' or 'itemtype:count:outputpath')\n";
                continue;
            }

            std::string typeStr = token.substr(0, colon1);
            std::string rest = token.substr(colon1 + 1);
            
            size_t colon2 = rest.find(':');
            std::string countStr = (colon2 == std::string::npos) ? rest : rest.substr(0, colon2);
            std::string outputPath = (colon2 == std::string::npos) ? "" : rest.substr(colon2 + 1);

            ItemType itemType = ParseItemType(typeStr);
            int count = std::atoi(countStr.c_str());
            
            if (count <= 0)
            {
                std::cerr << "[CommandLineParser] Warning: Invalid count for " << typeStr
                    << ": " << countStr << "\n";
                continue;
            }

            BatchItem batchItem;
            batchItem.itemType = itemType;
            batchItem.count = count;
            batchItem.outputPath = outputPath;
            
            result.push_back(batchItem);
        }

        if (result.empty())
        {
            std::cerr << "[CommandLineParser] Error: No valid batch items parsed from: " << batchStr << "\n";
        }
        else
        {
            std::cout << "[CommandLineParser] Parsed " << result.size() << " batch items.\n";
        }

        return result;
    }
}


