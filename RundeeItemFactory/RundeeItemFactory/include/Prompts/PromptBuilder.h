/**
 * @file PromptBuilder.h
 * @brief Builds prompts for LLM item generation
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides methods to build structured prompts for LLM item generation.
 * Supports both built-in presets and custom presets for different game worlds.
 */

#pragma once

#include <string>
#include <set>
#include "Helpers/ItemGenerateParams.h"

// Forward declaration
struct CustomPreset;

/**
 * @enum PresetType
 * @brief Built-in preset types for item generation
 * 
 * Each preset provides different world context and flavor text for LLM prompts.
 */
enum class PresetType
{
    Default,  ///< Default/generic survival game setting
    Forest,   ///< Forest/wilderness survival setting
    Desert,   ///< Desert/arid survival setting
    Coast,    ///< Coastal/beach survival setting
    City      ///< Urban/city survival setting
};

/**
 * @class PromptBuilder
 * @brief Static class for building LLM prompts
 * 
 * Constructs detailed prompts for LLM item generation, including context,
 * constraints, examples, and formatting instructions. Supports all item types
 * and both built-in and custom presets.
 */
class PromptBuilder
{
public:
    /**
     * @brief Get preset flavor text for built-in preset
     * 
     * Returns world context text for the specified preset type.
     * This text is included in prompts to guide LLM generation.
     * 
     * @param preset Built-in preset type
     * @return Flavor text describing the world context
     */
    static std::string GetPresetFlavorText(PresetType preset);

    /**
     * @brief Get preset flavor text for custom preset
     * 
     * Returns world context text from a custom preset.
     * 
     * @param customPreset Custom preset structure
     * @return Flavor text from the custom preset
     */
    static std::string GetPresetFlavorText(const CustomPreset& customPreset);

    /**
     * @brief Build food item generation prompt (built-in preset)
     * 
     * Constructs a complete prompt for generating food items using a built-in preset.
     * Includes context, constraints, examples, and JSON format instructions.
     * 
     * @param params Generation parameters (count, maxHunger, maxThirst, etc.)
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs to avoid duplicates
     * @param modelName LLM model name (for context)
     * @param generationTimestamp Timestamp of generation (for logging)
     * @param existingCount Number of existing items (for context)
     * @return Complete prompt string for LLM
     */
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build food item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build drink item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build drink item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build medicine item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildMedicineJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build medicine item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildMedicineJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build material item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build material item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build weapon item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildWeaponJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build weapon item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildWeaponJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build weapon component item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build weapon component item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build ammo item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildAmmoJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build ammo item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildAmmoJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build armor item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildArmorJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build armor item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildArmorJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build clothing item generation prompt (built-in preset)
     * 
     * @param params Generation parameters
     * @param preset Built-in preset type
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildClothingJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    /**
     * @brief Build clothing item generation prompt (custom preset)
     * 
     * @param params Generation parameters
     * @param customPreset Custom preset structure
     * @param excludeIds Set of existing item IDs
     * @param modelName LLM model name
     * @param generationTimestamp Timestamp of generation
     * @param existingCount Number of existing items
     * @return Complete prompt string for LLM
     */
    static std::string BuildClothingJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
};


