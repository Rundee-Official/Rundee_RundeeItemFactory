/**
 * @file DynamicPromptBuilder.h
 * @brief Builds LLM prompts dynamically from item profiles
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Generates LLM prompts dynamically based on item profiles, allowing
 * custom item structures to be used for generation.
 */

#pragma once

#include "Data/ItemProfile.h"
#include <string>
#include <set>

// Forward declarations
struct FoodGenerateParams;
struct CustomPreset;

/**
 * @class DynamicPromptBuilder
 * @brief Static class for building dynamic prompts from profiles
 * 
 * Constructs LLM prompts by converting item profiles into JSON schema
 * descriptions that the LLM can understand and generate items from.
 */
class DynamicPromptBuilder
{
public:
    /**
     * @brief Build a prompt from a profile with custom preset
     * @param profile Item profile to use
     * @param params Generation parameters
     * @param customPreset Custom preset
     * @param excludeIds Set of IDs to exclude
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp string
     * @param existingCount Number of existing items
     * @return Generated prompt string
     */
    static std::string BuildPromptFromProfile(
        const ItemProfile& profile,
        const FoodGenerateParams& params,
        const CustomPreset& customPreset,
        const std::set<std::string>& excludeIds,
        const std::string& modelName,
        const std::string& generationTimestamp,
        int existingCount);
    
    /**
     * @brief Convert profile to JSON schema string for LLM
     * @param profile Profile to convert
     * @return JSON schema description string
     */
    static std::string ProfileToJsonSchemaString(const ItemProfile& profile);
    
    /**
     * @brief Convert field to JSON schema description
     * @param field Field to convert
     * @return Field description string
     */
    static std::string FieldToSchemaDescription(const ProfileField& field);
    
    /**
     * @brief Build validation rules description
     * @param profile Profile with fields to describe
     * @return Validation rules text
     */
    static std::string BuildValidationRules(const ItemProfile& profile);

private:
    /**
     * @brief Get preset flavor text from custom preset
     * @param customPreset Custom preset
     * @return Flavor text
     */
    static std::string GetPresetFlavorText(const CustomPreset& customPreset);
    
    /**
     * @brief Build exclusion section
     * @param excludeIds Set of IDs to exclude
     * @return Exclusion text
     */
    static std::string BuildExcludeSection(const std::set<std::string>& excludeIds);
};

