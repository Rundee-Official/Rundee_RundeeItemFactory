/**
 * @file PlayerProfileManager.h
 * @brief Player profile manager declarations
 * @author Haneul Lee (Rundee)
 * @date 2025-12-23
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides functions for loading and managing player profiles.
 */

#pragma once

#include "Data/PlayerProfile.h"
#include <string>

/**
 * @namespace PlayerProfileManager
 * @brief Manager for loading player profiles
 */
namespace PlayerProfileManager
{
    /**
     * @brief Load a player profile from file
     * @param profileId Profile ID (filename without .json extension)
     * @param profilesDir Directory containing player profile files
     * @return Loaded player profile (empty if not found)
     */
    PlayerProfile LoadProfile(const std::string& profileId, const std::string& profilesDir);
    
    /**
     * @brief Load a player profile from file path
     * @param filePath Full path to profile JSON file
     * @return Loaded player profile (empty if not found)
     */
    PlayerProfile LoadProfileFromPath(const std::string& filePath);
    
    /**
     * @brief Get default player profile
     * @param profilesDir Directory containing player profile files
     * @return Default player profile (or empty if none found)
     */
    PlayerProfile GetDefaultProfile(const std::string& profilesDir);
}
