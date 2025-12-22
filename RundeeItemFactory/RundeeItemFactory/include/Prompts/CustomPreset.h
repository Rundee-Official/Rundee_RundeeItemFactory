/**
 * @file CustomPreset.h
 * @brief Custom preset data structure and management
 * @author Haneul Lee (Rundee)
 * @date 2025-11-17
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the structure for custom presets that allow users to create
 * their own world contexts for item generation. Supports loading and saving
 * presets from JSON files.
 */

#pragma once

#include <string>
#include <vector>

/**
 * @struct CustomPreset
 * @brief Data structure for custom world presets
 * 
 * Represents a user-defined preset with custom world context and flavor text.
 * Used instead of built-in presets to provide unique world settings for item generation.
 */
struct CustomPreset
{
    /**
     * @brief Unique identifier (lowercase, underscore-separated)
     * 
     * Examples: "post_apocalyptic_city", "fantasy_forest"
     */
    std::string id;

    /**
     * @brief Human-readable display name
     * 
     * Examples: "Post-Apocalyptic City", "Fantasy Forest"
     */
    std::string displayName;

    /**
     * @brief Description of the preset
     * 
     * Explains what this preset represents and when to use it.
     */
    std::string description;

    /**
     * @brief World context text for LLM prompts
     * 
     * This text is included in prompts to guide LLM generation with the
     * appropriate world context and flavor.
     */
    std::string flavorText;

    /**
     * @brief Optional author name
     * 
     * Creator of the preset (optional metadata).
     */
    std::string author;

    /**
     * @brief Optional version string
     * 
     * Version of the preset (optional metadata).
     */
    std::string version;

    /**
     * @brief Optional tags for categorization
     * 
     * Examples: "survival", "post-apocalyptic", "fantasy", "sci-fi"
     */
    std::vector<std::string> tags;

    /**
     * @brief Check if preset is valid
     * 
     * A preset is valid if it has a non-empty id, displayName, and flavorText.
     * 
     * @return true if preset is valid, false otherwise
     */
    bool isValid() const
    {
        return !id.empty() && !displayName.empty() && !flavorText.empty();
    }
};

/**
 * @namespace CustomPresetManager
 * @brief Namespace for custom preset management functions
 */
namespace CustomPresetManager
{
    /**
     * @brief Load preset from JSON file
     * 
     * Reads a CustomPreset structure from a JSON file.
     * 
     * @param filePath Path to JSON file containing preset data
     * @param[out] preset Preset structure to populate
     * @return true on success, false on error
     * 
     * @note JSON file must contain id, displayName, and flavorText fields
     */
    bool LoadPresetFromFile(const std::string& filePath, CustomPreset& preset);
    
    /**
     * @brief Save preset to JSON file
     * 
     * Writes a CustomPreset structure to a JSON file.
     * 
     * @param preset Preset structure to save
     * @param filePath Path to JSON file to create/overwrite
     * @return true on success, false on error
     */
    bool SavePresetToFile(const CustomPreset& preset, const std::string& filePath);
    
    /**
     * @brief Get preset flavor text
     * 
     * Returns the flavor text from a custom preset for use in prompts.
     * 
     * @param preset Custom preset structure
     * @return Flavor text string
     */
    std::string GetPresetFlavorText(const CustomPreset& preset);
    
    /**
     * @brief Validate preset data
     * 
     * Checks if a preset structure is valid and provides error messages.
     * 
     * @param preset Preset structure to validate
     * @param[out] errorMessage Error message if validation fails
     * @return true if preset is valid, false otherwise
     */
    bool ValidatePreset(const CustomPreset& preset, std::string& errorMessage);
    
    /**
     * @brief Create a CustomPreset from a preset name
     * 
     * Creates a CustomPreset structure from a user-provided preset name.
     * The preset will have a simple flavor text based on the name.
     * 
     * @param presetName User-provided preset name (e.g., "Forest", "Desert", "My Custom World")
     * @param[out] preset Preset structure to populate
     * @return true on success, false on error
     * 
     * @note If presetName is empty, creates a default preset
     * @note The preset ID is generated from the name (lowercase, underscore-separated)
     */
    bool CreatePresetFromName(const std::string& presetName, CustomPreset& preset);
}









