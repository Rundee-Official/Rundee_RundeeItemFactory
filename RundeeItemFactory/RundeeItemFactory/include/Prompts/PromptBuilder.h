// ===============================
// Project Name: RundeeItemFactory
// File Name: PromptBuilder.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Builds prompts for LLM item generation.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

// Standard Library Includes
#include <set>
#include <string>

// Project Includes
#include "Helpers/ItemGenerateParams.h"

// ============================================================================
// SECTION 1: Forward Declarations
// ============================================================================

struct CustomPreset;

// ============================================================================
// SECTION 2: Enums
// ============================================================================

enum class PresetType
{
    Default,
    Forest,
    Desert,
    Coast,
    City,
    Arctic
};

// ============================================================================
// SECTION 3: PromptBuilder Class
// ============================================================================

class PromptBuilder
{
public:
    // Get preset flavor text
    static std::string GetPresetFlavorText(PresetType preset);
    static std::string GetPresetFlavorText(const CustomPreset& customPreset);

    // Build food item generation prompt
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build drink item generation prompt
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build material item generation prompt
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build weapon item generation prompt
    static std::string BuildWeaponJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildWeaponJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build weapon component item generation prompt
    static std::string BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build ammo item generation prompt
    static std::string BuildAmmoJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildAmmoJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build armor item generation prompt
    static std::string BuildArmorJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildArmorJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);

    // Build clothing item generation prompt
    static std::string BuildClothingJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
    static std::string BuildClothingJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>(), const std::string& modelName = std::string(), const std::string& generationTimestamp = std::string(), int existingCount = -1);
};


