/**
 * @file DynamicPromptBuilder.h
 * @brief Dynamic prompt builder for profile-based item generation
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Builds LLM prompts dynamically from item profiles, including all field
 * definitions, validation rules, and relationship constraints.
 */

#pragma once

#include "Data/ItemProfile.h"
#include "Helpers/ItemGenerateParams.h"
#include "Prompts/CustomPreset.h"
#include <string>
#include <set>

/**
 * @class DynamicPromptBuilder
 * @brief Static class for building prompts from profiles
 */
class DynamicPromptBuilder
{
public:
    /**
     * @brief Build prompt from profile with all field information
     * @param profile Item profile with fields and validation
     * @param params Generation parameters (count, player stats)
     * @param customPreset Custom preset for world context
     * @param existingIds Set of existing IDs to avoid
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp string
     * @param existingCount Number of existing items
     * @return Complete prompt string with all profile data
     */
    static std::string BuildPromptFromProfile(
        const ItemProfile& profile,
        const FoodGenerateParams& params,
        const CustomPreset& customPreset,
        const std::set<std::string>& existingIds,
        const std::string& modelName,
        const std::string& generationTimestamp,
        int existingCount);
};
