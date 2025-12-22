/**
 * @file DynamicItemJsonParser.h
 * @brief Dynamic JSON parser for profile-based item parsing
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Parses JSON items dynamically based on item profiles, allowing
 * custom item structures to be parsed without hardcoded types.
 */

#pragma once

#include "Data/ItemProfile.h"
#include <string>
#include <vector>
#include <json.hpp>

/**
 * @class DynamicItemJsonParser
 * @brief Static class for parsing items dynamically from JSON
 * 
 * Parses items from JSON text using item profiles to determine
 * the expected structure. Returns items as JSON objects rather
 * than typed structs.
 */
class DynamicItemJsonParser
{
public:
    /**
     * @brief Parse items from JSON text using a profile
     * @param jsonText JSON text to parse
     * @param profile Profile defining the expected structure
     * @param outItems Output vector of parsed items (as JSON objects)
     * @return True if parsing succeeded
     */
    static bool ParseItemsFromJsonText(
        const std::string& jsonText,
        const ItemProfile& profile,
        std::vector<nlohmann::json>& outItems);
    
    /**
     * @brief Validate an item against its profile
     * @param item JSON object representing an item
     * @param profile Profile to validate against
     * @param errors Output vector of error messages
     * @return True if item is valid
     */
    static bool ValidateItem(
        const nlohmann::json& item,
        const ItemProfile& profile,
        std::vector<std::string>& errors);
    
    /**
     * @brief Apply default values from profile to an item
     * @param item JSON object to modify
     * @param profile Profile with default values
     */
    static void ApplyDefaults(nlohmann::json& item, const ItemProfile& profile);
    
    /**
     * @brief Get field value from item JSON
     * @param item JSON object
     * @param fieldName Field name
     * @param profile Profile for type information
     * @return JSON value (can be null if not found)
     */
    static nlohmann::json GetFieldValue(
        const nlohmann::json& item,
        const std::string& fieldName,
        const ItemProfile& profile);

private:
    /**
     * @brief Validate a single field value
     * @param value JSON value
     * @param field Field definition
     * @param errors Output vector of error messages
     * @return True if field is valid
     */
    static bool ValidateFieldValue(
        const nlohmann::json& value,
        const ProfileField& field,
        std::vector<std::string>& errors);
    
    /**
     * @brief Clean JSON text (remove markdown, extract array, etc.)
     * @param jsonText Raw JSON text
     * @return Cleaned JSON text
     */
    static std::string CleanJsonText(const std::string& jsonText);
};

