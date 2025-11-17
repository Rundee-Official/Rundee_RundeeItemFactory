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
#include "Generators/ItemFoodGenerator.h"

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

    // Build food item generation prompt
    static std::string BuildFoodJsonPrompt(const FoodGenerateParams& params, PresetType preset);

    // Build drink item generation prompt
    static std::string BuildDrinkJsonPrompt(const FoodGenerateParams& params, PresetType preset);

    // Build material item generation prompt
    static std::string BuildMaterialJsonPrompt(const FoodGenerateParams& params, PresetType preset);
};


