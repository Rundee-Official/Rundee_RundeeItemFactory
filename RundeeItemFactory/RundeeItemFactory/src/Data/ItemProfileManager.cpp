/**
 * @file ItemProfileManager.cpp
 * @brief Implementation of profile manager
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Data/ItemProfileManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <set>

std::string ItemProfileManager::s_profilesDir;

bool ItemProfileManager::Initialize(const std::string& profilesDir)
{
    s_profilesDir = profilesDir;
    
    // Ensure directory exists
    if (!std::filesystem::exists(profilesDir))
    {
        std::filesystem::create_directories(profilesDir);
    }
    
    // Create default profiles if directory is empty
    if (std::filesystem::is_empty(profilesDir))
    {
        CreateDefaultProfiles(profilesDir);
    }
    
    return true;
}

ItemProfile ItemProfileManager::LoadProfile(const std::string& profileId)
{
    std::string filePath = GetProfileFilePath(profileId);
    return LoadProfileFromPath(filePath);
}

ItemProfile ItemProfileManager::LoadProfileFromPath(const std::string& filePath)
{
    ItemProfile profile;
    
    if (!std::filesystem::exists(filePath))
    {
        return profile;
    }
    
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return profile;
    }
    
    try
    {
        nlohmann::json j;
        file >> j;
        profile = ParseProfileFromJson(j);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemProfileManager] Failed to parse profile: " << e.what() << "\n";
    }
    
    return profile;
}

bool ItemProfileManager::SaveProfile(const ItemProfile& profile)
{
    if (profile.id.empty())
    {
        return false;
    }
    
    std::string filePath = GetProfileFilePath(profile.id);
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }
    
    try
    {
        nlohmann::json j = ProfileToJson(profile);
        file << j.dump(2);
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemProfileManager] Failed to save profile: " << e.what() << "\n";
        return false;
    }
}

ItemProfile ItemProfileManager::GetDefaultProfile(const std::string& itemTypeName)
{
    std::string defaultId = "default_" + itemTypeName;
    std::transform(defaultId.begin(), defaultId.end(), defaultId.begin(), ::tolower);
    return LoadProfile(defaultId);
}

std::vector<ItemProfile> ItemProfileManager::GetProfilesForType(const std::string& itemTypeName)
{
    std::vector<ItemProfile> profiles;
    auto all = GetAllProfiles();
    for (const auto& [id, profile] : all)
    {
        if (profile.itemTypeName == itemTypeName)
        {
            profiles.push_back(profile);
        }
    }
    return profiles;
}

std::map<std::string, ItemProfile> ItemProfileManager::GetAllProfiles()
{
    std::map<std::string, ItemProfile> profiles;
    
    if (!std::filesystem::exists(s_profilesDir))
    {
        return profiles;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(s_profilesDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            std::string filePath = entry.path().string();
            ItemProfile profile = LoadProfileFromPath(filePath);
            if (!profile.id.empty())
            {
                profiles[profile.id] = profile;
            }
        }
    }
    
    return profiles;
}

bool ItemProfileManager::ProfileExists(const std::string& profileId)
{
    std::string filePath = GetProfileFilePath(profileId);
    return std::filesystem::exists(filePath);
}

bool ItemProfileManager::DeleteProfile(const std::string& profileId)
{
    std::string filePath = GetProfileFilePath(profileId);
    if (std::filesystem::exists(filePath))
    {
        return std::filesystem::remove(filePath);
    }
    return false;
}

bool ItemProfileManager::ValidateProfile(const ItemProfile& profile, std::vector<std::string>& errors)
{
    errors.clear();
    
    if (profile.id.empty())
    {
        errors.push_back("Profile ID is required");
    }
    
    if (profile.itemTypeName.empty())
    {
        errors.push_back("Item type name is required");
    }
    
    if (profile.fields.empty())
    {
        errors.push_back("Profile must have at least one field");
    }
    
    // Check for duplicate field names
    std::set<std::string> fieldNames;
    for (const auto& field : profile.fields)
    {
        if (fieldNames.count(field.name))
        {
            errors.push_back("Duplicate field name: " + field.name);
        }
        fieldNames.insert(field.name);
    }
    
    return errors.empty();
}

std::string ItemProfileManager::GetProfilesDirectory()
{
    return s_profilesDir;
}

bool ItemProfileManager::CreateDefaultProfiles(const std::string& profilesDir)
{
    // Create default_food profile
    ItemProfile foodProfile;
    foodProfile.id = "default_food";
    foodProfile.displayName = "Default Food Profile";
    foodProfile.description = "Default profile for Food items";
    foodProfile.itemTypeName = "Food";
    foodProfile.version = 1;
    foodProfile.isDefault = true;
    
    // Add fields
    ProfileField idField;
    idField.name = "id";
    idField.type = ProfileFieldType::String;
    idField.displayName = "ID";
    idField.description = "Unique item identifier";
    idField.category = "Identity";
    idField.displayOrder = 0;
    idField.validation.isRequired = true;
    foodProfile.fields.push_back(idField);
    
    ProfileField displayNameField;
    displayNameField.name = "displayName";
    displayNameField.type = ProfileFieldType::String;
    displayNameField.displayName = "Display Name";
    displayNameField.description = "User-facing name";
    displayNameField.category = "Identity";
    displayNameField.displayOrder = 1;
    displayNameField.validation.isRequired = true;
    foodProfile.fields.push_back(displayNameField);
    
    ProfileField categoryField;
    categoryField.name = "category";
    categoryField.type = ProfileFieldType::String;
    categoryField.displayName = "Category";
    categoryField.description = "Item category";
    categoryField.category = "Identity";
    categoryField.displayOrder = 2;
    categoryField.validation.isRequired = true;
    categoryField.validation.allowedValues = {"Food"};
    foodProfile.fields.push_back(categoryField);
    
    ProfileField rarityField;
    rarityField.name = "rarity";
    rarityField.type = ProfileFieldType::String;
    rarityField.displayName = "Rarity";
    rarityField.description = "Item rarity";
    rarityField.category = "Identity";
    rarityField.displayOrder = 3;
    rarityField.validation.isRequired = true;
    rarityField.validation.allowedValues = {"Common", "Uncommon", "Rare"};
    foodProfile.fields.push_back(rarityField);
    
    ProfileField maxStackField;
    maxStackField.name = "maxStack";
    maxStackField.type = ProfileFieldType::Integer;
    maxStackField.displayName = "Max Stack";
    maxStackField.description = "Maximum stack size";
    maxStackField.category = "Inventory";
    maxStackField.displayOrder = 4;
    maxStackField.defaultValue = 1;
    maxStackField.validation.minValue = 1.0;
    maxStackField.validation.maxValue = 999.0;
    foodProfile.fields.push_back(maxStackField);
    
    ProfileField descriptionField;
    descriptionField.name = "description";
    descriptionField.type = ProfileFieldType::String;
    descriptionField.displayName = "Description";
    descriptionField.description = "Item description";
    descriptionField.category = "Identity";
    descriptionField.displayOrder = 99;
    descriptionField.validation.isRequired = true;
    foodProfile.fields.push_back(descriptionField);
    
    ProfileField hungerRestoreField;
    hungerRestoreField.name = "hungerRestore";
    hungerRestoreField.type = ProfileFieldType::Integer;
    hungerRestoreField.displayName = "Hunger Restore";
    hungerRestoreField.description = "Amount of hunger restored (0-100)";
    hungerRestoreField.category = "Effects";
    hungerRestoreField.displayOrder = 10;
    hungerRestoreField.validation.minValue = 0.0;
    hungerRestoreField.validation.maxValue = 100.0;
    RelationshipConstraint hungerConstraint;
    hungerConstraint.description = "Food items primarily restore hunger, so hungerRestore should be >= thirstRestore";
    hungerConstraint.operator_ = ">=";
    hungerConstraint.targetField = "thirstRestore";
    hungerRestoreField.validation.relationshipConstraints.push_back(hungerConstraint);
    foodProfile.fields.push_back(hungerRestoreField);
    
    ProfileField thirstRestoreField;
    thirstRestoreField.name = "thirstRestore";
    thirstRestoreField.type = ProfileFieldType::Integer;
    thirstRestoreField.displayName = "Thirst Restore";
    thirstRestoreField.description = "Amount of thirst restored (0-100)";
    thirstRestoreField.category = "Effects";
    thirstRestoreField.displayOrder = 11;
    thirstRestoreField.validation.minValue = 0.0;
    thirstRestoreField.validation.maxValue = 100.0;
    foodProfile.fields.push_back(thirstRestoreField);
    
    ProfileField healthRestoreField;
    healthRestoreField.name = "healthRestore";
    healthRestoreField.type = ProfileFieldType::Integer;
    healthRestoreField.displayName = "Health Restore";
    healthRestoreField.description = "Amount of health restored (0-100)";
    healthRestoreField.category = "Effects";
    healthRestoreField.displayOrder = 12;
    healthRestoreField.validation.minValue = 0.0;
    healthRestoreField.validation.maxValue = 100.0;
    foodProfile.fields.push_back(healthRestoreField);
    
    ProfileField spoilsField;
    spoilsField.name = "spoils";
    spoilsField.type = ProfileFieldType::Boolean;
    spoilsField.displayName = "Spoils";
    spoilsField.description = "Whether this food item spoils over time";
    spoilsField.category = "Spoilage";
    spoilsField.displayOrder = 20;
    foodProfile.fields.push_back(spoilsField);
    
    ProfileField spoilTimeField;
    spoilTimeField.name = "spoilTimeMinutes";
    spoilTimeField.type = ProfileFieldType::Integer;
    spoilTimeField.displayName = "Spoil Time (minutes)";
    spoilTimeField.description = "Time until spoilage in minutes";
    spoilTimeField.category = "Spoilage";
    spoilTimeField.displayOrder = 21;
    spoilTimeField.validation.minValue = 0.0;
    spoilTimeField.validation.maxValue = 10000.0;
    foodProfile.fields.push_back(spoilTimeField);
    
    // Set player settings
    foodProfile.playerSettings["maxHunger"] = 100;
    foodProfile.playerSettings["maxThirst"] = 100;
    foodProfile.playerSettings["maxHealth"] = 100;
    foodProfile.playerSettings["maxStamina"] = 100;
    foodProfile.playerSettings["maxWeight"] = 50000;
    foodProfile.playerSettings["maxEnergy"] = 100;
    
    SaveProfile(foodProfile);
    std::cout << "[ItemProfileManager] Created default profile: " << foodProfile.id << "\n";
    
    // Create other default profiles similarly (simplified for now)
    // In production, load from JSON files in profiles/ directory
    
    return true;
}

std::string ItemProfileManager::GetProfileFilePath(const std::string& profileId)
{
    return s_profilesDir + "/" + profileId + ".json";
}

ItemProfile ItemProfileManager::ParseProfileFromJson(const nlohmann::json& json)
{
    ItemProfile profile;
    
    if (json.contains("id") && json["id"].is_string())
        profile.id = json["id"];
    if (json.contains("displayName") && json["displayName"].is_string())
        profile.displayName = json["displayName"];
    if (json.contains("description") && json["description"].is_string())
        profile.description = json["description"];
    if (json.contains("itemTypeName") && json["itemTypeName"].is_string())
        profile.itemTypeName = json["itemTypeName"];
    if (json.contains("version") && json["version"].is_number())
        profile.version = json["version"];
    if (json.contains("isDefault") && json["isDefault"].is_boolean())
        profile.isDefault = json["isDefault"];
    if (json.contains("customContext") && json["customContext"].is_string())
        profile.customContext = json["customContext"];
    
    if (json.contains("fields") && json["fields"].is_array())
    {
        for (const auto& fieldJson : json["fields"])
        {
            profile.fields.push_back(ParseFieldFromJson(fieldJson));
        }
    }
    
    if (json.contains("playerSettings") && json["playerSettings"].is_object())
    {
        for (const auto& [key, value] : json["playerSettings"].items())
        {
            if (value.is_number_integer())
            {
                profile.playerSettings[key] = value.get<int>();
            }
        }
    }
    
    return profile;
}

nlohmann::json ItemProfileManager::ProfileToJson(const ItemProfile& profile)
{
    nlohmann::json j;
    j["id"] = profile.id;
    j["displayName"] = profile.displayName;
    j["description"] = profile.description;
    j["itemTypeName"] = profile.itemTypeName;
    j["version"] = profile.version;
    j["isDefault"] = profile.isDefault;
    if (!profile.customContext.empty())
        j["customContext"] = profile.customContext;
    
    j["fields"] = nlohmann::json::array();
    for (const auto& field : profile.fields)
    {
        j["fields"].push_back(FieldToJson(field));
    }
    
    j["playerSettings"] = nlohmann::json::object();
    for (const auto& [key, value] : profile.playerSettings)
    {
        j["playerSettings"][key] = value;
    }
    
    j["metadata"] = profile.metadata;
    
    return j;
}

ProfileField ItemProfileManager::ParseFieldFromJson(const nlohmann::json& json)
{
    ProfileField field;
    
    if (json.contains("name") && json["name"].is_string())
        field.name = json["name"];
    if (json.contains("displayName") && json["displayName"].is_string())
        field.displayName = json["displayName"];
    if (json.contains("description") && json["description"].is_string())
        field.description = json["description"];
    if (json.contains("category") && json["category"].is_string())
        field.category = json["category"];
    if (json.contains("displayOrder") && json["displayOrder"].is_number())
        field.displayOrder = json["displayOrder"];
    if (json.contains("defaultValue"))
        field.defaultValue = json["defaultValue"];
    
    // Parse type
    if (json.contains("type") && json["type"].is_string())
    {
        std::string typeStr = json["type"];
        if (typeStr == "string")
            field.type = ProfileFieldType::String;
        else if (typeStr == "integer")
            field.type = ProfileFieldType::Integer;
        else if (typeStr == "float")
            field.type = ProfileFieldType::Float;
        else if (typeStr == "boolean")
            field.type = ProfileFieldType::Boolean;
        else if (typeStr == "array")
            field.type = ProfileFieldType::Array;
        else if (typeStr == "object")
            field.type = ProfileFieldType::Object;
    }
    
    if (json.contains("validation") && json["validation"].is_object())
    {
        field.validation = ParseValidationFromJson(json["validation"]);
    }
    
    return field;
}

nlohmann::json ItemProfileManager::FieldToJson(const ProfileField& field)
{
    nlohmann::json j;
    j["name"] = field.name;
    j["displayName"] = field.displayName;
    j["description"] = field.description;
    j["category"] = field.category;
    j["displayOrder"] = field.displayOrder;
    if (!field.defaultValue.is_null())
        j["defaultValue"] = field.defaultValue;
    
    // Type to string
    switch (field.type)
    {
        case ProfileFieldType::String: j["type"] = "string"; break;
        case ProfileFieldType::Integer: j["type"] = "integer"; break;
        case ProfileFieldType::Float: j["type"] = "float"; break;
        case ProfileFieldType::Boolean: j["type"] = "boolean"; break;
        case ProfileFieldType::Array: j["type"] = "array"; break;
        case ProfileFieldType::Object: j["type"] = "object"; break;
    }
    
    j["validation"] = ValidationToJson(field.validation);
    
    return j;
}

ProfileFieldValidation ItemProfileManager::ParseValidationFromJson(const nlohmann::json& json)
{
    ProfileFieldValidation validation;
    
    if (json.contains("isRequired") && json["isRequired"].is_boolean())
        validation.isRequired = json["isRequired"];
    if (json.contains("minLength") && json["minLength"].is_number())
        validation.minLength = json["minLength"];
    if (json.contains("maxLength") && json["maxLength"].is_number())
        validation.maxLength = json["maxLength"];
    if (json.contains("minValue") && json["minValue"].is_number())
        validation.minValue = json["minValue"];
    if (json.contains("maxValue") && json["maxValue"].is_number())
        validation.maxValue = json["maxValue"];
    if (json.contains("customConstraint") && json["customConstraint"].is_string())
        validation.customConstraint = json["customConstraint"];
    
    if (json.contains("allowedValues") && json["allowedValues"].is_array())
    {
        for (const auto& val : json["allowedValues"])
        {
            if (val.is_string())
                validation.allowedValues.push_back(val.get<std::string>());
        }
    }
    
    if (json.contains("relationshipConstraints") && json["relationshipConstraints"].is_array())
    {
        for (const auto& constraintJson : json["relationshipConstraints"])
        {
            RelationshipConstraint constraint;
            if (constraintJson.contains("description") && constraintJson["description"].is_string())
                constraint.description = constraintJson["description"];
            if (constraintJson.contains("operator") && constraintJson["operator"].is_string())
                constraint.operator_ = constraintJson["operator"];
            if (constraintJson.contains("targetField") && constraintJson["targetField"].is_string())
                constraint.targetField = constraintJson["targetField"];
            validation.relationshipConstraints.push_back(constraint);
        }
    }
    
    return validation;
}

nlohmann::json ItemProfileManager::ValidationToJson(const ProfileFieldValidation& validation)
{
    nlohmann::json j;
    j["isRequired"] = validation.isRequired;
    j["minLength"] = validation.minLength;
    j["maxLength"] = validation.maxLength;
    j["minValue"] = validation.minValue;
    j["maxValue"] = validation.maxValue;
    if (!validation.customConstraint.empty())
        j["customConstraint"] = validation.customConstraint;
    
    j["allowedValues"] = nlohmann::json::array();
    for (const auto& val : validation.allowedValues)
    {
        j["allowedValues"].push_back(val);
    }
    
    j["relationshipConstraints"] = nlohmann::json::array();
    for (const auto& constraint : validation.relationshipConstraints)
    {
        nlohmann::json c;
        c["description"] = constraint.description;
        c["operator"] = constraint.operator_;
        c["targetField"] = constraint.targetField;
        j["relationshipConstraints"].push_back(c);
    }
    
    return j;
}

ProfileField ItemProfileManager::CreateBaseField(const std::string& name, const std::string& displayName,
    const std::string& description, const std::string& category, int order, bool required,
    const std::vector<std::string>& allowedValues, int defaultValue, int minValue, int maxValue)
{
    ProfileField field;
    field.name = name;
    field.displayName = displayName;
    field.description = description;
    field.category = category;
    field.displayOrder = order;
    field.type = ProfileFieldType::String;
    field.validation.isRequired = required;
    field.validation.allowedValues = allowedValues;
    field.validation.minValue = minValue;
    field.validation.maxValue = maxValue;
    return field;
}

ProfileField ItemProfileManager::CreateField(const std::string& name, ProfileFieldType type,
    const std::string& displayName, const std::string& description, const std::string& category,
    int order, bool required, double minValue, double maxValue)
{
    ProfileField field;
    field.name = name;
    field.type = type;
    field.displayName = displayName;
    field.description = description;
    field.category = category;
    field.displayOrder = order;
    field.validation.isRequired = required;
    field.validation.minValue = minValue;
    field.validation.maxValue = maxValue;
    return field;
}
