/**
 * @file ItemProfile.h
 * @brief Dynamic item profile system for custom item structures
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the structure for dynamic item profiles that allow users to define
 * custom item data structures. Profiles define fields, types, validation rules,
 * and default values for item generation.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <json.hpp>

/**
 * @enum ProfileFieldType
 * @brief Supported data types for profile fields
 */
enum class ProfileFieldType
{
    String,      ///< String field
    Integer,     ///< Integer field
    Float,       ///< Floating-point field
    Boolean,     ///< Boolean field
    Array,       ///< Array field (homogeneous)
    Object       ///< Object field (nested JSON object)
};

/**
 * @struct FieldRelationshipConstraint
 * @brief Relationship constraint between fields (e.g., fieldA >= fieldB)
 */
struct FieldRelationshipConstraint
{
    /** @brief Relationship operator: ">=", "<=", ">", "<", "==", "!=" */
    std::string operator_;
    
    /** @brief Target field name to compare against */
    std::string targetField;
    
    /** @brief Optional offset value (e.g., "fieldA >= fieldB + 5") */
    double offset = 0.0;
    
    /** @brief Description of the constraint (for prompt generation) */
    std::string description;
};

/**
 * @struct ProfileFieldValidation
 * @brief Validation rules for a profile field
 */
struct ProfileFieldValidation
{
    /** @brief Minimum value (for numeric types) */
    double minValue = 0.0;
    
    /** @brief Maximum value (for numeric types) */
    double maxValue = 0.0;
    
    /** @brief Required field flag */
    bool isRequired = false;
    
    /** @brief Allowed values (for enum-like validation) */
    std::vector<std::string> allowedValues;
    
    /** @brief Custom validation regex (for string types) */
    std::string regexPattern;
    
    /** @brief Minimum length (for string/array types) */
    int minLength = 0;
    
    /** @brief Maximum length (for string/array types) */
    int maxLength = 0;
    
    /** @brief Relationship constraints with other fields */
    std::vector<FieldRelationshipConstraint> relationshipConstraints;
    
    /** @brief Custom constraint expression (e.g., "this >= otherField * 2") */
    std::string customConstraint;
};

/**
 * @struct ProfileField
 * @brief Definition of a single field in an item profile
 */
struct ProfileField
{
    /** @brief Field name (used as JSON key) */
    std::string name;
    
    /** @brief Field data type */
    ProfileFieldType type;
    
    /** @brief Display name for UI */
    std::string displayName;
    
    /** @brief Description/help text */
    std::string description;
    
    /** @brief Default value (JSON value) */
    nlohmann::json defaultValue;
    
    /** @brief Validation rules */
    ProfileFieldValidation validation;
    
    /** @brief Category/group for organization in UI */
    std::string category;
    
    /** @brief Order/hint for UI display */
    int displayOrder = 0;
};

/**
 * @struct PlayerSettings
 * @brief Player statistics used for item generation
 * 
 * Defines maximum values for player attributes, used to balance
 * generated items appropriately.
 */
struct PlayerSettings
{
    /** @brief Maximum hunger value */
    int maxHunger = 100;
    
    /** @brief Maximum thirst value */
    int maxThirst = 100;
    
    /** @brief Maximum health points */
    int maxHealth = 100;
    
    /** @brief Maximum stamina points */
    int maxStamina = 100;
    
    /** @brief Maximum weight capacity (in grams) */
    int maxWeight = 50000;  // 50kg default
    
    /** @brief Maximum energy points */
    int maxEnergy = 100;
};

/**
 * @struct ItemProfile
 * @brief Complete item profile definition
 * 
 * Defines a complete structure for a type of item, including all fields,
 * validation rules, and metadata. Profiles are stored as JSON files and
 * loaded at runtime to generate items with custom structures.
 */
struct ItemProfile
{
    /** @brief Unique profile identifier */
    std::string id;
    
    /** @brief Display name */
    std::string displayName;
    
    /** @brief Profile description */
    std::string description;
    
    /** @brief Item type name (e.g., "Food", "Weapon", "CustomType") */
    std::string itemTypeName;
    
    /** @brief Profile version (for migration/validation) */
    int version = 1;
    
    /** @brief Whether this is the default profile for the item type */
    bool isDefault = false;
    
    /** @brief Custom world context/background text (user-defined) */
    std::string customContext;
    
    /** @brief List of field definitions */
    std::vector<ProfileField> fields;
    
    /** @brief Metadata (key-value pairs) */
    std::map<std::string, std::string> metadata;
    
    /** @brief Player settings for this profile */
    PlayerSettings playerSettings;
    
    /**
     * @brief Get field by name
     * @param name Field name
     * @return Pointer to field if found, nullptr otherwise
     */
    const ProfileField* GetField(const std::string& name) const
    {
        for (const auto& field : fields)
        {
            if (field.name == name)
                return &field;
        }
        return nullptr;
    }
    
    /**
     * @brief Check if profile has a field
     * @param name Field name
     * @return True if field exists
     */
    bool HasField(const std::string& name) const
    {
        return GetField(name) != nullptr;
    }
    
    /**
     * @brief Get all fields in a category
     * @param category Category name
     * @return Vector of field pointers in the category
     */
    std::vector<const ProfileField*> GetFieldsByCategory(const std::string& category) const
    {
        std::vector<const ProfileField*> result;
        for (const auto& field : fields)
        {
            if (field.category == category)
                result.push_back(&field);
        }
        return result;
    }
};

/**
 * @brief Convert ProfileFieldType to string
 * @param type Field type
 * @return String representation
 */
inline std::string ProfileFieldTypeToString(ProfileFieldType type)
{
    switch (type)
    {
        case ProfileFieldType::String:  return "string";
        case ProfileFieldType::Integer: return "integer";
        case ProfileFieldType::Float:   return "float";
        case ProfileFieldType::Boolean: return "boolean";
        case ProfileFieldType::Array:   return "array";
        case ProfileFieldType::Object:  return "object";
        default: return "unknown";
    }
}

/**
 * @brief Convert string to ProfileFieldType
 * @param str String representation
 * @return ProfileFieldType (defaults to String if invalid)
 */
inline ProfileFieldType StringToProfileFieldType(const std::string& str)
{
    if (str == "string") return ProfileFieldType::String;
    if (str == "integer" || str == "int") return ProfileFieldType::Integer;
    if (str == "float" || str == "double") return ProfileFieldType::Float;
    if (str == "boolean" || str == "bool") return ProfileFieldType::Boolean;
    if (str == "array") return ProfileFieldType::Array;
    if (str == "object") return ProfileFieldType::Object;
    return ProfileFieldType::String;
}

