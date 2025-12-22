/**
 * @file ItemProfileManager.cpp
 * @brief Implementation of item profile manager
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Data/ItemProfileManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <set>
#include <iostream>
#include <json.hpp>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

std::string ItemProfileManager::s_profilesDir = "profiles/";

bool ItemProfileManager::Initialize(const std::string& profilesDir)
{
    s_profilesDir = profilesDir;
    
    // Ensure directory exists
    if (!std::filesystem::exists(s_profilesDir))
    {
        std::filesystem::create_directories(s_profilesDir);
    }
    
    // Create default profiles if they don't exist
    CreateDefaultProfiles(s_profilesDir);
    
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
    
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return profile; // Return empty profile
    }
    
    try
    {
        nlohmann::json json;
        file >> json;
        file.close();
        
        profile = ParseProfileFromJson(json);
    }
    catch (const std::exception& e)
    {
        // Return empty profile on error
    }
    
    return profile;
}

bool ItemProfileManager::SaveProfile(const ItemProfile& profile)
{
    std::vector<std::string> errors;
    if (!ValidateProfile(profile, errors))
    {
        return false;
    }
    
    std::string filePath = GetProfileFilePath(profile.id);
    
    // Ensure directory exists
    std::filesystem::path dirPath = std::filesystem::path(filePath).parent_path();
    if (!std::filesystem::exists(dirPath))
    {
        std::filesystem::create_directories(dirPath);
    }
    
    try
    {
        nlohmann::json json = ProfileToJson(profile);
        
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            return false;
        }
        
        file << json.dump(2); // Pretty print with 2 spaces
        file.close();
        
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

ItemProfile ItemProfileManager::GetDefaultProfile(const std::string& itemTypeName)
{
    // Try to load "default_<itemType>" profile
    std::string defaultId = "default_" + itemTypeName;
    std::transform(defaultId.begin(), defaultId.end(), defaultId.begin(), ::tolower);
    
    ItemProfile profile = LoadProfile(defaultId);
    if (!profile.id.empty())
    {
        return profile;
    }
    
    // Fallback: search for any profile with isDefault=true for this type
    auto allProfiles = GetAllProfiles();
    for (const auto& [id, prof] : allProfiles)
    {
        if (prof.itemTypeName == itemTypeName && prof.isDefault)
        {
            return prof;
        }
    }
    
    // If no profile found, try to create default profiles automatically
    std::cout << "[ItemProfileManager] No default profile found for '" << itemTypeName << "'. Creating default profiles...\n";
    if (CreateDefaultProfiles(s_profilesDir))
    {
        // Try loading again after creation
        profile = LoadProfile(defaultId);
        if (!profile.id.empty())
        {
            std::cout << "[ItemProfileManager] Successfully created and loaded default profile: " << defaultId << "\n";
            return profile;
        }
    }
    
    // Return empty profile if still not found
    return ItemProfile();
}

std::vector<ItemProfile> ItemProfileManager::GetProfilesForType(const std::string& itemTypeName)
{
    std::vector<ItemProfile> result;
    
    if (!std::filesystem::exists(s_profilesDir))
    {
        return result;
    }
    
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(s_profilesDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                ItemProfile profile = LoadProfileFromPath(entry.path().string());
                if (!profile.id.empty() && profile.itemTypeName == itemTypeName)
                {
                    result.push_back(profile);
                }
            }
        }
    }
    catch (const std::exception&)
    {
        // Return empty vector on error
    }
    
    return result;
}

std::map<std::string, ItemProfile> ItemProfileManager::GetAllProfiles()
{
    std::map<std::string, ItemProfile> result;
    
    if (!std::filesystem::exists(s_profilesDir))
    {
        return result;
    }
    
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(s_profilesDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                ItemProfile profile = LoadProfileFromPath(entry.path().string());
                if (!profile.id.empty())
                {
                    result[profile.id] = profile;
                }
            }
        }
    }
    catch (const std::exception&)
    {
        // Return empty map on error
    }
    
    return result;
}

bool ItemProfileManager::ProfileExists(const std::string& profileId)
{
    std::string filePath = GetProfileFilePath(profileId);
    return std::filesystem::exists(filePath);
}

bool ItemProfileManager::DeleteProfile(const std::string& profileId)
{
    std::string filePath = GetProfileFilePath(profileId);
    
    try
    {
        if (std::filesystem::exists(filePath))
        {
            std::filesystem::remove(filePath);
            return true;
        }
    }
    catch (const std::exception&)
    {
        // Return false on error
    }
    
    return false;
}

bool ItemProfileManager::ValidateProfile(const ItemProfile& profile, std::vector<std::string>& errors)
{
    errors.clear();
    
    if (profile.id.empty())
    {
        errors.push_back("Profile ID cannot be empty");
    }
    
    if (profile.itemTypeName.empty())
    {
        errors.push_back("Item type name cannot be empty");
    }
    
    if (profile.fields.empty())
    {
        errors.push_back("Profile must have at least one field");
    }
    
    // Check for duplicate field names
    std::set<std::string> fieldNames;
    for (const auto& field : profile.fields)
    {
        if (field.name.empty())
        {
            errors.push_back("Field name cannot be empty");
        }
        
        if (fieldNames.find(field.name) != fieldNames.end())
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
    if (!std::filesystem::exists(profilesDir))
    {
        std::filesystem::create_directories(profilesDir);
    }
    
    // Create default profiles for all item types
    std::vector<std::string> itemTypes = { "Food", "Drink", "Medicine", "Material", "Weapon", "WeaponComponent", "Ammo", "Armor", "Clothing" };
    
    for (const auto& typeName : itemTypes)
    {
        std::string profileId = "default_" + typeName;
        std::transform(profileId.begin(), profileId.end(), profileId.begin(), ::tolower);
        
        // Skip if profile already exists
        if (ProfileExists(profileId))
        {
            continue;
        }
        
        ItemProfile profile;
        profile.id = profileId;
        profile.displayName = "Default " + typeName + " Profile";
        profile.description = "Default profile for " + typeName + " items matching the original hardcoded structure";
        profile.itemTypeName = typeName;
        profile.version = 1;
        profile.isDefault = true;
        
        // Add common base fields
        profile.fields.push_back(CreateBaseField("id", "ID", "Unique item identifier", "Identity", 0, true));
        profile.fields.push_back(CreateBaseField("displayName", "Display Name", "User-facing name", "Identity", 1, true));
        profile.fields.push_back(CreateBaseField("category", "Category", "Item category", "Identity", 2, true, {typeName}));
        profile.fields.push_back(CreateBaseField("rarity", "Rarity", "Item rarity", "Identity", 3, true, {"Common", "Uncommon", "Rare"}));
        profile.fields.push_back(CreateBaseField("maxStack", "Max Stack", "Maximum stack size", "Inventory", 4, false, std::vector<std::string>(), 1, 1, 999));
        profile.fields.push_back(CreateBaseField("description", "Description", "Item description", "Identity", 99, true));
        
        // Add type-specific fields
        if (typeName == "Food")
        {
            ProfileField hungerField = CreateField("hungerRestore", ProfileFieldType::Integer, "Hunger Restore", "Amount of hunger restored (0-100)", "Effects", 10, false, 0, 100);
            ProfileField thirstField = CreateField("thirstRestore", ProfileFieldType::Integer, "Thirst Restore", "Amount of thirst restored (0-100)", "Effects", 11, false, 0, 100);
            
            // Add relationship constraint: hungerRestore >= thirstRestore (Food should restore more hunger)
            FieldRelationshipConstraint hungerConstraint;
            hungerConstraint.operator_ = ">=";
            hungerConstraint.targetField = "thirstRestore";
            hungerConstraint.description = "Food items primarily restore hunger, so hungerRestore should be >= thirstRestore";
            hungerField.validation.relationshipConstraints.push_back(hungerConstraint);
            
            profile.fields.push_back(hungerField);
            profile.fields.push_back(thirstField);
            profile.fields.push_back(CreateField("healthRestore", ProfileFieldType::Integer, "Health Restore", "Amount of health restored (0-100)", "Effects", 12, false, 0, 100));
            profile.fields.push_back(CreateField("spoils", ProfileFieldType::Boolean, "Spoils", "Whether this food item spoils over time", "Spoilage", 20, false));
            profile.fields.push_back(CreateField("spoilTimeMinutes", ProfileFieldType::Integer, "Spoil Time (minutes)", "Time until spoilage in minutes", "Spoilage", 21, false, 0, 10000));
        }
        else if (typeName == "Drink")
        {
            ProfileField thirstField = CreateField("thirstRestore", ProfileFieldType::Integer, "Thirst Restore", "Amount of thirst restored (0-100)", "Effects", 11, false, 0, 100);
            ProfileField hungerField = CreateField("hungerRestore", ProfileFieldType::Integer, "Hunger Restore", "Amount of hunger restored (0-100)", "Effects", 10, false, 0, 100);
            
            // Add relationship constraints: thirstRestore >= hungerRestore AND thirstRestore >= 10
            FieldRelationshipConstraint thirstConstraint1;
            thirstConstraint1.operator_ = ">=";
            thirstConstraint1.targetField = "hungerRestore";
            thirstConstraint1.description = "Drink items primarily restore thirst, so thirstRestore should be >= hungerRestore";
            thirstField.validation.relationshipConstraints.push_back(thirstConstraint1);
            
            // Minimum value constraint (thirstRestore >= 10)
            thirstField.validation.minValue = 10.0;
            
            profile.fields.push_back(hungerField);
            profile.fields.push_back(thirstField);
            profile.fields.push_back(CreateField("healthRestore", ProfileFieldType::Integer, "Health Restore", "Amount of health restored (0-100)", "Effects", 12, false, 0, 100));
            profile.fields.push_back(CreateField("spoils", ProfileFieldType::Boolean, "Spoils", "Whether this drink item spoils over time", "Spoilage", 20, false));
            profile.fields.push_back(CreateField("spoilTimeMinutes", ProfileFieldType::Integer, "Spoil Time (minutes)", "Time until spoilage in minutes", "Spoilage", 21, false, 0, 10000));
        }
        else if (typeName == "Medicine")
        {
            ProfileField healthField = CreateField("healthRestore", ProfileFieldType::Integer, "Health Restore", "Amount of health restored (0-100)", "Effects", 10, false, 0, 100);
            // Minimum value: healthRestore >= 10 for all medicine, >= 20 for Rare
            healthField.validation.minValue = 10.0;
            // Note: Rare-specific constraint would need to be checked at generation time or via customConstraint
            profile.fields.push_back(healthField);
        }
        else if (typeName == "Material")
        {
            profile.fields.push_back(CreateField("materialType", ProfileFieldType::String, "Material Type", "Type of material (e.g., Wood, Metal, Stone)", "Properties", 10, false));
            profile.fields.push_back(CreateField("hardness", ProfileFieldType::Integer, "Hardness", "Material hardness (0-100)", "Properties", 11, false, 0, 100));
            profile.fields.push_back(CreateField("flammability", ProfileFieldType::Integer, "Flammability", "Material flammability (0-100)", "Properties", 12, false, 0, 100));
            profile.fields.push_back(CreateField("value", ProfileFieldType::Integer, "Value", "Material value", "Properties", 13, false, 0, 1000));
        }
        // Note: Weapon, WeaponComponent, Ammo, Armor, Clothing are more complex
        // For now, just create basic structure - can be expanded later
        
        // Save profile
        if (!SaveProfile(profile))
        {
            std::cerr << "[ItemProfileManager] Warning: Failed to create default profile for " << typeName << "\n";
        }
        else
        {
            std::cout << "[ItemProfileManager] Created default profile: " << profileId << "\n";
        }
    }
    
    return true;
}

// Helper function to create base field
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
    
    if (!allowedValues.empty())
    {
        field.validation.allowedValues = allowedValues;
    }
    
    if (name == "maxStack")
    {
        field.type = ProfileFieldType::Integer;
        field.defaultValue = defaultValue;
        field.validation.minValue = minValue;
        field.validation.maxValue = maxValue;
    }
    else if (name == "category")
    {
        field.validation.allowedValues = allowedValues;
    }
    else if (name == "rarity")
    {
        field.validation.allowedValues = allowedValues;
    }
    
    return field;
}

// Helper function to create field
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
    
    if (type == ProfileFieldType::Integer || type == ProfileFieldType::Float)
    {
        field.validation.minValue = minValue;
        field.validation.maxValue = maxValue;
    }
    
    return field;
}

ItemProfile ItemProfileManager::ParseProfileFromJson(const nlohmann::json& json)
{
    ItemProfile profile;
    
    if (json.contains("id")) profile.id = json["id"].get<std::string>();
    if (json.contains("displayName")) profile.displayName = json["displayName"].get<std::string>();
    if (json.contains("description")) profile.description = json["description"].get<std::string>();
    if (json.contains("itemTypeName")) profile.itemTypeName = json["itemTypeName"].get<std::string>();
    if (json.contains("version")) profile.version = json["version"].get<int>();
    if (json.contains("isDefault")) profile.isDefault = json["isDefault"].get<bool>();
    if (json.contains("customContext")) profile.customContext = json["customContext"].get<std::string>();
    
    // Parse playerSettings
    if (json.contains("playerSettings") && json["playerSettings"].is_object())
    {
        const auto& ps = json["playerSettings"];
        profile.playerSettings.maxHunger = ps.value("maxHunger", 100);
        profile.playerSettings.maxThirst = ps.value("maxThirst", 100);
        profile.playerSettings.maxHealth = ps.value("maxHealth", 100);
        profile.playerSettings.maxStamina = ps.value("maxStamina", 100);
        profile.playerSettings.maxWeight = ps.value("maxWeight", 50000);
        profile.playerSettings.maxEnergy = ps.value("maxEnergy", 100);
    }
    
    if (json.contains("fields") && json["fields"].is_array())
    {
        for (const auto& fieldJson : json["fields"])
        {
            profile.fields.push_back(ParseFieldFromJson(fieldJson));
        }
    }
    
    if (json.contains("metadata") && json["metadata"].is_object())
    {
        for (const auto& [key, value] : json["metadata"].items())
        {
            if (value.is_string())
            {
                profile.metadata[key] = value.get<std::string>();
            }
        }
    }
    
    return profile;
}

nlohmann::json ItemProfileManager::ProfileToJson(const ItemProfile& profile)
{
    nlohmann::json json;
    
    json["id"] = profile.id;
    json["displayName"] = profile.displayName;
    json["description"] = profile.description;
    json["itemTypeName"] = profile.itemTypeName;
    json["version"] = profile.version;
    json["isDefault"] = profile.isDefault;
    if (!profile.customContext.empty())
    {
        json["customContext"] = profile.customContext;
    }
    
    // Add playerSettings
    json["playerSettings"] = nlohmann::json::object();
    json["playerSettings"]["maxHunger"] = profile.playerSettings.maxHunger;
    json["playerSettings"]["maxThirst"] = profile.playerSettings.maxThirst;
    json["playerSettings"]["maxHealth"] = profile.playerSettings.maxHealth;
    json["playerSettings"]["maxStamina"] = profile.playerSettings.maxStamina;
    json["playerSettings"]["maxWeight"] = profile.playerSettings.maxWeight;
    json["playerSettings"]["maxEnergy"] = profile.playerSettings.maxEnergy;
    
    json["fields"] = nlohmann::json::array();
    for (const auto& field : profile.fields)
    {
        json["fields"].push_back(FieldToJson(field));
    }
    
    json["metadata"] = nlohmann::json::object();
    for (const auto& [key, value] : profile.metadata)
    {
        json["metadata"][key] = value;
    }
    
    return json;
}

ProfileField ItemProfileManager::ParseFieldFromJson(const nlohmann::json& json)
{
    ProfileField field;
    
    if (json.contains("name")) field.name = json["name"].get<std::string>();
    if (json.contains("displayName")) field.displayName = json["displayName"].get<std::string>();
    if (json.contains("description")) field.description = json["description"].get<std::string>();
    if (json.contains("category")) field.category = json["category"].get<std::string>();
    if (json.contains("displayOrder")) field.displayOrder = json["displayOrder"].get<int>();
    
    if (json.contains("type"))
    {
        field.type = StringToProfileFieldType(json["type"].get<std::string>());
    }
    
    if (json.contains("defaultValue"))
    {
        field.defaultValue = json["defaultValue"];
    }
    
    if (json.contains("validation"))
    {
        field.validation = ParseValidationFromJson(json["validation"]);
    }
    
    return field;
}

nlohmann::json ItemProfileManager::FieldToJson(const ProfileField& field)
{
    nlohmann::json json;
    
    json["name"] = field.name;
    json["type"] = ProfileFieldTypeToString(field.type);
    json["displayName"] = field.displayName;
    json["description"] = field.description;
    json["category"] = field.category;
    json["displayOrder"] = field.displayOrder;
    json["defaultValue"] = field.defaultValue;
    json["validation"] = ValidationToJson(field.validation);
    
    return json;
}

ProfileFieldValidation ItemProfileManager::ParseValidationFromJson(const nlohmann::json& json)
{
    ProfileFieldValidation validation;
    
    if (json.contains("minValue")) validation.minValue = json["minValue"].get<double>();
    if (json.contains("maxValue")) validation.maxValue = json["maxValue"].get<double>();
    if (json.contains("isRequired")) validation.isRequired = json["isRequired"].get<bool>();
    if (json.contains("minLength")) validation.minLength = json["minLength"].get<int>();
    if (json.contains("maxLength")) validation.maxLength = json["maxLength"].get<int>();
    if (json.contains("regexPattern")) validation.regexPattern = json["regexPattern"].get<std::string>();
    if (json.contains("customConstraint")) validation.customConstraint = json["customConstraint"].get<std::string>();
    
    // Parse relationship constraints
    if (json.contains("relationshipConstraints") && json["relationshipConstraints"].is_array())
    {
        for (const auto& constraintJson : json["relationshipConstraints"])
        {
            FieldRelationshipConstraint constraint;
            if (constraintJson.contains("operator")) constraint.operator_ = constraintJson["operator"].get<std::string>();
            if (constraintJson.contains("targetField")) constraint.targetField = constraintJson["targetField"].get<std::string>();
            if (constraintJson.contains("offset")) constraint.offset = constraintJson["offset"].get<double>();
            if (constraintJson.contains("description")) constraint.description = constraintJson["description"].get<std::string>();
            validation.relationshipConstraints.push_back(constraint);
        }
    }
    
    if (json.contains("allowedValues") && json["allowedValues"].is_array())
    {
        for (const auto& value : json["allowedValues"])
        {
            if (value.is_string())
            {
                validation.allowedValues.push_back(value.get<std::string>());
            }
        }
    }
    
    return validation;
}

nlohmann::json ItemProfileManager::ValidationToJson(const ProfileFieldValidation& validation)
{
    nlohmann::json json;
    
    json["minValue"] = validation.minValue;
    json["maxValue"] = validation.maxValue;
    json["isRequired"] = validation.isRequired;
    json["minLength"] = validation.minLength;
    json["maxLength"] = validation.maxLength;
    if (!validation.regexPattern.empty())
    {
        json["regexPattern"] = validation.regexPattern;
    }
    if (!validation.customConstraint.empty())
    {
        json["customConstraint"] = validation.customConstraint;
    }
    
    if (!validation.allowedValues.empty())
    {
        json["allowedValues"] = nlohmann::json::array();
        for (const auto& value : validation.allowedValues)
        {
            json["allowedValues"].push_back(value);
        }
    }
    
    // Serialize relationship constraints
    if (!validation.relationshipConstraints.empty())
    {
        json["relationshipConstraints"] = nlohmann::json::array();
        for (const auto& constraint : validation.relationshipConstraints)
        {
            nlohmann::json constraintJson;
            constraintJson["operator"] = constraint.operator_;
            constraintJson["targetField"] = constraint.targetField;
            if (constraint.offset != 0.0)
            {
                constraintJson["offset"] = constraint.offset;
            }
            if (!constraint.description.empty())
            {
                constraintJson["description"] = constraint.description;
            }
            json["relationshipConstraints"].push_back(constraintJson);
        }
    }
    
    return json;
}

std::string ItemProfileManager::GetProfileFilePath(const std::string& profileId)
{
    return s_profilesDir + profileId + ".json";
}

