/**
 * @file PromptTemplateLoader.h
 * @brief Loads and processes prompt templates from external files
 * @author Haneul Lee (Rundee)
 * @date 2025-11-20
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides functionality to load prompt templates from external files
 * and replace template variables with actual values. Supports dynamic
 * prompt generation from template files.
 */

#pragma once

#include <string>
#include <set>

/**
 * @class PromptTemplateLoader
 * @brief Static class for loading and processing prompt templates
 * 
 * Loads prompt templates from external files and performs variable substitution.
 * Templates are stored in the prompts/ directory relative to the executable.
 */
class PromptTemplateLoader
{
public:
    /**
     * @brief Load template from file and replace variables
     * 
     * Loads a template file and replaces template variables with provided values.
     * Template variables use syntax like {{VARIABLE_NAME}}.
     * 
     * @param templateName Name of template file (without path, e.g., "food_template.txt")
     * @param presetContext World context/flavor text from preset
     * @param maxHunger Maximum hunger value for constraints
     * @param maxThirst Maximum thirst value for constraints
     * @param count Number of items to generate
     * @param excludeIds Set of existing item IDs to avoid duplicates
     * @param presetName Name of the preset being used
     * @param itemTypeName Name of the item type (e.g., "Food", "Weapon")
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Processed template string with variables replaced, or empty string on error
     * 
     * @note Template files are located in prompts/ directory relative to executable
     * @note Returns empty string if file not found or error occurs
     */
    static std::string LoadTemplate(const std::string& templateName, 
                                     const std::string& presetContext,
                                     int maxHunger,
                                     int maxThirst,
                                     int count,
                                     const std::set<std::string>& excludeIds = std::set<std::string>(),
                                     const std::string& presetName = "",
                                     const std::string& itemTypeName = "",
                                     const std::string& modelName = "",
                                     const std::string& generationTimestamp = "",
                                     int existingCount = -1);

    /**
     * @brief Get the base directory for templates
     * 
     * Returns the directory path where template files are stored.
     * Typically: executable directory + "prompts/"
     * 
     * @return Template directory path
     */
    static std::string GetTemplateDirectory();

    /**
     * @brief Check if template file exists
     * 
     * Verifies whether a template file exists in the template directory.
     * 
     * @param templateName Name of template file to check
     * @return true if template exists, false otherwise
     */
    static bool TemplateExists(const std::string& templateName);
};
