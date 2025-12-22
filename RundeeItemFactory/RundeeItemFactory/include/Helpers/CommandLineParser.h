/**
 * @file CommandLineParser.h
 * @brief Command line argument parsing helpers
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides structures and functions for parsing command line arguments
 * and converting between string and enum representations.
 */

#pragma once

#include <string>
#include <vector>
#include "Helpers/ItemGenerateParams.h"

/**
 * @enum RunMode
 * @brief Execution mode for the item generator
 */
enum class RunMode
{
    LLM    ///< Single item type generation mode
};

/**
 * @enum ItemType
 * @brief Supported item types for generation
 */
enum class ItemType
{
    Food,            ///< Food items (restore hunger)
    Drink,           ///< Drink items (restore thirst)
    Medicine,        ///< Medicine items (restore health)
    Material,        ///< Material items (crafting resources)
    Weapon,          ///< Weapon items (combat equipment)
    WeaponComponent, ///< Weapon component items (attachments)
    Ammo,            ///< Ammunition items (weapon ammo)
    Armor,           ///< Armor items (protection equipment)
    Clothing         ///< Clothing items (warmth and style)
};

/**
 * @struct CommandLineArgs
 * @brief Parsed command line arguments structure
 * 
 * Contains all configuration needed for item generation, including
 * model selection, preset type, item type, generation parameters, and output paths.
 */
struct CommandLineArgs
{
    std::string modelName = "llama3";        ///< LLM model name (e.g., "llama3", "mistral")
    RunMode mode = RunMode::LLM;             ///< Execution mode (LLM)
    std::string presetName = "";             ///< User-defined preset name (empty = use customPresetPath or profile customContext)
    ItemType itemType = ItemType::Food;      ///< Item type to generate
    FoodGenerateParams params;               ///< Item-specific generation parameters
    std::string reportPath;                  ///< Path to JSON file for balance report (empty if not reporting)
    
    std::string customPresetPath;            ///< Path to custom preset JSON file (empty if using built-in preset)
    std::string additionalPrompt;           ///< Additional user-defined prompt text to append
    bool useTestMode = false;                ///< If true, outputs go to Test/ folder instead of ItemJson/
    std::string profileId;                   ///< Item profile ID to use for generation (empty = use default profile for item type)
};

/**
 * @namespace CommandLineParser
 * @brief Namespace for command line parsing utilities
 */
namespace CommandLineParser
{
    /**
     * @brief Parse command line arguments
     * 
     * Parses argc/argv into a CommandLineArgs structure. Supports:
     * - Model selection: --model <name>
     * - Item type: --type <food|drink|material|weapon|weaponcomponent|ammo>
     * - Count: --count <number>
     * - Preset: --preset <preset_name> (user-defined preset name)
     * - Output path: --output <path>
     * - Custom preset: --custom-preset <path> (path to custom preset JSON file)
     * - Additional prompt: --additional-prompt <text>
     * 
     * @param argc Number of command line arguments
     * @param argv Array of command line argument strings
     * @return Parsed CommandLineArgs structure
     */
    CommandLineArgs ParseArguments(int argc, char** argv);

    /**
     * @brief Convert ItemType enum to string
     * @param itemType Item type enum value
     * @return String representation (e.g., "Food", "Weapon")
     */
    std::string GetItemTypeName(ItemType itemType);

    /**
     * @brief Convert RunMode enum to string
     * @param mode Run mode enum value
     * @return String representation ("LLM")
     */
    std::string GetRunModeName(RunMode mode);
    
    /**
     * @brief Convert string to ItemType enum
     * 
     * Parses a string (case-insensitive) into an ItemType enum value.
     * 
     * @param typeStr String representation of item type
     * @return ItemType enum value, or ItemType::Food if parsing fails
     * 
     * @note Valid strings: "food", "drink", "material", "weapon", "weaponcomponent", "ammo"
     */
    ItemType ParseItemType(const std::string& typeStr);
}


