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
#include "Generators/ItemFoodGenerator.h"
#include "Prompts/PromptBuilder.h"

enum class RunMode
{
    Dummy,
    LLM
};

enum class ItemType
{
    Food,
    Drink,
    Material
};

struct CommandLineArgs
{
    std::string modelName = "llama3";
    RunMode mode = RunMode::Dummy;
    PresetType preset = PresetType::Default;
    ItemType itemType = ItemType::Food;
    FoodGenerateParams params;
};

namespace CommandLineParser
{
    // Parse command line arguments
    CommandLineArgs ParseArguments(int argc, char** argv);

    // Convert enum to string
    std::string GetPresetName(PresetType preset);
    std::string GetItemTypeName(ItemType itemType);
    std::string GetRunModeName(RunMode mode);
}


