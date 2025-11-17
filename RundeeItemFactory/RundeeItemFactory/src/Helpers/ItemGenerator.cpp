// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemGenerator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of high-level item generation.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Helpers/ItemGenerator.h"
#include "Parsers/ItemJsonParser.h"
#include "Writers/ItemJsonWriter.h"
#include "Prompts/PromptBuilder.h"
#include "Clients/OllamaClient.h"
#include "Generators/ItemFoodGenerator.h"
#include <iostream>
#include <fstream>

static bool SaveTextFile(const std::string& path, const std::string& text)
{
    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[SaveTextFile] Failed to open file: " << path << "\n";
        return false;
    }
    ofs << text;
    return true;
}

static int ProcessLLMResponse_Food(const std::string& jsonResponse, const std::string& outputPath)
{
    if (jsonResponse.empty())
    {
        std::cout << "[ItemGenerator] LLM response is empty.\n";
        return 1;
    }

    std::string rawPath = outputPath + ".raw.json";
    if (SaveTextFile(rawPath, jsonResponse))
    {
        std::cout << "[ItemGenerator] Saved LLM raw JSON to " << rawPath << "\n";
    }

    std::vector<ItemFoodData> items;
    if (!ItemJsonParser::ParseFoodFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Items From LLM (" << items.size() << ") ===\n";
    for (const auto& item : items)
    {
        std::cout << "- " << item.id
            << " / " << item.displayName
            << " / category: " << item.category
            << " / Hunger +" << item.hungerRestore
            << ", Thirst +" << item.thirstRestore
            << "\n";
    }

    if (!ItemJsonWriter::WriteFoodToFile(items, outputPath))
    {
        std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
        return 1;
    }

    std::cout << "[ItemGenerator] Wrote LLM-generated items to " << outputPath << "\n";
    return 0;
}

static int ProcessLLMResponse_Drink(const std::string& jsonResponse, const std::string& outputPath)
{
    if (jsonResponse.empty())
    {
        std::cout << "[ItemGenerator] LLM response is empty.\n";
        return 1;
    }

    std::string rawPath = outputPath + ".raw.json";
    if (SaveTextFile(rawPath, jsonResponse))
    {
        std::cout << "[ItemGenerator] Saved LLM raw JSON to " << rawPath << "\n";
    }

    std::vector<ItemDrinkData> items;
    if (!ItemJsonParser::ParseDrinkFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Items From LLM (" << items.size() << ") ===\n";
    for (const auto& item : items)
    {
        std::cout << "- " << item.id
            << " / " << item.displayName
            << " / category: " << item.category
            << " / Hunger +" << item.hungerRestore
            << ", Thirst +" << item.thirstRestore
            << "\n";
    }

    if (!ItemJsonWriter::WriteDrinkToFile(items, outputPath))
    {
        std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
        return 1;
    }

    std::cout << "[ItemGenerator] Wrote LLM-generated items to " << outputPath << "\n";
    return 0;
}

static int ProcessLLMResponse_Material(const std::string& jsonResponse, const std::string& outputPath)
{
    if (jsonResponse.empty())
    {
        std::cout << "[ItemGenerator] LLM response is empty.\n";
        return 1;
    }

    std::string rawPath = outputPath + ".raw.json";
    if (SaveTextFile(rawPath, jsonResponse))
    {
        std::cout << "[ItemGenerator] Saved LLM raw JSON to " << rawPath << "\n";
    }

    std::vector<ItemMaterialData> items;
    if (!ItemJsonParser::ParseMaterialFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse material JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Material Items (" << items.size() << ") ===\n";
    for (const auto& item : items)
    {
        std::cout << "- " << item.id
            << " / " << item.displayName
            << " / category: " << item.category
            << " / materialType: " << item.materialType
            << " / value: " << item.value
            << "\n";
    }

    if (!ItemJsonWriter::WriteMaterialToFile(items, outputPath))
    {
        std::cout << "[ItemGenerator] Failed to write material JSON file.\n";
        return 1;
    }

    std::cout << "[ItemGenerator] Wrote material items to " << outputPath << "\n";
    return 0;
}

namespace ItemGenerator
{
    int GenerateWithLLM(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in LLM mode.\n";

        std::string prompt;
        std::string jsonResponse;

        // Build prompt and get LLM response
        if (args.itemType == ItemType::Food)
        {
            prompt = PromptBuilder::BuildFoodJsonPrompt(args.params, args.preset);
            std::cout << "=== Ollama Food JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Drink)
        {
            prompt = PromptBuilder::BuildDrinkJsonPrompt(args.params, args.preset);
            std::cout << "=== Ollama Drink JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Material)
        {
            prompt = PromptBuilder::BuildMaterialJsonPrompt(args.params, args.preset);
            std::cout << "=== Ollama Material JSON Response ===\n";
        }
        else
        {
            std::cout << "[ItemGenerator] Unknown item type.\n";
            return 1;
        }

        jsonResponse = OllamaClient::RunSimple(args.modelName, prompt);
        std::cout << jsonResponse << "\n";

        // Process response based on item type
        if (args.itemType == ItemType::Food)
        {
            return ProcessLLMResponse_Food(jsonResponse, args.params.outputPath);
        }
        else if (args.itemType == ItemType::Drink)
        {
            return ProcessLLMResponse_Drink(jsonResponse, args.params.outputPath);
        }
        else if (args.itemType == ItemType::Material)
        {
            return ProcessLLMResponse_Material(jsonResponse, args.params.outputPath);
        }

        return 1;
    }

    int GenerateDummy(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in Dummy mode.\n";

        if (args.itemType == ItemType::Food)
        {
            ItemFoodGenerator generator;
            std::vector<ItemFoodData> items = generator.GenerateDummy(args.params);

            std::cout << "=== Generated Dummy Food Items (" << items.size() << ") ===\n";
            for (const auto& item : items)
            {
                std::cout << "- " << item.displayName
                    << " (Hunger +" << item.hungerRestore
                    << ", Thirst +" << item.thirstRestore << ")\n";
            }

            if (!args.params.outputPath.empty())
            {
                if (!ItemJsonWriter::WriteFoodToFile(items, args.params.outputPath))
                {
                    std::cout << "[ItemGenerator] Failed to write dummy JSON file.\n";
                    return 1;
                }
            }
        }
        else
        {
            std::cout << "[ItemGenerator] Dummy mode for " 
                << CommandLineParser::GetItemTypeName(args.itemType) 
                << " is not implemented yet.\n";
        }

        return 0;
    }
}


