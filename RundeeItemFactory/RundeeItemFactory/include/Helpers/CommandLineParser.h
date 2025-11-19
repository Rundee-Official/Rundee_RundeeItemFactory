// ===============================
// Project Name: RundeeItemFactory
// File Name: CommandLineParser.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Command line argument parsing helpers.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include "Helpers/ItemGenerateParams.h"
#include "Prompts/PromptBuilder.h"

enum class RunMode
{
    Dummy,
    LLM,
    Batch
};

enum class ItemType
{
    Food,
    Drink,
    Material,
    Weapon,
    WeaponComponent,
    Ammo
};

struct BatchItem
{
    ItemType itemType;
    int count;
    std::string outputPath;  // Optional: custom output path for this item type
};

struct CommandLineArgs
{
    std::string modelName = "llama3";
    RunMode mode = RunMode::LLM;
    PresetType preset = PresetType::Default;
    ItemType itemType = ItemType::Food;
    FoodGenerateParams params;
    std::string reportPath;   // Path to JSON file for balance report (empty if not reporting)
    
    // Batch generation
    std::vector<BatchItem> batchItems;  // For batch mode: multiple item types to generate
    
    // Custom preset
    std::string customPresetPath;  // Path to custom preset JSON file (empty if using built-in preset)
};

namespace CommandLineParser
{
    // Parse command line arguments
    CommandLineArgs ParseArguments(int argc, char** argv);

    // Parse batch string (e.g., "food:10,weapon:5,ammo:20")
    std::vector<BatchItem> ParseBatchString(const std::string& batchStr);

    // Convert enum to string
    std::string GetPresetName(PresetType preset);
    std::string GetItemTypeName(ItemType itemType);
    std::string GetRunModeName(RunMode mode);
    
    // Convert string to ItemType
    ItemType ParseItemType(const std::string& typeStr);
}


