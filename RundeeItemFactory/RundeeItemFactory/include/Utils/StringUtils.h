/**
 * @file StringUtils.h
 * @brief String utility functions
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides utility functions for string manipulation, JSON cleaning,
 * escaping, and control character removal.
 */

#pragma once

#include <string>

/**
 * @namespace StringUtils
 * @brief Namespace for string utility functions
 */
namespace StringUtils
{
    /**
     * @brief Trim whitespace from both ends of a string
     * 
     * Removes leading and trailing whitespace characters (spaces, tabs, newlines).
     * Modifies the string in-place.
     * 
     * @param[in,out] str String to trim (modified in-place)
     */
    void TrimString(std::string& str);

    /**
     * @brief Clean JSON array text from LLM responses
     * 
     * Removes trailing commas and closes unclosed brackets in JSON array text.
     * Handles common JSON formatting errors from LLM responses.
     * 
     * @param input Raw JSON text from LLM
     * @return Cleaned JSON text
     * 
     * @note Removes trailing commas before closing brackets
     * @note Attempts to close unclosed brackets
     */
    std::string CleanJsonArrayText(const std::string& input);

    /**
     * @brief Fix common JSON errors
     * 
     * Attempts to fix common JSON formatting errors like trailing commas,
     * unclosed brackets, and malformed structures.
     * 
     * @param input JSON text with potential errors
     * @return Fixed JSON text
     */
    std::string FixCommonJsonErrors(const std::string& input);

    /**
     * @brief Escape special characters for command line
     * 
     * Escapes special characters that have meaning in command line shells
     * (quotes, spaces, etc.) so the string can be safely used in command line arguments.
     * 
     * @param s String to escape
     * @return Escaped string safe for command line use
     */
    std::string EscapeForCmd(const std::string& s);

    /**
     * @brief Escape string for JSON
     * 
     * Escapes special characters for JSON format (quotes, backslashes, newlines, etc.).
     * 
     * @param s String to escape
     * @return JSON-escaped string
     */
    std::string EscapeString(const std::string& s);

    /**
     * @brief Normalize whitespace
     * 
     * Replaces newlines and tabs with spaces, and collapses multiple spaces into one.
     * 
     * @param s String to normalize
     * @return String with normalized whitespace
     */
    std::string NormalizeWhitespace(const std::string& s);

    /**
     * @brief Remove ANSI escape codes
     * 
     * Removes ANSI escape sequences (e.g., cursor hide/show codes, color codes)
     * from terminal output strings.
     * 
     * @param s String potentially containing ANSI codes
     * @return String with ANSI codes removed
     */
    std::string StripAnsiEscapeCodes(const std::string& s);

    /**
     * @brief Remove non-printable control characters
     * 
     * Removes control characters (except whitespace) from the string.
     * Preserves spaces, tabs, and newlines.
     * 
     * @param s String potentially containing control characters
     * @return String with control characters removed
     */
    std::string RemoveControlCharacters(const std::string& s);
}
