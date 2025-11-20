// ===============================
// Project Name: RundeeItemFactory
// File Name: CustomPreset.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-17
// Description: Custom preset data structure and management.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include <vector>

struct CustomPreset
{
    std::string id;              // Unique identifier (lowercase, underscore)
    std::string displayName;     // Human-readable name
    std::string description;      // Description of the preset
    std::string flavorText;      // World context text for LLM prompts
    
    // Optional metadata
    std::string author;
    std::string version;
    std::vector<std::string> tags;  // e.g., "survival", "post-apocalyptic", "fantasy"
    
    bool isValid() const
    {
        return !id.empty() && !displayName.empty() && !flavorText.empty();
    }
};

namespace CustomPresetManager
{
    // Load preset from JSON file
    bool LoadPresetFromFile(const std::string& filePath, CustomPreset& preset);
    
    // Save preset to JSON file
    bool SavePresetToFile(const CustomPreset& preset, const std::string& filePath);
    
    // Get preset flavor text (for use in prompts)
    std::string GetPresetFlavorText(const CustomPreset& preset);
    
    // Validate preset data
    bool ValidatePreset(const CustomPreset& preset, std::string& errorMessage);
}



