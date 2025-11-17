// ===============================
// Project Name: RundeeItemFactory
// File Name: StringUtils.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: String utility functions.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>

namespace StringUtils
{
    // Trim whitespace from both ends of a string
    void TrimString(std::string& str);

    // Clean JSON array text from LLM responses
    // - Removes trailing commas
    // - Closes unclosed brackets
    std::string CleanJsonArrayText(const std::string& input);

    // Escape special characters for command line
    std::string EscapeForCmd(const std::string& s);

    // Escape string for JSON
    std::string EscapeString(const std::string& s);

    // Normalize whitespace (replace newlines/tabs with spaces)
    std::string NormalizeWhitespace(const std::string& s);
}


