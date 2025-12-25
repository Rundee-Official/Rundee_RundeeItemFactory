/**
 * @file DynamicItemJsonParser.cpp
 * @brief Implementation of dynamic JSON parser
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Parsers/DynamicItemJsonParser.h"
#include "Utils/StringUtils.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <map>
#include <vector>

using nlohmann::json;

bool DynamicItemJsonParser::ParseItemsFromJsonText(
    const std::string& jsonText,
    const ItemProfile& profile,
    std::vector<nlohmann::json>& outItems)
{
    outItems.clear();
    
    // Check for empty input
    if (jsonText.empty() || jsonText.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        std::cerr << "[DynamicItemJsonParser] Error: Input JSON text is empty or contains only whitespace.\n";
        return false;
    }
    
    // Clean LLM response string
    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);
    
    if (cleaned.empty() || cleaned.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        std::cerr << "[DynamicItemJsonParser] Error: JSON text became empty after cleaning.\n";
        return false;
    }
    
    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const json::parse_error& e)
    {
        std::cerr << "[DynamicItemJsonParser] JSON parse error (position " << e.byte << "): "
            << e.what() << "\n";
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[DynamicItemJsonParser] JSON parse error: " << e.what() << "\n";
        return false;
    }
    
    if (root.is_null() || !root.is_array())
    {
        std::cerr << "[DynamicItemJsonParser] Error: Root JSON is not an array.\n";
        return false;
    }
    
    if (root.empty())
    {
        std::cerr << "[DynamicItemJsonParser] Warning: JSON array is empty.\n";
        return false;
    }
    
    // Parse each item
    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[DynamicItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }
        
        // Apply defaults from profile
        json item = jItem;
        ApplyDefaults(item, profile);
        
        // Ensure id and displayName are always present (generate if missing)
        EnsureIdAndDisplayName(item, profile, i);
        
        // Validate item
        std::vector<std::string> errors;
        if (!ValidateItem(item, profile, errors))
        {
            std::cerr << "[DynamicItemJsonParser] Item at index " << i << " validation failed:\n";
            for (const auto& error : errors)
            {
                std::cerr << "  - " << error << "\n";
            }
            continue;
        }
        
        outItems.push_back(item);
    }
    
    std::cout << "[DynamicItemJsonParser] Parsed " << outItems.size()
        << " items from JSON using profile \"" << profile.id << "\".\n";
    
    return !outItems.empty();
}

bool DynamicItemJsonParser::ValidateItem(
    const nlohmann::json& item,
    const ItemProfile& profile,
    std::vector<std::string>& errors)
{
    errors.clear();
    
    if (!item.is_object())
    {
        errors.push_back("Item is not a JSON object");
        return false;
    }
    
    // Check required fields
    for (const auto& field : profile.fields)
    {
        if (field.validation.isRequired)
        {
            if (!item.contains(field.name) || item[field.name].is_null())
            {
                errors.push_back("Required field '" + field.name + "' is missing");
                continue;
            }
        }
        
        // Validate field value if present
        if (item.contains(field.name))
        {
            if (!ValidateFieldValue(item[field.name], field, errors))
            {
                // Error messages are added in ValidateFieldValue
            }
        }
    }
    
    // Check for unknown fields (optional - can be disabled if desired)
    // for (const auto& [key, value] : item.items())
    // {
    //     if (!profile.HasField(key))
    //     {
    //         errors.push_back("Unknown field: " + key);
    //     }
    // }
    
    return errors.empty();
}

void DynamicItemJsonParser::ApplyDefaults(nlohmann::json& item, const ItemProfile& profile)
{
    for (const auto& field : profile.fields)
    {
        if (!item.contains(field.name) || item[field.name].is_null())
        {
            if (!field.defaultValue.is_null())
            {
                item[field.name] = field.defaultValue;
            }
        }
    }
}

nlohmann::json DynamicItemJsonParser::GetFieldValue(
    const nlohmann::json& item,
    const std::string& fieldName,
    const ItemProfile& profile)
{
    if (!item.is_object() || !item.contains(fieldName))
    {
        return nlohmann::json();
    }
    
    return item[fieldName];
}

bool DynamicItemJsonParser::ValidateFieldValue(
    const nlohmann::json& value,
    const ProfileField& field,
    std::vector<std::string>& errors)
{
    // Type checking
    switch (field.type)
    {
        case ProfileFieldType::String:
            if (!value.is_string())
            {
                errors.push_back("Field '" + field.name + "' must be a string");
                return false;
            }
            // Length validation
            if (field.validation.minLength > 0 && value.get<std::string>().length() < static_cast<size_t>(field.validation.minLength))
            {
                errors.push_back("Field '" + field.name + "' must be at least " + std::to_string(field.validation.minLength) + " characters");
                return false;
            }
            if (field.validation.maxLength > 0 && value.get<std::string>().length() > static_cast<size_t>(field.validation.maxLength))
            {
                errors.push_back("Field '" + field.name + "' must be at most " + std::to_string(field.validation.maxLength) + " characters");
                return false;
            }
            // Allowed values validation
            if (!field.validation.allowedValues.empty())
            {
                std::string strValue = value.get<std::string>();
                bool found = false;
                for (const auto& allowed : field.validation.allowedValues)
                {
                    if (strValue == allowed)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    errors.push_back("Field '" + field.name + "' has invalid value. Allowed: " + 
                        [&field]() {
                            std::string result;
                            for (size_t i = 0; i < field.validation.allowedValues.size(); ++i)
                            {
                                if (i > 0) result += ", ";
                                result += field.validation.allowedValues[i];
                            }
                            return result;
                        }());
                    return false;
                }
            }
            break;
            
        case ProfileFieldType::Integer:
            if (!value.is_number_integer())
            {
                errors.push_back("Field '" + field.name + "' must be an integer");
                return false;
            }
            {
                int intValue = value.get<int>();
                if (field.validation.minValue != 0.0 && intValue < static_cast<int>(field.validation.minValue))
                {
                    errors.push_back("Field '" + field.name + "' must be >= " + std::to_string(static_cast<int>(field.validation.minValue)));
                    return false;
                }
                if (field.validation.maxValue != 0.0 && intValue > static_cast<int>(field.validation.maxValue))
                {
                    errors.push_back("Field '" + field.name + "' must be <= " + std::to_string(static_cast<int>(field.validation.maxValue)));
                    return false;
                }
            }
            break;
            
        case ProfileFieldType::Float:
            if (!value.is_number())
            {
                errors.push_back("Field '" + field.name + "' must be a number");
                return false;
            }
            {
                double floatValue = value.get<double>();
                if (field.validation.minValue != 0.0 && floatValue < field.validation.minValue)
                {
                    errors.push_back("Field '" + field.name + "' must be >= " + std::to_string(field.validation.minValue));
                    return false;
                }
                if (field.validation.maxValue != 0.0 && floatValue > field.validation.maxValue)
                {
                    errors.push_back("Field '" + field.name + "' must be <= " + std::to_string(field.validation.maxValue));
                    return false;
                }
            }
            break;
            
        case ProfileFieldType::Boolean:
            if (!value.is_boolean())
            {
                errors.push_back("Field '" + field.name + "' must be a boolean");
                return false;
            }
            break;
            
        case ProfileFieldType::Array:
            if (!value.is_array())
            {
                errors.push_back("Field '" + field.name + "' must be an array");
                return false;
            }
            // Array length validation
            if (field.validation.minLength > 0 && value.size() < static_cast<size_t>(field.validation.minLength))
            {
                errors.push_back("Field '" + field.name + "' array must have at least " + std::to_string(field.validation.minLength) + " elements");
                return false;
            }
            if (field.validation.maxLength > 0 && value.size() > static_cast<size_t>(field.validation.maxLength))
            {
                errors.push_back("Field '" + field.name + "' array must have at most " + std::to_string(field.validation.maxLength) + " elements");
                return false;
            }
            break;
            
        case ProfileFieldType::Object:
            if (!value.is_object())
            {
                errors.push_back("Field '" + field.name + "' must be an object");
                return false;
            }
            break;
    }
    
    return true;
}

std::string DynamicItemJsonParser::CleanJsonText(const std::string& jsonText)
{
    // Use existing StringUtils function
    return StringUtils::CleanJsonArrayText(jsonText);
}

void DynamicItemJsonParser::EnsureIdAndDisplayName(
    nlohmann::json& item,
    const ItemProfile& profile,
    size_t index)
{
    // Generate item type prefix (lowercase, no spaces)
    std::string itemTypePrefix = profile.itemTypeName;
    std::transform(itemTypePrefix.begin(), itemTypePrefix.end(), itemTypePrefix.begin(), ::tolower);
    // Remove spaces and special characters
    itemTypePrefix.erase(std::remove_if(itemTypePrefix.begin(), itemTypePrefix.end(),
        [](char c) { return !std::isalnum(c); }), itemTypePrefix.end());
    
    // Ensure displayName field exists first (needed for ID generation)
    if (!item.contains("displayName") || item["displayName"].is_null() ||
        (item["displayName"].is_string() && item["displayName"].get<std::string>().empty()))
    {
        // Try to generate from other fields that might contain a name
        std::string displayName;
        
        // Check common name fields
        std::vector<std::string> nameFields = {"name", "title", "weaponName", "itemName", "foodName"};
        for (const auto& nameField : nameFields)
        {
            if (item.contains(nameField) && item[nameField].is_string())
            {
                displayName = item[nameField].get<std::string>();
                break;
            }
        }
        
        // If still empty, use a generic name based on item type
        if (displayName.empty())
        {
            displayName = profile.itemTypeName + " Item " + std::to_string(index + 1);
        }
        
        item["displayName"] = displayName;
    }
    
    // ALWAYS generate ID from displayName (ignore LLM-generated ID to ensure consistency)
    if (item.contains("displayName") && item["displayName"].is_string())
    {
        std::string displayName = item["displayName"].get<std::string>();
        if (!displayName.empty())
        {
            // Generate ID from displayName - extract key identifiers only
            std::string idSuffix = GenerateShortIdFromDisplayName(displayName);
            
            // Limit length to avoid too long IDs (max 30 chars for suffix)
            if (idSuffix.length() > 30)
            {
                idSuffix = idSuffix.substr(0, 30);
            }
            
            // If suffix is empty after cleaning, use index as fallback
            if (idSuffix.empty())
            {
                idSuffix = std::to_string(index + 1);
            }
            
            std::ostringstream idStream;
            idStream << itemTypePrefix << "_" << idSuffix;
            item["id"] = idStream.str();
        }
    }
    
    // Fallback: if displayName is not available, generate from index
    if (!item.contains("id") || item["id"].is_null() || 
        (item["id"].is_string() && item["id"].get<std::string>().empty()))
    {
        std::ostringstream idStream;
        idStream << itemTypePrefix << "_" << std::setfill('0') << std::setw(3) << (index + 1);
        item["id"] = idStream.str();
    }
}

std::string DynamicItemJsonParser::GenerateShortIdFromDisplayName(const std::string& displayName)
{
    // Common words to remove (generic descriptors)
    std::vector<std::string> commonWords = {
        "enhanced", "advanced", "professional", "premium", "standard", "basic",
        "deluxe", "ultimate", "superior", "elite", "master", "expert",
        "semiautomatic", "automatic", "semi-auto", "full-auto",
        "assault", "rifle", "pistol", "carbine", "shotgun", "sniper",
        "model", "mk", "mark", "version", "ver", "v", "edition", "ed",
        "lever-action", "bolt-action", "pump-action", "action"
    };
    
    // Manufacturer abbreviations
    std::map<std::string, std::string> manufacturerAbbrevs = {
        {"heckler", "hk"}, {"koch", ""}, {"&", ""}, {"and", ""},
        {"colt", "colt"}, {"sig", "sig"}, {"sauer", ""},
        {"winchester", "win"}, {"remington", "rem"},
        {"fn", "fn"}, {"herstal", ""},
        {"glock", "glock"}, {"beretta", "ber"}, {"smith", "sw"}, {"wesson", ""}
    };
    
    std::string result = displayName;
    
    // Convert to lowercase
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    
    // Remove common words
    for (const auto& word : commonWords)
    {
        size_t pos = 0;
        while ((pos = result.find(word, pos)) != std::string::npos)
        {
            // Check if it's a whole word (surrounded by non-alphanumeric or at boundaries)
            bool isWholeWord = true;
            if (pos > 0 && std::isalnum(result[pos - 1]))
                isWholeWord = false;
            if (pos + word.length() < result.length() && std::isalnum(result[pos + word.length()]))
                isWholeWord = false;
            
            if (isWholeWord)
            {
                result.erase(pos, word.length());
                // Remove following space/hyphen if exists
                if (pos < result.length() && (result[pos] == ' ' || result[pos] == '-'))
                    result.erase(pos, 1);
            }
            else
            {
                pos += word.length();
            }
        }
    }
    
    // Apply manufacturer abbreviations
    for (const auto& [full, abbrev] : manufacturerAbbrevs)
    {
        size_t pos = 0;
        while ((pos = result.find(full, pos)) != std::string::npos)
        {
            bool isWholeWord = true;
            if (pos > 0 && std::isalnum(result[pos - 1]))
                isWholeWord = false;
            if (pos + full.length() < result.length() && std::isalnum(result[pos + full.length()]))
                isWholeWord = false;
            
            if (isWholeWord)
            {
                result.erase(pos, full.length());
                if (!abbrev.empty())
                {
                    result.insert(pos, abbrev);
                    pos += abbrev.length();
                }
                // Remove following space/hyphen if exists
                if (pos < result.length() && (result[pos] == ' ' || result[pos] == '-'))
                    result.erase(pos, 1);
            }
            else
            {
                pos += full.length();
            }
        }
    }
    
    // Remove all spaces, hyphens, and special characters, keep only alphanumeric
    result.erase(std::remove_if(result.begin(), result.end(),
        [](char c) { return !std::isalnum(c); }), result.end());
    
    return result;
}

