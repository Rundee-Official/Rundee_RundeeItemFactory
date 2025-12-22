// ===============================
// Project Name: RundeeItemFactory
// File Name: RundeeItemFactory.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Entry point for LLM/dummy-based item generation pipeline.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include <iostream>
#include "Helpers/AppConfig.h"
#include "Helpers/CommandLineParser.h"
#include "Generators/ItemGenerator.h"

int main(int argc, char** argv)
{
    // Load optional configuration (host/port/timeouts) before doing any work.
    AppConfig::LoadFromDefaultLocation();

    // Parse command line arguments
    CommandLineArgs args = CommandLineParser::ParseArguments(argc, argv);

    // Print configuration
    std::cout << "[Main] Mode = " << CommandLineParser::GetRunModeName(args.mode)
        << ", itemType = " << CommandLineParser::GetItemTypeName(args.itemType)
        << ", model = " << args.modelName
        << ", count = " << args.params.count
        << ", out = " << args.params.outputPath << "\n";
    if (!args.presetName.empty())
    {
        std::cout << "[Main] Preset = " << args.presetName << "\n";
    }
    if (!args.customPresetPath.empty())
    {
        std::cout << "[Main] Custom Preset = " << args.customPresetPath << "\n";
    }

    // Generate items
    return ItemGenerator::GenerateWithLLM(args);
}
