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

// Standard Library Includes
#include <string>

// ============================================================================
// SECTION 1: StringUtils Namespace
// ============================================================================

namespace StringUtils
{
    // ========================================================================
    // JSON Processing Functions
    // ========================================================================
    
    // Clean JSON array text from LLM responses
    // - Removes trailing commas
    // - Closes unclosed brackets
    // - Removes comments
    std::string CleanJsonArrayText(const std::string& input);
    
    // Fix common JSON errors (missing quotes, spaces in IDs, etc.)
    std::string FixCommonJsonErrors(const std::string& input);
    
    // ========================================================================
    // String Manipulation Functions
    // ========================================================================
    
    // Trim whitespace from both ends of a string
    void TrimString(std::string& str);
    
    // Normalize whitespace (replace newlines/tabs with spaces)
    std::string NormalizeWhitespace(const std::string& s);
    
    // ========================================================================
    // Escape Functions
    // ========================================================================
    
    // Escape special characters for command line
    std::string EscapeForCmd(const std::string& s);
    
    // Escape string for JSON
    std::string EscapeString(const std::string& s);
    
    // ========================================================================
    // Control Character Removal Functions
    // ========================================================================
    
    // Remove ANSI escape/control sequences (e.g., cursor hide/show codes)
    std::string StripAnsiEscapeCodes(const std::string& s);
    
    // Remove other non-printable control characters (except whitespace)
    std::string RemoveControlCharacters(const std::string& s);
}


