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
     */
    std::string version;
    
    /**
     * @brief Optional tags for categorization
     */
    std::vector<std::string> tags;
};

/**
 * @namespace CustomPresetManager
 * @brief Manager for custom preset operations
 */
namespace CustomPresetManager
{
    /**
     * @brief Load preset from JSON file
     * @param filePath Path to preset JSON file
     * @param preset Output preset
     * @return True if loaded successfully
     */
    bool LoadPresetFromFile(const std::string& filePath, CustomPreset& preset);
    
    /**
     * @brief Save preset to JSON file
     * @param preset Preset to save
     * @param filePath Path to save to
     * @return True if saved successfully
     */
    bool SavePresetToFile(const CustomPreset& preset, const std::string& filePath);
    
    /**
     * @brief Create preset from built-in name
     * @param presetName Name of built-in preset (e.g., "default")
     * @param preset Output preset
     * @return True if created successfully
     */
    bool CreatePresetFromName(const std::string& presetName, CustomPreset& preset);
    
    /**
     * @brief Validate preset
     * @param preset Preset to validate
     * @param errorMessage Output error message
     * @return True if valid
     */
    bool ValidatePreset(const CustomPreset& preset, std::string& errorMessage);
    
    /**
     * @brief Get flavor text from preset
     * @param preset Preset
     * @return Flavor text string
     */
    std::string GetPresetFlavorText(const CustomPreset& preset);
}
