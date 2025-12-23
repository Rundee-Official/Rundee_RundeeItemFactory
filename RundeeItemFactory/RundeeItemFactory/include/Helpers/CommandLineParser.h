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
 * @brief Command line parsing functions
 */
namespace CommandLineParser
{
    /**
     * @brief Parse command line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return Parsed arguments
     */
    CommandLineArgs ParseArguments(int argc, char** argv);
    
    /**
     * @brief Get run mode name as string
     * @param mode Run mode
     * @return Mode name string
     */
    std::string GetRunModeName(RunMode mode);
    
    /**
     * @brief Get item type name as string
     * @param type Item type
     * @return Type name string
     */
    std::string GetItemTypeName(ItemType type);
}
