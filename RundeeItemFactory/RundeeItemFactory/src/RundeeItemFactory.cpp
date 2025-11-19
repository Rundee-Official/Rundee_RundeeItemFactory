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
    if (args.mode == RunMode::Batch)
    {
        std::cout << "[Main] Mode = Batch\n";
        std::cout << "[Main] Model = " << args.modelName << "\n";
        std::cout << "[Main] Preset = " << CommandLineParser::GetPresetName(args.preset) << "\n";
        std::cout << "[Main] Batch Items: " << args.batchItems.size() << "\n";
        for (size_t i = 0; i < args.batchItems.size(); ++i)
        {
            const auto& item = args.batchItems[i];
            std::cout << "  " << (i + 1) << ". " << CommandLineParser::GetItemTypeName(item.itemType)
                << " x" << item.count;
            if (!item.outputPath.empty())
            {
                std::cout << " -> " << item.outputPath;
            }
            std::cout << "\n";
        }
    }
    else
    {
        std::cout << "[Main] Mode = " << CommandLineParser::GetRunModeName(args.mode)
            << ", itemType = " << CommandLineParser::GetItemTypeName(args.itemType)
            << ", model = " << args.modelName
            << ", count = " << args.params.count
            << ", out = " << args.params.outputPath << "\n";
        std::cout << "[Main] Preset = " << CommandLineParser::GetPresetName(args.preset) << "\n";
    }

    // Generate items based on mode
    if (args.mode == RunMode::Batch)
    {
        return ItemGenerator::GenerateBatch(args);
    }
    else if (args.mode == RunMode::LLM)
    {
        return ItemGenerator::GenerateWithLLM(args);
    }
    else
    {
        return ItemGenerator::GenerateDummy(args);
    }
}
