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
    prompt += "  \"weaponType\": \"AssaultRifle\" | \"SMG\" | \"Pistol\" | \"SniperRifle\" | \"Shotgun\" | \"LMG\" | \"DMR\",\n";
    prompt += "  \"caliber\": string (e.g., \"9mm\", \"5.56mm\", \"7.62mm\", \"12gauge\", \".45ACP\"),\n";
    prompt += "\n";
    prompt += "  \"minDamage\": integer,      // 0-100, minimum damage per shot\n";
    prompt += "  \"maxDamage\": integer,      // 0-100, maximum damage per shot\n";
    prompt += "  \"fireRate\": integer,       // 0-1200, rounds per minute\n";
    prompt += "  \"accuracy\": integer,       // 0-100, base accuracy (higher is better)\n";
    prompt += "  \"recoil\": integer,         // 0-100, recoil control (lower is better)\n";
    prompt += "  \"ergonomics\": integer,     // 0-100, handling (higher is better)\n";
    prompt += "  \"weight\": integer,         // Weight in grams\n";
    prompt += "  \"durability\": integer,     // 0-100, weapon condition\n";
    prompt += "\n";
    prompt += "  \"magazineCapacity\": integer,  // Default magazine capacity\n";
    prompt += "  \"magazineType\": string,        // \"Standard\", \"Extended\", \"Drum\", etc.\n";
    prompt += "\n";
    prompt += "  \"attachmentSlots\": [\n";
    prompt += "    {\n";
    prompt += "      \"slotType\": string,    // \"Muzzle\", \"Grip\", \"Sight\", \"Scope\", \"Stock\", \"Barrel\", \"Handguard\", \"Rail\", etc.\n";
    prompt += "      \"slotIndex\": integer,   // For multiple slots of same type (default 0)\n";
    prompt += "      \"isRequired\": boolean  // Whether this slot must be filled (default false)\n";
    prompt += "    }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"description\": string\n";
    prompt += "}\n";
    prompt += "\n";
    prompt += "IMPORTANT:\n";
    prompt += "- category MUST be \"Weapon\".\n";
    prompt += "- description must NOT be empty and should be 1 short sentence (5-20 words)\n";
    prompt += "  that describes the weapon's appearance and characteristics.\n";
    prompt += "- attachmentSlots should include common slots like Muzzle, Grip, Sight, Stock, etc.\n";
    prompt += "- Some weapons may have multiple Rail slots (use slotIndex to differentiate).\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Common weapons: lower damage (20-40), basic stats, simple design.\n";
    prompt += "- Uncommon weapons: medium damage (35-60), better stats, more attachment slots.\n";
    prompt += "- Rare weapons: high damage (50-80), excellent stats, many attachment slots.\n";
    prompt += "- Assault rifles: fireRate 600-900, damage 30-60, multiple attachment slots.\n";
    prompt += "- SMGs: fireRate 700-1000, damage 20-40, lighter weight.\n";
    prompt += "- Pistols: fireRate 400-600, damage 15-35, low weight, few slots.\n";
    prompt += "- Sniper rifles: fireRate 20-60, damage 70-100, high accuracy, heavy weight.\n";
    prompt += "- Shotguns: fireRate 60-120, damage 40-80, low range.\n";
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
    prompt += "  \"componentType\": \"Muzzle\" | \"Grip\" | \"Sight\" | \"Scope\" | \"Stock\" | \"Barrel\" | \"Handguard\" | \"Rail\" | \"Flashlight\" | \"Laser\",\n";
    prompt += "\n";
    prompt += "  \"compatibleSlots\": [string],  // Which weapon slots this can attach to, e.g., [\"Muzzle\", \"Barrel\"]\n";
    prompt += "\n";
    prompt += "  \"subSlots\": [\n";
    prompt += "    {\n";
    prompt += "      \"slotType\": string,        // \"Muzzle\", \"Grip\", \"Sight\", \"Rail\", etc.\n";
    prompt += "      \"slotIndex\": integer,       // For multiple slots of same type (default 0)\n";
    prompt += "      \"hasBuiltInRail\": boolean   // If true, this slot already has a rail (default false)\n";
    prompt += "    }\n";
    prompt += "  ],\n";
    prompt += "\n";
    prompt += "  \"damageModifier\": integer,      // Damage bonus/penalty (can be negative)\n";
    prompt += "  \"recoilModifier\": integer,     // Recoil reduction (positive = less recoil, can be negative)\n";
    prompt += "  \"ergonomicsModifier\": integer, // Ergonomics bonus/penalty (can be negative)\n";
    prompt += "  \"accuracyModifier\": integer,   // Accuracy bonus/penalty (can be negative)\n";
    prompt += "  \"weightModifier\": integer,     // Weight change in grams (can be negative)\n";
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
    prompt += "\n";
    prompt += "Component Examples:\n";
    prompt += "- Muzzle: Suppressor, Compensator, Flash Hider (attach to Muzzle slot, may reduce recoil, add weight).\n";
    prompt += "- Grip: Vertical Grip, Angled Grip (attach to Rail or Handguard with rail, improve ergonomics).\n";
    prompt += "- Sight: Red Dot, Holographic (attach to Rail, improve accuracy).\n";
    prompt += "- Scope: 4x Scope, 8x Scope (attach to Rail, improve accuracy, may reduce ergonomics).\n";
    prompt += "- Stock: Fixed Stock, Folding Stock (attach to Stock slot, improve recoil/ergonomics).\n";
    prompt += "- Barrel: Long Barrel, Short Barrel (attach to Barrel slot, may have Muzzle subSlot).\n";
    prompt += "- Handguard: Standard Handguard, Extended Handguard (attach to Handguard slot, may have Rail subSlots or built-in rail).\n";
    prompt += "- Rail: Picatinny Rail, M-LOK Rail (attach to Rail slot or Handguard subSlot, can have Grip/Flashlight/Laser subSlots).\n";
    prompt += "- Flashlight: Tactical Flashlight (attach to Rail, no stat modifiers).\n";
    prompt += "- Laser: Red Laser, IR Laser (attach to Rail, improve accuracy).\n";
    prompt += "\n";
    prompt += "Balancing:\n";
    prompt += "- Common components: small stat bonuses (0-5), basic design.\n";
    prompt += "- Uncommon components: medium stat bonuses (5-15), better design.\n";
    prompt += "- Rare components: large stat bonuses (10-25), advanced design, may have multiple subSlots.\n";
    prompt += "- Modifiers can be negative (e.g., heavy components reduce ergonomics).\n";
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


