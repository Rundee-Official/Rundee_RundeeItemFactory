/**
 * @file JsonUtils.h
 * @brief JSON utility functions for safe parsing
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides safe JSON parsing utilities that return default values on error,
 * preventing crashes from malformed or missing JSON fields.
 */

#pragma once

#include <string>
#include "json.hpp"

using nlohmann::json;

/**
 * @namespace JsonUtils
 * @brief Namespace for JSON utility functions
 */
namespace JsonUtils
{
    /**
     * @brief Safely get integer value from JSON object
     * 
     * Attempts to retrieve an integer value from a JSON object by key.
     * Returns the default value if the key doesn't exist, the value is not a number,
     * or any other error occurs.
     * 
     * @param j JSON object to query
     * @param key Key to look up
     * @param defaultValue Value to return on error (default: 0)
     * @return Integer value from JSON, or defaultValue on error
     */
    int GetIntSafe(const json& j, const char* key, int defaultValue = 0);

    /**
     * @brief Safely get boolean value from JSON object
     * 
     * Attempts to retrieve a boolean value from a JSON object by key.
     * Returns the default value if the key doesn't exist, the value is not a boolean,
     * or any other error occurs.
     * 
     * @param j JSON object to query
     * @param key Key to look up
     * @param defaultValue Value to return on error (default: false)
     * @return Boolean value from JSON, or defaultValue on error
     */
    bool GetBoolSafe(const json& j, const char* key, bool defaultValue = false);

    /**
     * @brief Safely get string value from JSON object
     * 
     * Attempts to retrieve a string value from a JSON object by key.
     * Returns the default value if the key doesn't exist, the value is not a string,
     * or any other error occurs.
     * 
     * @param j JSON object to query
     * @param key Key to look up
     * @param defaultValue Value to return on error (default: empty string)
     * @return String value from JSON, or defaultValue on error
     */
    std::string GetStringSafe(const json& j, const char* key, const std::string& defaultValue = "");

    /**
     * @brief Clamp integer value between min and max
     * 
     * Ensures an integer value is within the specified range.
     * Values below min are set to min, values above max are set to max.
     * 
     * @param v Value to clamp
     * @param minV Minimum allowed value
     * @param maxV Maximum allowed value
     * @return Clamped value (guaranteed to be between minV and maxV)
     */
    int ClampInt(int v, int minV, int maxV);
}
