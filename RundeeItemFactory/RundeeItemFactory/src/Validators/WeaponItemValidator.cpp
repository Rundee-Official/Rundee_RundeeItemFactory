// ===============================
// Project Name: RundeeItemFactory
// File Name: WeaponItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation and balancing for ItemWeaponData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/WeaponItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>

namespace WeaponItemValidator
{
    void Validate(ItemWeaponData& item)
    {
        std::string originalId = item.id;

        // Normalize prefix: strip any leading weapon_ (any case, repeated), then add canonical "Weapon_"
        if (!item.id.empty())
        {
            auto stripPrefix = [](std::string& value, const std::string& prefixLower)
            {
                std::string lower = value;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                while (lower.rfind(prefixLower, 0) == 0)
                {
                    value = value.substr(prefixLower.size());
                    lower = value;
                    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                }
            };

            stripPrefix(item.id, "weapon_");
            item.id = "Weapon_" + item.id;
        }

        // #region agent log
        static int dbgCount = 0;
        if (dbgCount < 50)
        {
            ++dbgCount;
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
            if (dbg.is_open())
            {
                dbg << R"({"sessionId":"debug-session","runId":"prefix-debug","hypothesisId":"H1","location":"WeaponItemValidator.cpp:Validate","message":"id prefix normalization","data":{"before":")"
                    << originalId << R"(","after":")" << item.id << R"("},"timestamp":)" << ts << "})" << "\n";
            }
        }
        // #endregion

        // Default category
        if (item.category.empty())
            item.category = "Weapon";

        // Default weaponCategory
        if (item.weaponCategory.empty())
        {
            // Try to infer from weaponType
            std::string lowerType = item.weaponType;
            std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
            if (lowerType.find("sword") != std::string::npos || 
                lowerType.find("axe") != std::string::npos ||
                lowerType.find("knife") != std::string::npos ||
                lowerType.find("mace") != std::string::npos ||
                lowerType.find("spear") != std::string::npos ||
                lowerType.find("club") != std::string::npos ||
                lowerType.find("hammer") != std::string::npos ||
                lowerType.find("blade") != std::string::npos ||
                lowerType.find("melee") != std::string::npos)
            {
                item.weaponCategory = "Melee";
            }
            else
            {
                item.weaponCategory = "Ranged";
            }
        }

        // Clamp stat values
        item.minDamage = JsonUtils::ClampInt(item.minDamage, 0, 100);
        item.maxDamage = JsonUtils::ClampInt(item.maxDamage, 0, 100);
        item.fireRate = JsonUtils::ClampInt(item.fireRate, 0, 1200);
        item.accuracy = JsonUtils::ClampInt(item.accuracy, 0, 100);
        item.recoil = JsonUtils::ClampInt(item.recoil, 0, 100);
        item.ergonomics = JsonUtils::ClampInt(item.ergonomics, 0, 100);
        item.durability = JsonUtils::ClampInt(item.durability, 0, 100);
        item.muzzleVelocity = JsonUtils::ClampInt(item.muzzleVelocity, 0, 1500);
        item.effectiveRange = JsonUtils::ClampInt(item.effectiveRange, 0, 1000);
        item.penetrationPower = JsonUtils::ClampInt(item.penetrationPower, 0, 100);
        item.moddingSlots = JsonUtils::ClampInt(item.moddingSlots, 0, 10);
        item.attackSpeed = JsonUtils::ClampInt(item.attackSpeed, 0, 10);
        item.reach = JsonUtils::ClampInt(item.reach, 0, 500); // 0-500cm (0-5 meters)
        item.staminaCost = JsonUtils::ClampInt(item.staminaCost, 0, 100);
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

        // Validate based on weaponCategory
        if (item.weaponCategory == "Melee")
        {
            // Melee weapons: no caliber, no ranged stats
            if (!item.caliber.empty())
            {
                item.caliber = ""; // Clear caliber for melee
            }
            item.muzzleVelocity = 0;
            item.effectiveRange = 0;
            item.penetrationPower = 0;
            item.recoil = 0;
            
            // Ensure melee stats are set
            if (item.attackSpeed == 0)
                item.attackSpeed = 2; // Default 2 attacks per second
            if (item.reach == 0)
                item.reach = 100; // Default 1 meter reach
        }
        else if (item.weaponCategory == "Ranged")
        {
            // Ranged weapons: must have caliber
            if (item.caliber.empty())
            {
                item.caliber = "9mm"; // Default caliber
                std::cout << "[WeaponItemValidator] Warning: Ranged weapon " << item.id << " has empty caliber, using default.\n";
            }
            
            // Melee-only stats should be 0
            item.attackSpeed = 0;
            item.reach = 0;
            item.staminaCost = 0;
        }

        // Normalize weight to avoid unrealistically light weapons
        if (item.weight < 1500)
        {
            item.weight = 1500;
        }

        // For ranged, give default muzzle velocity if missing
        if (item.weaponCategory == "Ranged" && item.muzzleVelocity == 0)
        {
            // Simple default based on type
            std::string lowType = item.weaponType;
            std::transform(lowType.begin(), lowType.end(), lowType.begin(), ::tolower);
            if (lowType.find("bow") != std::string::npos || lowType.find("crossbow") != std::string::npos)
                item.muzzleVelocity = 300;
            else
                item.muzzleVelocity = 400;
        }

        // Ensure maxDamage >= minDamage
        if (item.maxDamage < item.minDamage)
        {
            item.maxDamage = item.minDamage;
        }

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " weapon.";
            std::cout << "[WeaponItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}

