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

