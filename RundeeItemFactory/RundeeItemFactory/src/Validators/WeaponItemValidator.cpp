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

namespace WeaponItemValidator
{
    void Validate(ItemWeaponData& item)
    {
        // Add prefix to ID
        if (!item.id.empty() && item.id.find("Weapon_") != 0)
        {
            item.id = "Weapon_" + item.id;
        }

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

