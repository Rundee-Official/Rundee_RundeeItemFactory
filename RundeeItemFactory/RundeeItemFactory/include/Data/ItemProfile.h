/**
 * @file ItemProfile.h
 * @brief Item profile data structures for dynamic item generation
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines data structures for item profiles that describe the structure
 * and validation rules for dynamically generated items.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <json.hpp>

/**
 * @enum ProfileFieldType
 * @brief Type of a profile field
 */
enum class ProfileFieldType
{
    String = 0,
    Integer = 1,
    Float = 2,
    Boolean = 3,
    Array = 4,
    Object = 5
};

/**
 * @struct RelationshipConstraint
 * @brief Constraint between two fields
 */
struct RelationshipConstraint
{
    std::string description;
    std::string operator_; // ">=", "<=", "==", etc.
    std::string targetField;
};

/**
 * @struct ProfileFieldValidation
 * @brief Validation rules for a profile field
 */
struct ProfileFieldValidation
{
    bool isRequired = false;
    int minLength = 0;
    int maxLength = 0;
    double minValue = 0.0;
    double maxValue = 0.0;
    std::vector<std::string> allowedValues;
    std::vector<RelationshipConstraint> relationshipConstraints;
    std::string customConstraint;
};

/**
 * @struct ProfileField
 * @brief Definition of a single field in an item profile
 */
struct ProfileField
{
    std::string name;
    ProfileFieldType type;
    std::string displayName;
    std::string description;
    std::string category;
    int displayOrder;
    nlohmann::json defaultValue;
    ProfileFieldValidation validation;
};

/**
 * @struct ItemProfile
 * @brief Complete item profile defining structure and validation
 */
struct ItemProfile
{
    std::string id;
    std::string displayName;
    std::string description;
    std::string itemTypeName;
    int version = 1;
    bool isDefault = false;
    std::string customContext;
    std::vector<ProfileField> fields;
    std::map<std::string, nlohmann::json> metadata;
    std::map<std::string, int> playerSettings;
};
