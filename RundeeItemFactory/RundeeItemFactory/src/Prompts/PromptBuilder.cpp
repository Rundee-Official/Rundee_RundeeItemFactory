// ===============================
// Project Name: RundeeItemFactory
// File Name: PromptBuilder.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of prompt building for LLM.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Prompts/PromptBuilder.h"

std::string PromptBuilder::GetPresetFlavorText(PresetType preset)
{
    switch (preset)
    {
    case PresetType::Forest:
        return
            "World context:\n"
            "- The setting is a temperate forest with plenty of bushes, mushrooms, and small animals.\n"
            "- Early-game items should focus on berries, nuts, roots, and simple cooked meals.\n"
            "- Water sources are streams and rainwater.\n\n";

    case PresetType::Desert:
        return
            "World context:\n"
            "- The setting is a harsh desert with scarce vegetation and limited water.\n"
            "- Food items should be low in quantity but efficient, like dried meat or cactus fruit.\n"
            "- Drinks are highly valuable and restore more thirst.\n"
            "- Emphasize scarcity and survival under heat.\n\n";

    case PresetType::Coast:
        return
            "World context:\n"
            "- The setting is a coastal island with beaches, palm trees, and shallow seawater.\n"
            "- Food comes from coconuts, fish, shellfish, and tropical fruits.\n"
            "- Drinks are mainly coconut water or boiled water.\n"
            "- Slightly higher thirst restoration for drinks.\n\n";

    case PresetType::City:
        return
            "World context:\n"
            "- The setting is an abandoned modern city with convenience stores, vending machines, and supermarkets.\n"
            "- Most food is processed: canned food, instant noodles, snacks, and energy bars.\n"
            "- Drinks are bottled water, soft drinks, and energy drinks with higher thirst restoration.\n"
            "- Many items do not spoil quickly, but some fresh food can still be found.\n\n";

    case PresetType::Default:
    default:
        return
            "World context:\n"
            "- Generic early-game survival environment with moderate resources.\n"
            "- Items should feel simple and grounded, not magical or high-tech.\n\n";
    }
}

std::string PromptBuilder::BuildFoodJsonPrompt(const FoodGenerateParams& params, PresetType preset)
{
    std::string prompt;

    // 1) 프리셋 세계관 컨텍스트 추가
    prompt += GetPresetFlavorText(preset);

    // 2) 플레이어 파라미터 설명
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";

    // 3) 작업 설명
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " food-related items ";
    prompt += "for an early-game survival setting.\n\n";

    // 4) JSON Rules/Schema/Output 텍스트를 여기서 정의하고 그대로 출력하도록
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

std::string PromptBuilder::BuildDrinkJsonPrompt(const FoodGenerateParams& params, PresetType preset)
{
    std::string prompt;

    // 1) 프리셋 세계관 컨텍스트 추가
    prompt += GetPresetFlavorText(preset);

    // 2) 플레이어 파라미터 설명
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";

    // 3) 작업 설명
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " drink-related items ";
    prompt += "for an early-game survival setting.\n\n";

    // 4) JSON Rules/Schema/Output 텍스트
    prompt += R"(Rules:
- Use this JSON schema EXACTLY for each item:
{
  "id": string (unique, lowercase, underscore),
  "displayName": string,
  "category": "Drink",
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
- Drinks: hungerRestore 0-10, thirstRestore 15-40
- Health drinks: thirstRestore 20-40, healthRestore 5-20
- All values must be between 0 and 100.
- Use only integers for all restore values.
- thirstRestore should be higher than hungerRestore for drinks.

Output:
- Output ONLY a JSON array of items.
- No comments, no extra text, no Markdown, no explanation.
)";

    return prompt;
}

std::string PromptBuilder::BuildMaterialJsonPrompt(const FoodGenerateParams& params, PresetType preset)
{
    std::string prompt;

    // 1) 프리셋 세계관 컨텍스트 추가
    prompt += GetPresetFlavorText(preset);

    // 2) 작업 설명
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " crafting materials and junk items ";
    prompt += "for this survival setting.\n\n";

    // 3) 스키마 / 규칙 정의
    prompt += R"(Item type:
- These items are used for crafting, building, or scrapping.
- Do NOT create food, drink, weapons, or medicine in this preset.
- Focus on materials like wood planks, scrap metal, nails, screws, springs, electronic boards, wires, cloth, etc.

Use this JSON schema EXACTLY for each item:
{
  "id": string (unique, lowercase, underscore),
  "displayName": string,

  "category": "Material" | "Junk" | "Component",
  "rarity": "Common" | "Uncommon" | "Rare",
  "maxStack": integer,

  "materialType": "Wood" | "Metal" | "Plastic" | "Fabric" | "Glass" | "Electronic" | "Stone" | "Rubber",

  "hardness": integer,      // 0-100, structural strength
  "flammability": integer,  // 0-100, how well it burns
  "value": integer,         // 0-100, scrap/trade value

  "description": string
}

Balancing rules:
- Common items: low to medium value (value 5-40), simple materials.
- Uncommon items: better structural stats or higher value (value 25-70).
- Rare items: high value or very useful components (value 50-100).
- hardness, flammability, and value must all be in the range 0-100.
- maxStack should be between 1 and 100.
- description must NOT be empty and should be 1 short sentence (5-20 words)
  that explains what the item looks like and how it is used.

Output:
- Output ONLY a JSON array of items.
- No comments, no extra text, no Markdown, no explanation.
)";

    return prompt;
}


