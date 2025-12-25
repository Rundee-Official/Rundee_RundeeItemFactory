/**
 * @file ItemProfileManager.h
 * @brief Manager for loading, saving, and managing item profiles
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides functionality to load, save, validate, and manage item profiles.
 * Profiles define custom item structures for dynamic item generation.
 */

#pragma once

#include "Data/ItemProfile.h"
#include <string>
#include <map>
#include <vector>

/**
 * @class ItemProfileManager
 * @brief Static manager class for item profiles
 * 
 * Handles loading, saving, validation, and lookup of item profiles.
 * Profiles are stored as JSON files in a profiles directory.
 */
class ItemProfileManager
{
public:
    /**
     * @brief Initialize the profile manager
     * @param profilesDir Directory containing profile files
     * @return True if initialization succeeded
     */
    static bool Initialize(const std::string& profilesDir);
    
    /**
     * @brief Load a profile from file
     * @param profileId Profile ID (filename without extension)
     * @return Loaded profile, or empty profile if not found
     */
    static ItemProfile LoadProfile(const std::string& profileId);
    
    /**
     * @brief Load a profile from file path
     * @param filePath Full path to profile file
     * @return Loaded profile, or empty profile if failed
     */
    static ItemProfile LoadProfileFromPath(const std::string& filePath);
    
    /**
     * @brief Save a profile to file
     * @param profile Profile to save
     * @return True if save succeeded
     */
    static bool SaveProfile(const ItemProfile& profile);
    
    /**
     * @brief Get default profile for an item type
     * @param itemTypeName Item type name (e.g., "Food", "Weapon")
     * @return Default profile for the type, or empty profile if not found
     */
    static ItemProfile GetDefaultProfile(const std::string& itemTypeName);
    
    /**
     * @brief Get all profiles for an item type
     * @param itemTypeName Item type name
     * @return Vector of profiles for the type
     */
    static std::vector<ItemProfile> GetProfilesForType(const std::string& itemTypeName);
    
    /**
     * @brief Get all available profiles
     * @return Map of profile ID to profile
     */
    static std::map<std::string, ItemProfile> GetAllProfiles();
    
    /**
     * @brief Check if a profile exists
     * @param profileId Profile ID
     * @return True if profile exists
     */
    static bool ProfileExists(const std::string& profileId);
    
    /**
     * @brief Delete a profile
     * @param profileId Profile ID
     * @return True if deletion succeeded
     */
    static bool DeleteProfile(const std::string& profileId);
    
    /**
     * @brief Validate a profile structure
     * @param profile Profile to validate
     * @param errors Output vector of error messages
     * @return True if profile is valid
     */
    static bool ValidateProfile(const ItemProfile& profile, std::vector<std::string>& errors);
    
    /**
     * @brief Get profiles directory
     * @return Profiles directory path
     */
    static std::string GetProfilesDirectory();
    
    /**
     * @brief Create default profiles for built-in item types
     * @param profilesDir Directory to create profiles in
     * @return True if creation succeeded
     */
    static bool CreateDefaultProfiles(const std::string& profilesDir);

private:
    /**
     * @brief Helper to create base field (id, displayName, etc.)
     */
    static ProfileField CreateBaseField(const std::string& name, const std::string& displayName,
        const std::string& description, const std::string& category, int order, bool required,
        const std::vector<std::string>& allowedValues = {},
        int defaultValue = 0, int minValue = 0, int maxValue = 0);
    
    /**
     * @brief Helper to create field with validation
     */
    static ProfileField CreateField(const std::string& name, ProfileFieldType type,
        const std::string& displayName, const std::string& description, const std::string& category,
        int order, bool required, double minValue = 0.0, double maxValue = 0.0);
    static std::string s_profilesDir;
    
    /**
     * @brief Parse profile from JSON
     * @param json JSON object
     * @return Parsed profile
     */
    static ItemProfile ParseProfileFromJson(const nlohmann::json& json);
    
    /**
     * @brief Convert profile to JSON
     * @param profile Profile to convert
     * @return JSON object
     */
    static nlohmann::json ProfileToJson(const ItemProfile& profile);
    
    /**
     * @brief Parse field from JSON
     * @param json JSON object
     * @return Parsed field
     */
    static ProfileField ParseFieldFromJson(const nlohmann::json& json);
    
    /**
     * @brief Convert field to JSON
     * @param field Field to convert
     * @return JSON object
     */
    static nlohmann::json FieldToJson(const ProfileField& field);
    
    /**
     * @brief Parse validation from JSON
     * @param json JSON object
     * @return Parsed validation
     */
    static ProfileFieldValidation ParseValidationFromJson(const nlohmann::json& json);
    
    /**
     * @brief Convert validation to JSON
     * @param validation Validation to convert
     * @return JSON object
     */
    static nlohmann::json ValidationToJson(const ProfileFieldValidation& validation);
    
    /**
     * @brief Get profile file path
     * @param profileId Profile ID
     * @return Full file path
     */
    static std::string GetProfileFilePath(const std::string& profileId);
    
    /**
     * @brief Ensure id and displayName fields are always present at the top
     * @param profile Profile to modify
     */
    static void EnsureRequiredFields(ItemProfile& profile);
};
