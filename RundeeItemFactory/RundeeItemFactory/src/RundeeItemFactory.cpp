// ===============================
// Project Name: RundeeItemFactory
// File Name: main.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Entry point for LLM/dummy-based food item generation pipeline.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>

#include "ItemFoodGenerator.h"
#include "FoodJsonWriter.h"
#include "FoodJsonParser.h"
#include "OllamaClient.h"
#include "json.hpp"

using nlohmann::json;

// 실행 모드
enum class RunMode
{
    Dummy,
    LLM
};

bool SaveTextFile(const std::string& path, const std::string& text)
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

// LLM용 프롬프트 빌더
std::string BuildFoodJsonPrompt(const FoodGenerateParams& params)
{
    std::string prompt;
    prompt += "You are a game designer assistant for a survival game.\n\n";
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";

    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " food items for an early-game survival setting.\n\n";

    prompt += R"(Rules:
- Use this JSON schema EXACTLY for each item:
{
  "id": string (unique, lowercase, underscore),
  "displayName": string,
  "category": "Food" or "Drink",
  "rarity": "Common" | "Uncommon" | "Rare",
  "maxStack": integer,

  "hungerRestore": integer,
  "thirstRestore": integer,
  "healthRestore": integer,

  "spoils": boolean,
  "spoilTimeMinutes": integer,

  "description": string
}

Balancing:
- Snacks: hungerRestore 5-15, thirstRestore 0-5
- Light meals: hungerRestore 15-30, thirstRestore 0-10
- Drinks: hungerRestore 0-10, thirstRestore 15-40
- All values must be between 0 and 100.
- Use only integers for all restore values.

Output:
- Output ONLY a JSON array of items.
- No comments, no extra text, no Markdown, no explanation.
)";

    return prompt;
}

int main(int argc, char** argv)
{
    RunMode mode = RunMode::Dummy;          // 기본값: 더미 모드
    std::string modelName = "llama3";       // 기본 모델 이름

    FoodGenerateParams params;
    params.count = 5;
    params.maxHunger = 100;
    params.maxThirst = 100;
    params.outputPath = "items_food.json";

    // ----- 커맨드라인 인자 파싱 -----
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--mode" && i + 1 < argc)
        {
            std::string m = argv[++i];
            if (m == "llm")
            {
                mode = RunMode::LLM;
            }
            else if (m == "dummy")
            {
                mode = RunMode::Dummy;
            }
            else
            {
                std::cout << "[Warning] Unknown mode: " << m
                    << " (use 'dummy' or 'llm')\n";
            }
        }
        else if (arg == "--model" && i + 1 < argc)
        {
            modelName = argv[++i];
        }
        else if (arg == "--count" && i + 1 < argc)
        {
            params.count = std::atoi(argv[++i]);
        }
        else if (arg == "--maxHunger" && i + 1 < argc)
        {
            params.maxHunger = std::atoi(argv[++i]);
        }
        else if (arg == "--maxThirst" && i + 1 < argc)
        {
            params.maxThirst = std::atoi(argv[++i]);
        }
        else if (arg == "--out" && i + 1 < argc)
        {
            params.outputPath = argv[++i];
        }
        else
        {
            std::cout << "[Warning] Unknown or incomplete argument: " << arg << "\n";
        }
    }

    std::cout << "[Main] Mode = "
        << (mode == RunMode::LLM ? "LLM" : "Dummy")
        << ", model = " << modelName
        << ", count = " << params.count
        << ", out = " << params.outputPath << "\n";

    // ==============================
    // LLM 모드
    // ==============================
    if (mode == RunMode::LLM)
    {
        std::cout << "[Main] Running in LLM mode.\n";

        std::string prompt = BuildFoodJsonPrompt(params);
        std::string jsonResponse = OllamaClient::RunSimple(modelName, prompt);

        std::cout << "=== Ollama Food JSON Response ===\n";
        std::cout << jsonResponse << "\n";

        if (jsonResponse.empty())
        {
            std::cout << "[Main] LLM response is empty.\n";
            return 1;
        }

        // raw JSON 저장 (out 이름 기준으로)
        std::string rawPath = params.outputPath + ".raw.json";
        if (SaveTextFile(rawPath, jsonResponse))
        {
            std::cout << "[Main] Saved LLM raw JSON to " << rawPath << "\n";
        }

        // JSON → ItemFoodData 파싱
        std::vector<ItemFoodData> llmItems;
        if (!FoodJsonParser::ParseFromJsonText(jsonResponse, llmItems))
        {
            std::cout << "[Main] Failed to parse LLM JSON.\n";
            return 1;
        }

        std::cout << "=== Parsed Items From LLM (" << llmItems.size() << ") ===\n";
        for (const auto& item : llmItems)
        {
            std::cout << "- " << item.id
                << " / " << item.displayName
                << " / category: " << item.category
                << " / Hunger +" << item.hungerRestore
                << ", Thirst +" << item.thirstRestore
                << "\n";
        }

        // 최종 결과를 outputPath로 저장
        if (!FoodJsonWriter::WriteToFile(llmItems, params.outputPath))
        {
            std::cout << "[Main] Failed to write LLM-generated JSON file.\n";
            return 1;
        }

        std::cout << "[Main] Wrote LLM-generated items to " << params.outputPath << "\n";
    }
    // ==============================
    // Dummy 모드
    // ==============================
    else
    {
        std::cout << "[Main] Running in Dummy mode.\n";

        ItemFoodGenerator generator;
        std::vector<ItemFoodData> items = generator.GenerateDummy(params);

        std::cout << "=== Generated Dummy Food Items (" << items.size() << ") ===\n";
        for (const auto& item : items)
        {
            std::cout << "- " << item.displayName
                << " (Hunger +" << item.hungerRestore
                << ", Thirst +" << item.thirstRestore << ")\n";
        }

        if (!params.outputPath.empty())
        {
            if (!FoodJsonWriter::WriteToFile(items, params.outputPath))
            {
                std::cout << "[Main] Failed to write dummy JSON file.\n";
                return 1;
            }
        }
    }

    return 0;
}
