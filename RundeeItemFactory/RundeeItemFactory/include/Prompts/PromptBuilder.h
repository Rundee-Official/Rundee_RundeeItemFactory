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

#include <string>
#include <set>
#include "Helpers/ItemGenerateParams.h"

// Forward declaration
struct CustomPreset;

enum class PresetType
{
    Default,
    Forest,
    Desert,
    Coast,
    City
};

class PromptBuilder
{
public:
    // Get preset flavor text
    static std::string GetPresetFlavorText(PresetType preset);
    static std::string GetPresetFlavorText(const CustomPreset& customPreset);

    // Build food item generation prompt
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>());
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>());

    // Build drink item generation prompt
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>());
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>());

    // Build material item generation prompt
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>());
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>());

    // Build weapon item generation prompt
    static std::string BuildWeaponJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>());
    static std::string BuildWeaponJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>());

    // Build weapon component item generation prompt
    static std::string BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>());
    static std::string BuildWeaponComponentJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>());

    // Build ammo item generation prompt
    static std::string BuildAmmoJsonPrompt(const FoodGenerateParams& params, PresetType preset, const std::set<std::string>& excludeIds = std::set<std::string>());
    static std::string BuildAmmoJsonPrompt(const FoodGenerateParams& params, const CustomPreset& customPreset, const std::set<std::string>& excludeIds = std::set<std::string>());
};


