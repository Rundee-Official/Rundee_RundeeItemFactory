/**
 * @file PlayerProfileManager.cpp
 * @brief Implementation of player profile manager
 * @author Haneul Lee (Rundee)
 * @date 2025-12-23
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Data/PlayerProfileManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <json.hpp>

using nlohmann::json;

namespace PlayerProfileManager
{
    PlayerProfile LoadProfileFromPath(const std::string& filePath)
    {
        PlayerProfile profile;
        
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
            json j;
            file >> j;
            
            profile.id = j.value("id", "");
            profile.displayName = j.value("displayName", "");
            profile.description = j.value("description", "");
            profile.version = j.value("version", 1);
            profile.isDefault = j.value("isDefault", false);
            
            // Load playerSettings
            if (j.contains("playerSettings") && j["playerSettings"].is_object())
            {
                const auto& ps = j["playerSettings"];
                profile.playerSettings.maxHunger = ps.value("maxHunger", 100);
                profile.playerSettings.maxThirst = ps.value("maxThirst", 100);
                profile.playerSettings.maxHealth = ps.value("maxHealth", 100);
                profile.playerSettings.maxStamina = ps.value("maxStamina", 100);
                profile.playerSettings.maxWeight = ps.value("maxWeight", 50000);
                profile.playerSettings.maxEnergy = ps.value("maxEnergy", 100);
            }
            
            // Load statSections
            if (j.contains("statSections") && j["statSections"].is_array())
            {
                for (const auto& sectionJson : j["statSections"])
                {
                    PlayerStatSection section;
                    section.name = sectionJson.value("name", "");
                    section.displayName = sectionJson.value("displayName", "");
                    section.description = sectionJson.value("description", "");
                    section.displayOrder = sectionJson.value("displayOrder", 0);
                    
                    if (sectionJson.contains("fields") && sectionJson["fields"].is_array())
                    {
                        for (const auto& fieldJson : sectionJson["fields"])
                        {
                            PlayerStatField field;
                            field.name = fieldJson.value("name", "");
                            field.displayName = fieldJson.value("displayName", "");
                            field.description = fieldJson.value("description", "");
                            field.value = fieldJson.value("value", 0);
                            field.displayOrder = fieldJson.value("displayOrder", 0);
                            
                            section.fields.push_back(field);
                        }
                    }
                    
                    profile.statSections.push_back(section);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[PlayerProfileManager] Failed to parse profile: " << e.what() << "\n";
        }
        
        return profile;
    }
    
    PlayerProfile LoadProfile(const std::string& profileId, const std::string& profilesDir)
    {
        if (profileId.empty() || profilesDir.empty())
        {
            return PlayerProfile();
        }
        
        std::filesystem::path profilePath(profilesDir);
        profilePath /= (profileId + ".json");
        
        return LoadProfileFromPath(profilePath.string());
    }
    
    PlayerProfile GetDefaultProfile(const std::string& profilesDir)
    {
        if (!std::filesystem::exists(profilesDir) || !std::filesystem::is_directory(profilesDir))
        {
            return PlayerProfile();
        }
        
        // Look for profile with isDefault = true
        for (const auto& entry : std::filesystem::directory_iterator(profilesDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                PlayerProfile profile = LoadProfileFromPath(entry.path().string());
                if (!profile.id.empty() && profile.isDefault)
                {
                    return profile;
                }
            }
        }
        
        // If no default found, return first profile
        for (const auto& entry : std::filesystem::directory_iterator(profilesDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                PlayerProfile profile = LoadProfileFromPath(entry.path().string());
                if (!profile.id.empty())
                {
                    return profile;
                }
            }
        }
        
        return PlayerProfile();
    }
}
