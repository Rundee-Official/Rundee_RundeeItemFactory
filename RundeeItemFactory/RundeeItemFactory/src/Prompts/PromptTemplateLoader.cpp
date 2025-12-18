// ===============================
// Project Name: RundeeItemFactory
// File Name: PromptTemplateLoader.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-20
// Description: Implementation of prompt template loading.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <algorithm>
#include <fstream>
#include <sstream>

// Windows Platform Includes
#include <windows.h>

// Project Includes
#include "Prompts/PromptTemplateLoader.h"

// ============================================================================
// SECTION 1: PromptTemplateLoader Class Implementation
// ============================================================================

std::string PromptTemplateLoader::GetTemplateDirectory()
{
    // Get executable directory
    char exePath[MAX_PATH];
    DWORD pathLen = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (pathLen == 0 || pathLen >= MAX_PATH)
    {
        return "prompts/"; // Fallback to relative path
    }

    // Extract directory
    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos)
    {
        exeDir = exeDir.substr(0, lastSlash + 1);
    }

    return exeDir + "prompts/";
}

bool PromptTemplateLoader::TemplateExists(const std::string& templateName)
{
    std::string templatePath = GetTemplateDirectory() + templateName + ".txt";
    std::ifstream file(templatePath);
    return file.good();
}

std::string PromptTemplateLoader::LoadTemplate(const std::string& templateName,
                                                const std::string& presetContext,
                                                int maxHunger,
                                                int maxThirst,
                                                int count,
                                                const std::set<std::string>& excludeIds,
                                                const std::string& presetName,
                                                const std::string& itemTypeName,
                                                const std::string& modelName,
                                                const std::string& generationTimestamp,
                                                int existingCount)
{
    std::string templatePath = GetTemplateDirectory() + templateName + ".txt";
    
    // Try to read template file
    std::ifstream file(templatePath);
    if (!file.is_open())
    {
        // Template file not found - return empty string (will fall back to hardcoded prompts)
        return {};
    }

    // Read entire file
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string templateContent = buffer.str();
    file.close();

    // Replace variables
    // {PRESET_CONTEXT}
    size_t pos = templateContent.find("{PRESET_CONTEXT}");
    while (pos != std::string::npos)
    {
        templateContent.replace(pos, 16, presetContext);
        pos = templateContent.find("{PRESET_CONTEXT}", pos + presetContext.length());
    }

    // {PRESET_NAME}
    if (!presetName.empty())
    {
        pos = templateContent.find("{PRESET_NAME}");
        while (pos != std::string::npos)
        {
            templateContent.replace(pos, 13, presetName);
            pos = templateContent.find("{PRESET_NAME}", pos + presetName.length());
        }
    }

    // {ITEM_TYPE}
    if (!itemTypeName.empty())
    {
        pos = templateContent.find("{ITEM_TYPE}");
        while (pos != std::string::npos)
        {
            templateContent.replace(pos, 10, itemTypeName);
            pos = templateContent.find("{ITEM_TYPE}", pos + itemTypeName.length());
        }
    }

    // {MODEL_NAME}
    if (!modelName.empty())
    {
        pos = templateContent.find("{MODEL_NAME}");
        while (pos != std::string::npos)
        {
            templateContent.replace(pos, 12, modelName);
            pos = templateContent.find("{MODEL_NAME}", pos + modelName.length());
        }
    }

    // {TIMESTAMP}
    if (!generationTimestamp.empty())
    {
        pos = templateContent.find("{TIMESTAMP}");
        while (pos != std::string::npos)
        {
            templateContent.replace(pos, 11, generationTimestamp);
            pos = templateContent.find("{TIMESTAMP}", pos + generationTimestamp.length());
        }
    }

    // {MAX_HUNGER}
    pos = templateContent.find("{MAX_HUNGER}");
    while (pos != std::string::npos)
    {
        templateContent.replace(pos, 12, std::to_string(maxHunger));
        pos = templateContent.find("{MAX_HUNGER}", pos + std::to_string(maxHunger).length());
    }

    // {MAX_THIRST}
    pos = templateContent.find("{MAX_THIRST}");
    while (pos != std::string::npos)
    {
        templateContent.replace(pos, 12, std::to_string(maxThirst));
        pos = templateContent.find("{MAX_THIRST}", pos + std::to_string(maxThirst).length());
    }

    // {COUNT}
    pos = templateContent.find("{COUNT}");
    while (pos != std::string::npos)
    {
        templateContent.replace(pos, 7, std::to_string(count));
        pos = templateContent.find("{COUNT}", pos + std::to_string(count).length());
    }

    // {EXCLUDE_IDS}
    pos = templateContent.find("{EXCLUDE_IDS}");
    if (pos != std::string::npos)
    {
        std::string excludeIdsText;
        if (!excludeIds.empty())
        {
            excludeIdsText = "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
            int idCount = 0;
            for (const auto& id : excludeIds)
            {
                if (idCount > 0) excludeIdsText += ", ";
                excludeIdsText += id;
                idCount++;
                if (idCount >= 20) // Limit to first 20 to avoid prompt bloat
                {
                    excludeIdsText += " ... (and " + std::to_string(excludeIds.size() - 20) + " more, list truncated)";
                    break;
                }
            }
            excludeIdsText += "\nGenerate NEW unique IDs that are different from all existing IDs (assume many more exist). Avoid reusing stems; use fresh, novel names, not simple number suffixes.\n";
        }
        templateContent.replace(pos, 13, excludeIdsText);
    }

    // {EXISTING_COUNT}
    if (existingCount >= 0)
    {
        pos = templateContent.find("{EXISTING_COUNT}");
        while (pos != std::string::npos)
        {
            templateContent.replace(pos, 16, std::to_string(existingCount));
            pos = templateContent.find("{EXISTING_COUNT}", pos + std::to_string(existingCount).length());
        }
    }

    return templateContent;
}

