// ===============================
// Project Name: RundeeItemFactory
// File Name: CustomPreset.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-17
// Description: Implementation of custom preset management.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Prompts/CustomPreset.h"
#include "Utils/JsonUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace CustomPresetManager
{
    bool LoadPresetFromFile(const std::string& filePath, CustomPreset& preset)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "[CustomPresetManager] Failed to open preset file: " << filePath << "\n";
            return false;
        }

        std::string jsonText((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        if (jsonText.empty())
        {
            std::cerr << "[CustomPresetManager] Preset file is empty: " << filePath << "\n";
            return false;
        }

        try
        {
            nlohmann::json json = nlohmann::json::parse(jsonText);

            preset.id = JsonUtils::GetStringSafe(json, "id", "");
            preset.displayName = JsonUtils::GetStringSafe(json, "displayName", "");
            preset.description = JsonUtils::GetStringSafe(json, "description", "");
            preset.flavorText = JsonUtils::GetStringSafe(json, "flavorText", "");
            preset.author = JsonUtils::GetStringSafe(json, "author", "");
            preset.version = JsonUtils::GetStringSafe(json, "version", "");

            // Load tags array
            preset.tags.clear();
            if (json.contains("tags") && json["tags"].is_array())
            {
                for (const auto& tag : json["tags"])
                {
                    if (tag.is_string())
                    {
                        preset.tags.push_back(tag.get<std::string>());
                    }
                }
            }

            // Validate
            std::string errorMsg;
            if (!ValidatePreset(preset, errorMsg))
            {
                std::cerr << "[CustomPresetManager] Invalid preset: " << errorMsg << "\n";
                return false;
            }

            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[CustomPresetManager] Failed to parse preset JSON: " << e.what() << "\n";
            return false;
        }
    }

    bool SavePresetToFile(const CustomPreset& preset, const std::string& filePath)
    {
        std::string errorMsg;
        if (!ValidatePreset(preset, errorMsg))
        {
            std::cerr << "[CustomPresetManager] Cannot save invalid preset: " << errorMsg << "\n";
            return false;
        }

        try
        {
            nlohmann::json json;
            json["id"] = preset.id;
            json["displayName"] = preset.displayName;
            json["description"] = preset.description;
            json["flavorText"] = preset.flavorText;
            
            if (!preset.author.empty())
            {
                json["author"] = preset.author;
            }
            if (!preset.version.empty())
            {
                json["version"] = preset.version;
            }
            if (!preset.tags.empty())
            {
                json["tags"] = preset.tags;
            }

            std::ofstream file(filePath);
            if (!file.is_open())
            {
                std::cerr << "[CustomPresetManager] Failed to create preset file: " << filePath << "\n";
                return false;
            }

            file << json.dump(2); // Pretty print with 2-space indent
            file.close();

            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[CustomPresetManager] Failed to save preset: " << e.what() << "\n";
            return false;
        }
    }

    std::string GetPresetFlavorText(const CustomPreset& preset)
    {
        if (preset.flavorText.empty())
        {
            return "World context:\n- Generic survival environment.\n\n";
        }

        // Ensure it ends with newlines
        std::string text = preset.flavorText;
        if (text.back() != '\n')
        {
            text += "\n";
        }
        if (text.find("\n\n") == std::string::npos)
        {
            text += "\n";
        }

        return text;
    }

    bool ValidatePreset(const CustomPreset& preset, std::string& errorMessage)
    {
        if (preset.id.empty())
        {
            errorMessage = "Preset ID is required";
            return false;
        }

        // Validate ID format (lowercase, underscore, alphanumeric)
        for (char c : preset.id)
        {
            if (!std::isalnum(c) && c != '_')
            {
                errorMessage = "Preset ID must contain only lowercase letters, numbers, and underscores";
                return false;
            }
            if (std::isupper(c))
            {
                errorMessage = "Preset ID must be lowercase";
                return false;
            }
        }

        if (preset.displayName.empty())
        {
            errorMessage = "Display name is required";
            return false;
        }

        if (preset.flavorText.empty())
        {
            errorMessage = "Flavor text is required";
            return false;
        }

        if (preset.flavorText.length() < 20)
        {
            errorMessage = "Flavor text is too short (minimum 20 characters)";
            return false;
        }

        return true;
    }

    bool CreatePresetFromName(const std::string& presetName, CustomPreset& preset)
    {
        preset = CustomPreset();
        
        if (presetName == "default" || presetName.empty())
        {
            preset.id = "default";
            preset.displayName = "Default World";
            preset.description = "Generic survival environment";
            preset.flavorText = "World context:\n- Generic survival environment.\n- Post-apocalyptic setting.\n- Scarcity of resources.\n- Focus on survival mechanics.\n\n";
            preset.author = "System";
            preset.version = "1.0";
            return true;
        }
        
        // For other preset names, return false (not implemented)
        return false;
    }
}
