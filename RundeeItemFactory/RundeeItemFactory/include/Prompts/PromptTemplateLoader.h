// ===============================
// Project Name: RundeeItemFactory
// File Name: PromptTemplateLoader.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-20
// Description: Loads and processes prompt templates from external files.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include <set>

class PromptTemplateLoader
{
public:
    // Load template from file and replace variables
    // Returns empty string if file not found or error
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

    // Get the base directory for templates (executable directory + "prompts/")
    static std::string GetTemplateDirectory();

    // Check if template file exists
    static bool TemplateExists(const std::string& templateName);
};

