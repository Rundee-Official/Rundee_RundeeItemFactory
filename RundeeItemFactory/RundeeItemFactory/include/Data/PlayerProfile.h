/**
 * @file PlayerProfile.h
 * @brief Player profile data structures
 * @author Haneul Lee (Rundee)
 * @date 2025-12-23
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines data structures for player profiles that describe player stat settings.
 */

#pragma once

#include <string>
#include <map>
#include <vector>

/**
 * @struct PlayerSettings
 * @brief Player stat maximum values
 */
struct PlayerSettings
{
    int maxHunger = 100;
    int maxThirst = 100;
    int maxHealth = 100;
    int maxStamina = 100;
    int maxWeight = 50000;
    int maxEnergy = 100;
};

/**
 * @struct PlayerStatField
 * @brief Player stat field definition
 */
struct PlayerStatField
{
    std::string name;
    std::string displayName;
    std::string description;
    int value = 0;
    int displayOrder = 0;
};

/**
 * @struct PlayerStatSection
 * @brief Player stat section definition
 */
struct PlayerStatSection
{
    std::string name;
    std::string displayName;
    std::string description;
    int displayOrder = 0;
    std::vector<PlayerStatField> fields;
};

/**
 * @struct PlayerProfile
 * @brief Complete player profile defining player stat settings
 */
struct PlayerProfile
{
    std::string id;
    std::string displayName;
    std::string description;
    int version = 1;
    bool isDefault = false;
    PlayerSettings playerSettings;
    std::vector<PlayerStatSection> statSections;
};
