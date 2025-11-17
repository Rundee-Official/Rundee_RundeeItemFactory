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
                else
                {
                    std::cout << "[Warning] Unknown mode: " << m
                        << " (use 'dummy' or 'llm')\n";
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
        return (mode == RunMode::LLM) ? "LLM" : "Dummy";
    }
}


