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
        args.params.outputPath = "ItemJson/items_food.json";

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
                else
                {
                    std::cout << "[Warning] Unknown mode: " << m
                        << " (use 'llm')\n";
                }
            }
            else if (arg == "--preset" && i + 1 < argc)
            {
                args.presetName = argv[++i];
            }
            else if (arg == "--itemType" && i + 1 < argc)
            {
                std::string t = argv[++i];
                if (t == "food")
                    args.itemType = ItemType::Food;
                else if (t == "drink")
                    args.itemType = ItemType::Drink;
                else if (t == "medicine")
                    args.itemType = ItemType::Medicine;
                else if (t == "material")
                    args.itemType = ItemType::Material;
                else if (t == "weapon")
                    args.itemType = ItemType::Weapon;
                else if (t == "weaponcomponent")
                    args.itemType = ItemType::WeaponComponent;
                else if (t == "ammo")
                    args.itemType = ItemType::Ammo;
                else if (t == "armor")
                    args.itemType = ItemType::Armor;
                else if (t == "clothing")
                    args.itemType = ItemType::Clothing;
                else
                    std::cout << "[Warning] Unknown itemType: " << t
                    << " (use 'food', 'drink', 'medicine', 'material', 'weapon', 'weaponcomponent', 'ammo', 'armor', or 'clothing')\n";
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
                // report mode is independent of LLM; mode value unused when reportPath is set
            }
            else if (arg == "--customPreset" && i + 1 < argc)
            {
                args.customPresetPath = argv[++i];
            }
            else if (arg == "--additionalPrompt" && i + 1 < argc)
            {
                args.additionalPrompt = argv[++i];
            }
            else if (arg == "--test" || arg == "--testMode")
            {
                args.useTestMode = true;
            }
            else if (arg == "--profile" && i + 1 < argc)
            {
                args.profileId = argv[++i];
            }
            else
            {
                std::cout << "[Warning] Unknown or incomplete argument: " << arg << "\n";
            }
        }

        return args;
    }

    std::string GetItemTypeName(ItemType itemType)
    {
        switch (itemType)
        {
        case ItemType::Drink:         return "drink";
        case ItemType::Medicine:      return "medicine";
        case ItemType::Material:       return "material";
        case ItemType::Weapon:         return "weapon";
        case ItemType::WeaponComponent: return "weaponcomponent";
        case ItemType::Ammo:           return "ammo";
        case ItemType::Armor:          return "armor";
        case ItemType::Clothing:       return "clothing";
        case ItemType::Food:
        default:                       return "food";
        }
    }

    std::string GetRunModeName(RunMode mode)
    {
        switch (mode)
        {
        case RunMode::LLM: return "LLM";
        default: return "LLM";
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
        else if (lower == "medicine")
            return ItemType::Medicine;
        else if (lower == "material")
            return ItemType::Material;
        else if (lower == "weapon")
            return ItemType::Weapon;
        else if (lower == "weaponcomponent" || lower == "weapon_component")
            return ItemType::WeaponComponent;
        else if (lower == "ammo")
            return ItemType::Ammo;
        else if (lower == "armor")
            return ItemType::Armor;
        else if (lower == "clothing")
            return ItemType::Clothing;
        else
            return ItemType::Food; // Default
    }

}


