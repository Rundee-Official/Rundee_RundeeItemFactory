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
#include "Prompts/CustomPreset.h"
#include <set>
#include <sstream>

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

std::string PromptBuilder::GetPresetFlavorText(const CustomPreset& customPreset)
{
    return CustomPresetManager::GetPresetFlavorText(customPreset);
}

std::string PromptBuilder::BuildFoodJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(preset);

    // 2) Describe player parameters
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";

    // 3) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " food-related items ";
    prompt += "for an early-game survival setting.\n";
    
    // Add exclusion list if provided
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20) // Limit to first 20 to avoid prompt bloat
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";

    // 4) Define JSON Rules/Schema/Output text here and output as-is
    prompt += "Rules:\n";
    prompt += "- Use this JSON schema EXACTLY for each item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Food\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"hungerRestore\": integer,\n";
    prompt += "  \"thirstRestore\": integer,\n";
    prompt += "  \"healthRestore\": integer,\n";
    prompt += "\n";
    prompt += "  \"spoils\": boolean,\n";
    prompt += "  \"spoilTimeMinutes\": integer,\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "IMPORTANT:\n";
    prompt += "- category MUST be \"Food\" (NOT \"Drink\").\n";
    prompt += "- Generate ONLY food items, not drinks.\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that describes what the food looks like and its taste.\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Snacks: hungerRestore 5-15, thirstRestore 0-5\n";
    prompt += "- Light meals: hungerRestore 15-30, thirstRestore 0-10\n";
    prompt += "- Heavy meals: hungerRestore 30-50, thirstRestore 0-10\n";
    prompt += "- All values must be between 0 and 100.\n";
    prompt += "- Use only integers for all restore values.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildFoodJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(customPreset);

    // 2) Describe player parameters
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";

    // 3) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " food-related items ";
    prompt += "for an early-game survival setting.\n";
    
    // Add exclusion list if provided
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20) // Limit to first 20 to avoid prompt bloat
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";

    // 4) Define JSON Rules/Schema/Output text here and output as-is
    prompt += "Rules:\n";
    prompt += "- Use this JSON schema EXACTLY for each item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Food\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"hungerRestore\": integer,\n";
    prompt += "  \"thirstRestore\": integer,\n";
    prompt += "  \"healthRestore\": integer,\n";
    prompt += "\n";
    prompt += "  \"spoils\": boolean,\n";
    prompt += "  \"spoilTimeMinutes\": integer,\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing rules:\n";
    prompt += "- Common items: hungerRestore 5-25, thirstRestore 0-10, healthRestore 0-5.\n";
    prompt += "- Uncommon items: hungerRestore 20-40, thirstRestore 5-15, healthRestore 0-10.\n";
    prompt += "- Rare items: hungerRestore 35-60, thirstRestore 10-25, healthRestore 5-20.\n";
    prompt += "- Total power (hunger + thirst + health) should not exceed 80 for Common, 100 for Uncommon, 120 for Rare.\n";
    prompt += "- Items that spoil should have spoilTimeMinutes between 30 and 720 (30 minutes to 12 hours).\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildDrinkJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(preset);

    // 2) Describe player parameters
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";

    // 3) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " drink-related items ";
    prompt += "for an early-game survival setting.\n";
    
    // Add exclusion list if provided
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20) // Limit to first 20 to avoid prompt bloat
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";

    // 4) JSON Rules/Schema/Output text
    prompt += "Rules:\n";
    prompt += "- Use this JSON schema EXACTLY for each item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Drink\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"hungerRestore\": integer,\n";
    prompt += "  \"thirstRestore\": integer,\n";
    prompt += "  \"healthRestore\": integer,\n";
    prompt += "\n";
    prompt += "  \"spoils\": boolean,\n";
    prompt += "  \"spoilTimeMinutes\": integer,\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "IMPORTANT:\n";
    prompt += "- category MUST be \"Drink\" (NOT \"Food\").\n";
    prompt += "- Generate ONLY drink items, not food.\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that describes what the drink looks like and its taste.\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Drinks: hungerRestore 0-10, thirstRestore 15-40\n";
    prompt += "- Health drinks: thirstRestore 20-40, healthRestore 5-20\n";
    prompt += "- All values must be between 0 and 100.\n";
    prompt += "- Use only integers for all restore values.\n";
    prompt += "- thirstRestore should be higher than hungerRestore for drinks.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildDrinkJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds)
{
    std::string prompt;
    prompt += GetPresetFlavorText(customPreset);
    prompt += "The player has:\n";
    prompt += "- maxHunger = " + std::to_string(params.maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(params.maxThirst) + "\n\n";
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " drink-related items ";
    prompt += "for an early-game survival setting.\n";
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";
    prompt += "Rules:\n";
    prompt += "- Use this JSON schema EXACTLY for each item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Drink\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"hungerRestore\": integer,\n";
    prompt += "  \"thirstRestore\": integer,\n";
    prompt += "  \"healthRestore\": integer,\n";
    prompt += "\n";
    prompt += "  \"spoils\": boolean,\n";
    prompt += "  \"spoilTimeMinutes\": integer,\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing rules:\n";
    prompt += "- Common items: hungerRestore 0-5, thirstRestore 10-30, healthRestore 0-5.\n";
    prompt += "- Uncommon items: hungerRestore 0-10, thirstRestore 25-50, healthRestore 0-10.\n";
    prompt += "- Rare items: hungerRestore 0-15, thirstRestore 40-70, healthRestore 5-20.\n";
    prompt += "- Total power (hunger + thirst + health) should not exceed 80 for Common, 100 for Uncommon, 120 for Rare.\n";
    prompt += "- Items that spoil should have spoilTimeMinutes between 30 and 720.\n";
    prompt += "- Use only integers for all restore values.\n";
    prompt += "- thirstRestore should be higher than hungerRestore for drinks.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";
    return prompt;
}

std::string PromptBuilder::BuildMaterialJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(preset);

    // 2) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " crafting materials and junk items ";
    prompt += "for this survival setting.\n\n";

    // 3) Define schema / rules
    prompt += "Item type:\n";
    prompt += "- These items are used for crafting, building, or scrapping.\n";
    prompt += "- Do NOT create food, drink, weapons, or medicine in this preset.\n";
    prompt += "- Focus on materials like wood planks, scrap metal, nails, screws, springs, electronic boards, wires, cloth, etc.\n";
    prompt += "\n";
    prompt += "Use this JSON schema EXACTLY for each item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "\n";
    prompt += "  \"category\": \"Material\" | \"Junk\" | \"Component\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"materialType\": \"Wood\" | \"Metal\" | \"Plastic\" | \"Fabric\" | \"Glass\" | \"Electronic\" | \"Stone\" | \"Rubber\",\n";
    prompt += "\n";
    prompt += "  \"hardness\": integer,      // 0-100, structural strength\n";
    prompt += "  \"flammability\": integer,  // 0-100, how well it burns\n";
    prompt += "  \"value\": integer,         // 0-100, scrap/trade value\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing rules:\n";
    prompt += "- Common items: low to medium value (value 5-40), simple materials.\n";
    prompt += "- Uncommon items: better structural stats or higher value (value 25-70).\n";
    prompt += "- Rare items: high value or very useful components (value 50-100).\n";
    prompt += "- hardness, flammability, and value must all be in the range 0-100.\n";
    prompt += "- maxStack should be between 1 and 100.\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that explains what the item looks like and how it is used.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildMaterialJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds)
{
    std::string prompt;
    prompt += GetPresetFlavorText(customPreset);
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " crafting materials and junk items ";
    prompt += "for this survival setting.\n\n";
    prompt += "Item type:\n";
    prompt += "- These items are used for crafting, building, or scrapping.\n";
    prompt += "- Do NOT create food, drink, weapons, or medicine in this preset.\n";
    prompt += "- Focus on materials like wood planks, scrap metal, nails, screws, springs, electronic boards, wires, cloth, etc.\n";
    prompt += "\n";
    prompt += "Use this JSON schema EXACTLY for each item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "\n";
    prompt += "  \"category\": \"Material\" | \"Junk\" | \"Component\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"materialType\": \"Wood\" | \"Metal\" | \"Plastic\" | \"Fabric\" | \"Glass\" | \"Electronic\" | \"Stone\" | \"Rubber\",\n";
    prompt += "\n";
    prompt += "  \"hardness\": integer,      // 0-100, structural strength\n";
    prompt += "  \"flammability\": integer,  // 0-100, how well it burns\n";
    prompt += "  \"value\": integer,         // 0-100, scrap/trade value\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing rules:\n";
    prompt += "- Common items: low to medium value (value 5-40), simple materials.\n";
    prompt += "- Uncommon items: better structural stats or higher value (value 25-70).\n";
    prompt += "- Rare items: high value or very useful components (value 50-100).\n";
    prompt += "- hardness, flammability, and value must all be in the range 0-100.\n";
    prompt += "- maxStack should be between 1 and 100.\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that explains what the item looks like and how it is used.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";
    return prompt;
}

std::string PromptBuilder::BuildWeaponJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(preset);

    // 2) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " weapon items ";
    prompt += "for this survival setting.\n\n";

    // 3) Add exclusion list if provided
    if (!excludeIds.empty())
    {
        prompt += "IMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }

    // 4) Define JSON schema
    prompt += "Use this JSON schema EXACTLY for each weapon:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Weapon\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer (usually 1 for weapons),\n";
    prompt += "\n";
    prompt += "  \"weaponCategory\": \"Ranged\" | \"Melee\",  // Ranged = uses ammo, Melee = no ammo\n";
    prompt += "\n";
    prompt += "  \"weaponType\": string,\n";
    prompt += "    // For Ranged: \"AssaultRifle\", \"SMG\", \"Pistol\", \"SniperRifle\", \"Shotgun\", \"LMG\", \"DMR\"\n";
    prompt += "    // For Melee: \"Sword\", \"Axe\", \"Knife\", \"Mace\", \"Spear\", \"Club\", \"Hammer\", \"Blade\", etc.\n";
    prompt += "\n";
    prompt += "  \"caliber\": string,  // For Ranged: \"9mm\", \"5.56mm\", \"7.62mm\", \"12gauge\", etc. | For Melee: empty string\n";
    prompt += "\n";
    prompt += "  \"minDamage\": integer,      // 0-100, minimum damage per hit\n";
    prompt += "  \"maxDamage\": integer,      // 0-100, maximum damage per hit\n";
    prompt += "    // For Ranged: Base damage (actual = base + ammo.damageBonus)\n";
    prompt += "    // For Melee: Actual damage (no ammo modifier)\n";
    prompt += "\n";
    prompt += "  \"fireRate\": integer,       // For Ranged: 0-1200, rounds per minute | For Melee: 0-300, attacks per minute\n";
    prompt += "  \"accuracy\": integer,       // 0-100, base accuracy (Ranged) or hit chance (Melee)\n";
    prompt += "  \"recoil\": integer,         // 0-100, recoil control (Ranged only, 0 for Melee)\n";
    prompt += "  \"ergonomics\": integer,     // 0-100, handling (higher is better)\n";
    prompt += "  \"weight\": integer,         // Weight in grams\n";
    prompt += "  \"durability\": integer,     // 0-100, weapon condition\n";
    prompt += "\n";
    prompt += "  // Ranged weapon stats (set to 0 for Melee)\n";
    prompt += "  \"muzzleVelocity\": integer, // Muzzle velocity in m/s (Ranged only, typical: 300-1000)\n";
    prompt += "  \"effectiveRange\": integer, // Effective range in meters (Ranged only, typical: 50-800)\n";
    prompt += "  \"penetrationPower\": integer, // 0-100, armor penetration capability (Ranged only)\n";
    prompt += "\n";
    prompt += "  // Melee weapon stats (set to 0 for Ranged)\n";
    prompt += "  \"attackSpeed\": integer,    // Attacks per second (Melee only, typical: 1-5)\n";
    prompt += "  \"reach\": integer,           // Reach in meters (Melee only, typical: 0.5-3.0, store as cm: 50-300)\n";
    prompt += "  \"staminaCost\": integer,    // Stamina cost per attack (Melee only, 0-100)\n";
    prompt += "\n";
    prompt += "  \"moddingSlots\": integer,   // Number of available modding slots (typical: 3-8)\n";
    prompt += "\n";
    prompt += "  \"attachmentSlots\": [\n";
    prompt += "    {\n";
    prompt += "      \"slotType\": string,    // \"Muzzle\", \"Grip\", \"Sight\", \"Scope\", \"Stock\", \"Barrel\", \"Handguard\", \"Rail\", \"Magazine\", \"GasBlock\", \"ChargingHandle\", \"PistolGrip\", \"Foregrip\", etc.\n";
    prompt += "      \"slotIndex\": integer,   // For multiple slots of same type (default 0)\n";
    prompt += "      \"isRequired\": boolean  // Whether this slot must be filled (default false)\n";
    prompt += "    }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  NOTE: Magazine is now a separate WeaponComponent. Do NOT include magazineCapacity or magazineType in weapon data.\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "IMPORTANT:\n";
    prompt += "- category MUST be \"Weapon\".\n";
    prompt += "- weaponCategory MUST be either \"Ranged\" or \"Melee\".\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that describes the weapon's appearance and characteristics.\n";
    prompt += "\n";
    prompt += "Ranged Weapons:\n";
    prompt += "- Use ammo (caliber must match ammo caliber).\n";
    prompt += "- Damage is base damage + ammo.damageBonus (base damage should be low, 5-20).\n";
    prompt += "- Must have caliber, muzzleVelocity, effectiveRange, penetrationPower.\n";
    prompt += "- attachmentSlots should include: Muzzle, Grip, Sight, Scope, Stock, Barrel, Handguard, Rail, Magazine, GasBlock, ChargingHandle, PistolGrip, Foregrip, etc.\n";
    prompt += "- Magazine slot is REQUIRED (type: \"Magazine\").\n";
    prompt += "- Tarkov-style detail: Include realistic slots like GasBlock, ChargingHandle, PistolGrip for assault rifles.\n";
    prompt += "\n";
    prompt += "Melee Weapons:\n";
    prompt += "- Do NOT use ammo (caliber should be empty string).\n";
    prompt += "- Damage is actual damage (no ammo modifier, typical: 30-80).\n";
    prompt += "- Must have attackSpeed, reach, staminaCost.\n";
    prompt += "- Set muzzleVelocity, effectiveRange, penetrationPower, recoil to 0.\n";
    prompt += "- attachmentSlots are optional (e.g., Grip, Pommel for swords).\n";
    prompt += "- Magazine slot should NOT be included.\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Common weapons: lower damage, basic stats, simple design, fewer modding slots (3-4).\n";
    prompt += "- Uncommon weapons: medium damage, better stats, more attachment slots, moderate modding slots (5-6).\n";
    prompt += "- Rare weapons: high damage, excellent stats, many attachment slots, high modding slots (7-8).\n";
    prompt += "\n";
    prompt += "Ranged Weapon Examples:\n";
    prompt += "- Assault rifles: fireRate 600-900, base damage 5-15, muzzleVelocity 700-900 m/s, effectiveRange 300-500m.\n";
    prompt += "- SMGs: fireRate 700-1000, base damage 3-10, muzzleVelocity 300-500 m/s, effectiveRange 100-200m.\n";
    prompt += "- Pistols: fireRate 400-600, base damage 2-8, muzzleVelocity 300-450 m/s, effectiveRange 50-100m.\n";
    prompt += "- Sniper rifles: fireRate 20-60, base damage 10-20, muzzleVelocity 800-1000 m/s, effectiveRange 600-800m.\n";
    prompt += "- Shotguns: fireRate 60-120, base damage 8-15, muzzleVelocity 300-400 m/s, effectiveRange 30-50m.\n";
    prompt += "- DMRs: fireRate 200-400, base damage 8-15, muzzleVelocity 700-900 m/s, effectiveRange 400-600m.\n";
    prompt += "\n";
    prompt += "Melee Weapon Examples:\n";
    prompt += "- Swords: damage 40-60, attackSpeed 2-4, reach 100-150cm, staminaCost 10-20, moderate weight.\n";
    prompt += "- Axes: damage 50-70, attackSpeed 1-2, reach 80-120cm, staminaCost 20-30, heavy weight.\n";
    prompt += "- Knives: damage 20-35, attackSpeed 3-5, reach 30-50cm, staminaCost 5-10, light weight.\n";
    prompt += "- Spears: damage 45-65, attackSpeed 2-3, reach 150-250cm, staminaCost 15-25, moderate weight.\n";
    prompt += "- Clubs/Maces: damage 55-75, attackSpeed 1-2, reach 70-100cm, staminaCost 25-35, heavy weight.\n";
    prompt += "\n";
    prompt += "- All stat values must be within their specified ranges.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of weapons.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildWeaponJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds)
{
    std::string prompt;
    prompt += GetPresetFlavorText(customPreset);
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " weapons ";
    prompt += "for this survival setting.\n";
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";
    prompt += "Use this JSON schema EXACTLY for each weapon:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Weapon\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer (usually 1),\n";
    prompt += "\n";
    prompt += "  \"weaponCategory\": \"Ranged\" | \"Melee\",\n";
    prompt += "  \"weaponType\": string,\n";
    prompt += "  \"caliber\": string (empty for Melee),\n";
    prompt += "\n";
    prompt += "  \"minDamage\": integer,\n";
    prompt += "  \"maxDamage\": integer,\n";
    prompt += "  \"fireRate\": integer,\n";
    prompt += "  \"accuracy\": integer (0-100),\n";
    prompt += "  \"recoil\": integer (0-100, lower is better, Ranged only),\n";
    prompt += "  \"ergonomics\": integer (0-100, higher is better),\n";
    prompt += "  \"weight\": integer (grams),\n";
    prompt += "  \"durability\": integer (0-100),\n";
    prompt += "\n";
    prompt += "  \"muzzleVelocity\": integer (m/s, Ranged only, 0 for Melee),\n";
    prompt += "  \"effectiveRange\": integer (meters, Ranged only, 0 for Melee),\n";
    prompt += "  \"penetrationPower\": integer (0-100, Ranged only),\n";
    prompt += "\n";
    prompt += "  \"attackSpeed\": integer (attacks per second, Melee only, 0 for Ranged),\n";
    prompt += "  \"reach\": integer (meters, Melee only, 0 for Ranged),\n";
    prompt += "  \"staminaCost\": integer (0-100, Melee only),\n";
    prompt += "\n";
    prompt += "  \"moddingSlots\": integer,\n";
    prompt += "  \"attachmentSlots\": [\n";
    prompt += "    { \"slotType\": string, \"slotIndex\": integer, \"hasBuiltInRail\": boolean }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing rules:\n";
    prompt += "- Ranged weapons: base damage (actual = base + ammo.damageBonus), must have caliber.\n";
    prompt += "- Melee weapons: actual damage (no ammo modifier), no caliber, no ranged stats.\n";
    prompt += "- Common: damage 5-15 (Ranged) or 10-20 (Melee), basic stats.\n";
    prompt += "- Uncommon: damage 10-25 (Ranged) or 15-30 (Melee), better stats.\n";
    prompt += "- Rare: damage 15-35 (Ranged) or 20-40 (Melee), excellent stats.\n";
    prompt += "- All stat values must be within their specified ranges.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of weapons.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";
    return prompt;
}

std::string PromptBuilder::BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(preset);

    // 2) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " weapon attachment components ";
    prompt += "for this survival setting.\n\n";

    // 3) Add exclusion list if provided
    if (!excludeIds.empty())
    {
        prompt += "IMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }

    // 4) Define JSON schema
    prompt += "Use this JSON schema EXACTLY for each component:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"WeaponComponent\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer (usually 1 for components),\n";
    prompt += "\n";
    prompt += "  \"componentType\": \"Muzzle\" | \"Grip\" | \"Sight\" | \"Scope\" | \"Stock\" | \"Barrel\" | \"Handguard\" | \"Rail\" | \"Flashlight\" | \"Laser\" | \"Magazine\" | \"GasBlock\" | \"ChargingHandle\" | \"PistolGrip\" | \"Foregrip\" | \"Bipod\" | \"TacticalDevice\" | \"Mount\" | \"Adapter\",\n";
    prompt += "\n";
    prompt += "  // For Magazine components ONLY:\n";
    prompt += "  \"magazineCapacity\": integer,    // Capacity in rounds (ONLY for Magazine type, typical: 10-100)\n";
    prompt += "  \"caliber\": string,              // Compatible caliber (ONLY for Magazine type, e.g., \"9mm\", \"5.56mm\")\n";
    prompt += "  \"magazineType\": string,         // \"Standard\", \"Extended\", \"Drum\", \"QuadStack\" (ONLY for Magazine type)\n";
    prompt += "  \"loadedRounds\": [               // OPTIONAL preview of actual load order (top to bottom)\n";
    prompt += "    {\n";
    prompt += "      \"orderIndex\": integer,      // 0 = first rounds to fire, increasing deeper into mag\n";
    prompt += "      \"roundCount\": integer,      // How many consecutive rounds use this ammo type (1- magazineCapacity)\n";
    prompt += "      \"ammoId\": string,           // Reference ammo ID (must exist in Ammo dataset)\n";
    prompt += "      \"ammoDisplayName\": string,  // Friendly name (e.g., \"7.62x39 AP\")\n";
    prompt += "      \"ammoNotes\": string         // Short descriptor (\"AP\", \"FMJ\", \"Tracer\")\n";
    prompt += "    }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"compatibleSlots\": [string],  // Which weapon slots this can attach to, e.g., [\"Muzzle\", \"Barrel\"] or [\"Magazine\"]\n";
    prompt += "\n";
    prompt += "  \"subSlots\": [\n";
    prompt += "    {\n";
    prompt += "      \"slotType\": string,        // \"Muzzle\", \"Grip\", \"Sight\", \"Rail\", etc.\n";
    prompt += "      \"slotIndex\": integer,       // For multiple slots of same type (default 0)\n";
    prompt += "      \"hasBuiltInRail\": boolean   // If true, this slot already has a rail (default false)\n";
    prompt += "    }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"damageModifier\": integer,      // Damage bonus/penalty (can be negative, range -50 to 50)\n";
    prompt += "  \"recoilModifier\": integer,     // Recoil reduction (positive = less recoil, can be negative, range -50 to 50)\n";
    prompt += "  \"ergonomicsModifier\": integer, // Ergonomics bonus/penalty (can be negative, range -50 to 50)\n";
    prompt += "  \"accuracyModifier\": integer,   // Accuracy bonus/penalty (can be negative, range -50 to 50)\n";
    prompt += "  \"weightModifier\": integer,     // Weight change in grams (can be negative, range -2000 to 2000)\n";
    prompt += "  \"muzzleVelocityModifier\": integer, // Muzzle velocity modifier in m/s (can be negative, range -200 to 200)\n";
    prompt += "  \"effectiveRangeModifier\": integer, // Effective range modifier in meters (can be negative, range -200 to 200)\n";
    prompt += "  \"penetrationModifier\": integer,     // Penetration power modifier (can be negative, range -50 to 50)\n";
    prompt += "\n";
    prompt += "  \"hasBuiltInRail\": boolean,      // If true, this component has an integrated rail\n";
    prompt += "  \"railType\": string,            // \"Picatinny\", \"M-LOK\", \"KeyMod\", etc. (if hasBuiltInRail is true)\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "IMPORTANT:\n";
    prompt += "- category MUST be \"WeaponComponent\".\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that describes the component's appearance and function.\n";
    prompt += "- compatibleSlots must match the componentType (e.g., Muzzle components attach to Muzzle slots).\n";
    prompt += "- Some components can have subSlots:\n";
    prompt += "  * Handguard can have Rail subSlots (with or without built-in rail).\n";
    prompt += "  * Barrel can have Muzzle subSlot.\n";
    prompt += "  * Rail components can have Grip, Flashlight, Laser subSlots.\n";
    prompt += "  * Handguard with built-in rail (hasBuiltInRail=true) can directly have Grip/Flashlight/Laser subSlots.\n";
    prompt += "- Stat modifiers should be realistic (e.g., suppressors reduce recoil but add weight).\n";
    prompt += "- Magazines can show actual loaded order:\n";
    prompt += "  * orderIndex 0 = first rounds to fire (top of magazine).\n";
    prompt += "  * Sum of all roundCount values must be <= magazineCapacity.\n";
    prompt += "  * Mixing ammo types is allowed by default; include as many segments as needed.\n";
    prompt += "  * ammoId MUST reference an Ammo item ID (e.g., \"Ammo_556_AP\").\n";
    prompt += "  * Use ammoNotes like \"AP\", \"FMJ\", \"Tracer\" to describe the segment.\n";
    prompt += "\n";
    prompt += "Component Examples (Tarkov-style):\n";
    prompt += "- Magazine: Standard 30-round, Extended 60-round, Drum 100-round (attach to Magazine slot, compatibleSlots: [\"Magazine\"], includes magazineCapacity, caliber, magazineType, plus detailed loadedRounds for AP/FMJ mixes).\n";
    prompt += "- Muzzle: Suppressor, Compensator, Flash Hider, Muzzle Brake (attach to Muzzle slot, reduce recoil but add weight, may reduce muzzle velocity).\n";
    prompt += "- Grip: Vertical Grip, Angled Grip, Bipod (attach to Rail or Handguard with rail, improve ergonomics and recoil).\n";
    prompt += "- Sight: Red Dot, Holographic, Iron Sights (attach to Rail, improve accuracy, minimal weight).\n";
    prompt += "- Scope: 4x Scope, 8x Scope, Variable Zoom (attach to Rail, improve accuracy and effective range, may reduce ergonomics).\n";
    prompt += "- Stock: Fixed Stock, Folding Stock, Collapsible Stock (attach to Stock slot, improve recoil/ergonomics, weight varies).\n";
    prompt += "- Barrel: Long Barrel, Short Barrel, Heavy Barrel (attach to Barrel slot, may have Muzzle subSlot, affects muzzle velocity and effective range).\n";
    prompt += "- Handguard: Standard Handguard, Extended Handguard, KeyMod Handguard (attach to Handguard slot, may have Rail subSlots or built-in rail, affects ergonomics).\n";
    prompt += "- Rail: Picatinny Rail, M-LOK Rail, KeyMod Rail (attach to Rail slot or Handguard subSlot, can have Grip/Flashlight/Laser subSlots).\n";
    prompt += "- GasBlock: Standard Gas Block, Low Profile Gas Block (attach to GasBlock slot, affects reliability).\n";
    prompt += "- ChargingHandle: Standard, Extended, Ambidextrous (attach to ChargingHandle slot, affects ergonomics).\n";
    prompt += "- PistolGrip: Standard Grip, Ergonomic Grip, Rubberized Grip (attach to PistolGrip slot, affects ergonomics).\n";
    prompt += "- Foregrip: Vertical Foregrip, Angled Foregrip (attach to Rail or Handguard, improves recoil and ergonomics).\n";
    prompt += "- Flashlight: Tactical Flashlight, Weapon Light (attach to Rail, no stat modifiers, tactical use).\n";
    prompt += "- Laser: Red Laser, IR Laser, Laser/Flashlight Combo (attach to Rail, improve accuracy, may have flashlight built-in).\n";
    prompt += "- Mount: Scope Mount, Sight Mount (attach to Rail, allows mounting scopes/sights, may have subSlots).\n";
    prompt += "- Adapter: Thread Adapter, Rail Adapter (attach to various slots, allows compatibility between different systems).\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Common components: small stat bonuses (0-5), basic design.\n";
    prompt += "- Uncommon components: medium stat bonuses (5-15), better design.\n";
    prompt += "- Rare components: large stat bonuses (10-25), advanced design, may have multiple subSlots.\n";
    prompt += "- Modifiers can be negative (e.g., heavy components reduce ergonomics).\n";
    prompt += "\n";
    prompt += "Realism rules:\n";
    prompt += "- Reference real manufacturers/models whenever possible (e.g., EOTech EXPS3 sight, Aimpoint PRO, Trijicon ACOG, Vortex Razor, Magpul MOE SL stock, BCM Gunfighter charging handle, SureFire Scout light, Dead Air Sandman suppressor, Lancer L5 magazine).\n";
    prompt += "- Keep effects grounded in physics:\n";
    prompt += "  * Scopes/Sights improve accuracy/effective range, add weight, and NEVER change damage or recoil.\n";
    prompt += "  * Muzzle devices reduce recoil or flash; suppressors add noticeable weight and may slightly lower muzzle velocity.\n";
    prompt += "  * Grips/Foregrips/Pistol Grips improve ergonomics and recoil control but cannot increase damage.\n";
    prompt += "  * Stocks stabilize recoil and ergonomics; folding stocks can slightly improve ergonomics but rarely boost accuracy beyond 10.\n";
    prompt += "  * Barrels affect muzzle velocity, effective range, and weight in realistic directions (long/heavy barrels add weight and velocity, short barrels do the opposite).\n";
    prompt += "  * Handguards/Rails mainly provide mounting points, with small ergonomics/weight changes only.\n";
    prompt += "  * Flashlights/Lasers/Tactical devices add weight and provide minimal stat bonuses (laser can add small accuracy, flashlight typically adds none).\n";
    prompt += "  * Magazines only affect ergonomics/weight/reliability, never raw damage or penetration.\n";
    prompt += "  * Charging handles, mounts, adapters, and gas blocks offer subtle ergonomics or reliability tweaks, not huge stat swings.\n";
    prompt += "- Describe components like a loadout guide: include model name + manufacturer tone (\"Magpul MOE SL-K stock\"), material cues, and finish.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of components.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds)
{
    std::string prompt;
    prompt += GetPresetFlavorText(customPreset);
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " weapon attachment components ";
    prompt += "for this survival setting.\n";
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";
    prompt += "Use this JSON schema EXACTLY for each component:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"WeaponComponent\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer (usually 1 for components),\n";
    prompt += "\n";
    prompt += "  \"componentType\": \"Muzzle\" | \"Grip\" | \"Sight\" | \"Scope\" | \"Stock\" | \"Barrel\" | \"Handguard\" | \"Rail\" | \"Flashlight\" | \"Laser\" | \"Magazine\" | \"GasBlock\" | \"ChargingHandle\" | \"PistolGrip\" | \"Foregrip\" | \"Bipod\" | \"TacticalDevice\" | \"Mount\" | \"Adapter\",\n";
    prompt += "\n";
    prompt += "  // For Magazine components ONLY:\n";
    prompt += "  \"magazineCapacity\": integer,\n";
    prompt += "  \"caliber\": string,\n";
    prompt += "  \"magazineType\": string,\n";
    prompt += "  \"loadedRounds\": [\n";
    prompt += "    { \"orderIndex\": integer, \"roundCount\": integer, \"ammoId\": string, \"ammoDisplayName\": string, \"ammoNotes\": string }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"compatibleSlots\": [string],\n";
    prompt += "  \"subSlots\": [\n";
    prompt += "    { \"slotType\": string, \"slotIndex\": integer, \"hasBuiltInRail\": boolean }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"damageModifier\": integer,\n";
    prompt += "  \"recoilModifier\": integer,\n";
    prompt += "  \"ergonomicsModifier\": integer,\n";
    prompt += "  \"accuracyModifier\": integer,\n";
    prompt += "  \"weightModifier\": integer,\n";
    prompt += "  \"muzzleVelocityModifier\": integer,\n";
    prompt += "  \"effectiveRangeModifier\": integer,\n";
    prompt += "  \"penetrationModifier\": integer,\n";
    prompt += "\n";
    prompt += "  \"hasBuiltInRail\": boolean,\n";
    prompt += "  \"railType\": string,\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Common components: small stat bonuses (0-5), basic design.\n";
    prompt += "- Uncommon components: medium stat bonuses (5-15), better design.\n";
    prompt += "- Rare components: large stat bonuses (10-25), advanced design.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of components.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";
    return prompt;
}

std::string PromptBuilder::BuildAmmoJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds)
{
    std::string prompt;

    // 1) Add preset world context
    prompt += GetPresetFlavorText(preset);

    // 2) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " ammunition items ";
    prompt += "for this survival setting.\n\n";

    // 3) Add exclusion list if provided
    if (!excludeIds.empty())
    {
        prompt += "IMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }

    // 4) Define JSON schema
    prompt += "Use this JSON schema EXACTLY for each ammo:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Ammo\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer (usually 20-100 for ammo),\n";
    prompt += "\n";
    prompt += "  \"caliber\": string (e.g., \"9mm\", \"5.56mm\", \"7.62mm\", \"12gauge\", \".45ACP\"),\n";
    prompt += "\n";
    prompt += "  \"damageBonus\": integer,      // Damage bonus/penalty (can be negative, range -50 to 50)\n";
    prompt += "  \"penetration\": integer,      // 0-100, armor penetration (higher is better)\n";
    prompt += "  \"accuracyBonus\": integer,    // Accuracy bonus/penalty (can be negative, range -50 to 50)\n";
    prompt += "  \"recoilModifier\": integer,   // Recoil modifier (positive = less recoil, can be negative, range -50 to 50)\n";
    prompt += "\n";
    prompt += "  \"armorPiercing\": boolean,    // If true, has high penetration\n";
    prompt += "  \"hollowPoint\": boolean,      // If true, higher damage but lower penetration\n";
    prompt += "  \"tracer\": boolean,           // If true, visible trajectory\n";
    prompt += "  \"incendiary\": boolean,       // If true, can cause fire damage\n";
    prompt += "\n";
    prompt += "  \"value\": integer,            // 0-100, trade/scrap value\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "IMPORTANT:\n";
    prompt += "- category MUST be \"Ammo\".\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that describes the ammo's characteristics and use.\n";
    prompt += "- caliber must match weapon calibers (e.g., \"9mm\", \"5.56mm\", \"7.62mm\", \"12gauge\").\n";
    prompt += "- Special properties:\n";
    prompt += "  * Armor Piercing: high penetration (50-100), usually lower damage bonus.\n";
    prompt += "  * Hollow Point: high damage bonus (5-20), but low penetration (0-30).\n";
    prompt += "  * Tracer: visible trajectory, usually no stat modifiers.\n";
    prompt += "  * Incendiary: can cause fire damage, usually higher value.\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Common ammo: standard stats, no special properties, low value (5-20).\n";
    prompt += "- Uncommon ammo: better stats or one special property, medium value (15-40).\n";
    prompt += "- Rare ammo: excellent stats or multiple special properties, high value (30-80).\n";
    prompt += "- Standard ammo: damageBonus 0-5, penetration 20-40, no special properties.\n";
    prompt += "- Armor Piercing: penetration 50-100, damageBonus -10 to 5.\n";
    prompt += "- Hollow Point: damageBonus 5-20, penetration 0-30.\n";
    prompt += "- All stat values must be within their specified ranges.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of ammo items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";

    return prompt;
}

std::string PromptBuilder::BuildAmmoJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds)
{
    std::string prompt;
    prompt += GetPresetFlavorText(customPreset);
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " ammunition items ";
    prompt += "for this survival setting.\n";
    if (!excludeIds.empty())
    {
        prompt += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        for (const auto& id : excludeIds)
        {
            if (count > 0) prompt += ", ";
            prompt += id;
            count++;
            if (count >= 20)
            {
                prompt += " ... (and " + std::to_string(excludeIds.size() - 20) + " more)";
                break;
            }
        }
        prompt += "\nGenerate NEW unique IDs that are different from the above list.\n";
    }
    prompt += "\n";
    prompt += "Use this JSON schema EXACTLY for each ammo item:\n";
    prompt += "{\n";
    prompt += "  \"id\": string (unique, lowercase, underscore),\n";
    prompt += "  \"displayName\": string,\n";
    prompt += "  \"category\": \"Ammo\",\n";
    prompt += "  \"rarity\": \"Common\" | \"Uncommon\" | \"Rare\",\n";
    prompt += "  \"maxStack\": integer,\n";
    prompt += "\n";
    prompt += "  \"caliber\": string,\n";
    prompt += "  \"damageBonus\": integer,\n";
    prompt += "  \"penetration\": integer (0-100),\n";
    prompt += "  \"accuracyBonus\": integer,\n";
    prompt += "  \"recoilModifier\": integer,\n";
    prompt += "\n";
    prompt += "  \"armorPiercing\": boolean,\n";
    prompt += "  \"hollowPoint\": boolean,\n";
    prompt += "  \"tracer\": boolean,\n";
    prompt += "  \"incendiary\": boolean,\n";
    prompt += "\n";
    prompt += "  \"value\": integer,\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "Balancing rules:\n";
    prompt += "- Common: damageBonus 0-5, penetration 10-30, basic ammo.\n";
    prompt += "- Uncommon: damageBonus 3-10, penetration 25-50, special properties.\n";
    prompt += "- Rare: damageBonus 8-15, penetration 40-70, advanced properties.\n";
    prompt += "\n";
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of ammo items.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";
    return prompt;
}


