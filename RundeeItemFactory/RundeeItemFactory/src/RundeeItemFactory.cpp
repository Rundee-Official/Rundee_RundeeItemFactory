// ===============================
// Project Name: RundeeItemFactory
// File Name: main.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Entry point for LLM/dummy-based item generation pipeline.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include <iostream>
#include "Helpers/CommandLineParser.h"
#include "Helpers/ItemGenerator.h"
#include "Helpers/BalanceReporter.h"

int main(int argc, char** argv)
{
    // Parse command line arguments
    CommandLineArgs args = CommandLineParser::ParseArguments(argc, argv);

    // Check if report mode
    if (!args.reportPath.empty())
    {
        std::cout << "[Main] Generating balance report for: " << args.reportPath << "\n";
        return BalanceReporter::GenerateReport(args.reportPath, args.itemType);
    }

    // Print configuration
    std::cout << "[Main] Mode = " << CommandLineParser::GetRunModeName(args.mode)
        << ", itemType = " << CommandLineParser::GetItemTypeName(args.itemType)
        << ", model = " << args.modelName
        << ", count = " << args.params.count
        << ", out = " << args.params.outputPath << "\n";
    std::cout << "[Main] Preset = " << CommandLineParser::GetPresetName(args.preset) << "\n";

    // Generate items based on mode
    if (args.mode == RunMode::LLM)
    {
        return ItemGenerator::GenerateWithLLM(args);
    }
    else
    {
        return ItemGenerator::GenerateDummy(args);
    }
}
