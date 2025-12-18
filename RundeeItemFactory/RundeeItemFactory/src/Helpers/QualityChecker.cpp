// ===============================
// Project Name: RundeeItemFactory
// File Name: QualityChecker.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Implementation of quality and realism checking.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

// Project Includes
#include "Helpers/QualityChecker.h"
#include "Data/ItemAmmoData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemFoodData.h"
#include "Data/ItemMaterialData.h"
#include "Data/ItemWeaponComponentData.h"
#include "Data/ItemWeaponData.h"

// ============================================================================
// SECTION 1: Quality Check Functions (by Item Type)
// ============================================================================

namespace QualityChecker
{
    QualityResult CheckWeaponQuality(const ItemWeaponData& item)
    {
        QualityResult result;
        result.isValid = true;
        result.qualityScore = 100.0f;

        // Check for unrealistic damage values
        if (item.weaponCategory == "Ranged")
        {
            float avgDamage = (item.minDamage + item.maxDamage) / 2.0f;
            
            // Ranged weapons: base damage should be reasonable (ammo adds bonus)
            if (avgDamage > 80)
            {
                result.warnings.push_back("Very high base damage for ranged weapon (damage should come from ammo)");
                result.qualityScore -= 15.0f;
            }
            else if (avgDamage < 10 && item.rarity != "Common")
            {
                result.warnings.push_back("Low base damage for non-common weapon");
                result.qualityScore -= 10.0f;
            }

            // Check fire rate vs damage balance
            if (item.fireRate > 800 && avgDamage > 50)
            {
                result.warnings.push_back("High fire rate combined with high damage may be overpowered");
                result.qualityScore -= 10.0f;
            }

            // Check weight vs fire rate (heavy weapons usually have lower fire rate)
            if (item.weight > 4000 && item.fireRate > 600)
            {
                result.warnings.push_back("Heavy weapon with high fire rate may be unrealistic");
                result.qualityScore -= 5.0f;
            }

            // Check muzzle velocity vs caliber
            if (!item.caliber.empty())
            {
                if (item.caliber.find("9mm") != std::string::npos && item.muzzleVelocity > 500)
                {
                    result.warnings.push_back("9mm muzzle velocity seems high (typical: 300-400 m/s)");
                    result.qualityScore -= 5.0f;
                }
                else if (item.caliber.find("7.62") != std::string::npos && item.muzzleVelocity < 600)
                {
                    result.warnings.push_back("7.62mm muzzle velocity seems low (typical: 700-900 m/s)");
                    result.qualityScore -= 5.0f;
                }
            }

            // Check accuracy vs recoil balance
            if (item.accuracy > 80 && item.recoil > 50)
            {
                result.warnings.push_back("High accuracy with high recoil is contradictory");
                result.qualityScore -= 10.0f;
            }
        }
        else if (item.weaponCategory == "Melee")
        {
            float avgDamage = (item.minDamage + item.maxDamage) / 2.0f;

            // Melee weapons: damage should be inherent
            if (avgDamage > 70)
            {
                result.warnings.push_back("Very high damage for melee weapon");
                result.qualityScore -= 10.0f;
            }

            // Check attack speed vs damage balance
            if (item.attackSpeed > 5 && avgDamage > 40)
            {
                result.warnings.push_back("High attack speed with high damage may be overpowered");
                result.qualityScore -= 10.0f;
            }

            // Check reach vs weapon type
            if (item.weaponType.find("Knife") != std::string::npos && item.reach > 100)
            {
                result.warnings.push_back("Knife with long reach is unrealistic");
                result.qualityScore -= 10.0f;
            }
            else if (item.weaponType.find("Spear") != std::string::npos && item.reach < 200)
            {
                result.warnings.push_back("Spear with short reach is unrealistic");
                result.qualityScore -= 5.0f;
            }
        }

        // Check rarity vs stats correlation
        if (item.rarity == "Common" && (item.minDamage + item.maxDamage) / 2.0f > 50)
        {
            result.warnings.push_back("Common weapon with high damage may be unbalanced");
            result.qualityScore -= 5.0f;
        }
        else if (item.rarity == "Rare" && (item.minDamage + item.maxDamage) / 2.0f < 30)
        {
            result.warnings.push_back("Rare weapon with low damage may be underpowered");
            result.qualityScore -= 5.0f;
        }

        // Check weight realism
        if (item.weight < 100)
        {
            result.warnings.push_back("Weapon weight seems too light");
            result.qualityScore -= 5.0f;
        }
        else if (item.weight > 8000)
        {
            result.warnings.push_back("Weapon weight seems too heavy");
            result.qualityScore -= 5.0f;
        }

        // Clamp quality score
        result.qualityScore = std::max(0.0f, std::min(100.0f, result.qualityScore));

        return result;
    }

    QualityResult CheckWeaponComponentQuality(const ItemWeaponComponentData& item)
    {
        QualityResult result;
        result.isValid = true;
        result.qualityScore = 100.0f;

        // Check if all stat modifiers are zero (unrealistic and useless component)
        int nonZeroModifierCount = 0;
        if (item.damageModifier != 0) nonZeroModifierCount++;
        if (item.recoilModifier != 0) nonZeroModifierCount++;
        if (item.ergonomicsModifier != 0) nonZeroModifierCount++;
        if (item.accuracyModifier != 0) nonZeroModifierCount++;
        if (item.muzzleVelocityModifier != 0) nonZeroModifierCount++;
        if (item.effectiveRangeModifier != 0) nonZeroModifierCount++;
        if (item.penetrationModifier != 0) nonZeroModifierCount++;
        
        // Weight modifier is not a "stat" modifier, it's a physical property
        // But if weight is 0, that's also unrealistic
        
        if (nonZeroModifierCount == 0)
        {
            result.errors.push_back("Component has all stat modifiers at zero - unrealistic and useless");
            result.isValid = false;
            result.qualityScore -= 50.0f;
        }
        else if (nonZeroModifierCount == 1 && item.weightModifier == 0)
        {
            result.warnings.push_back("Component has only one stat modifier - should have at least 2-3 meaningful modifiers");
            result.qualityScore -= 20.0f;
        }
        else if (nonZeroModifierCount == 1)
        {
            result.warnings.push_back("Component has only one stat modifier - consider adding more realistic effects");
            result.qualityScore -= 10.0f;
        }

        // Check weight realism
        if (item.weightModifier == 0)
        {
            result.warnings.push_back("Component has zero weight - unrealistic (even small components weigh something)");
            result.qualityScore -= 15.0f;
        }
        else if (item.weightModifier < 20)
        {
            result.warnings.push_back("Component weight seems too light (minimum realistic weight is ~20g)");
            result.qualityScore -= 5.0f;
        }
        else if (item.weightModifier > 2000)
        {
            result.warnings.push_back("Component weight seems too heavy (maximum realistic weight is ~2000g)");
            result.qualityScore -= 10.0f;
        }

        // Check for unrealistic modifiers
        if (std::abs(item.damageModifier) > 30)
        {
            result.warnings.push_back("Very high damage modifier (most components don't affect damage)");
            result.qualityScore -= 15.0f;
        }

        // Check component type specific issues
        std::string typeLower = item.componentType;
        std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);

        if (typeLower.find("scope") != std::string::npos || typeLower.find("sight") != std::string::npos)
        {
            if (item.damageModifier != 0)
            {
                result.errors.push_back("Scopes/Sights cannot modify damage");
                result.isValid = false;
                result.qualityScore -= 30.0f;
            }
            if (item.recoilModifier != 0)
            {
                result.warnings.push_back("Scopes/Sights typically don't modify recoil");
                result.qualityScore -= 10.0f;
            }
        }

        if (typeLower.find("magazine") != std::string::npos)
        {
            if (item.damageModifier != 0)
            {
                result.warnings.push_back("Magazines should not modify damage");
                result.qualityScore -= 15.0f;
            }
            if (item.penetrationModifier != 0)
            {
                result.warnings.push_back("Magazines should not modify penetration");
                result.qualityScore -= 10.0f;
            }

            // Check magazine capacity
            if (item.magazineCapacity < 5)
            {
                result.warnings.push_back("Magazine capacity seems very low");
                result.qualityScore -= 5.0f;
            }
            else if (item.magazineCapacity > 150)
            {
                result.warnings.push_back("Magazine capacity seems very high");
                result.qualityScore -= 5.0f;
            }
        }

        // Check weight modifier realism
        if (item.weightModifier > 1000)
        {
            result.warnings.push_back("Very high weight modifier");
            result.qualityScore -= 10.0f;
        }

        // Check rarity vs modifier correlation
        if (item.rarity == "Common" && (std::abs(item.accuracyModifier) > 15 || std::abs(item.recoilModifier) > 15))
        {
            result.warnings.push_back("Common component with high modifiers may be unbalanced");
            result.qualityScore -= 5.0f;
        }

        // Clamp quality score
        result.qualityScore = std::max(0.0f, std::min(100.0f, result.qualityScore));

        return result;
    }

    QualityResult CheckAmmoQuality(const ItemAmmoData& item)
    {
        QualityResult result;
        result.isValid = true;
        result.qualityScore = 100.0f;

        // Check damage bonus vs caliber
        if (!item.caliber.empty())
        {
            if (item.caliber.find("9mm") != std::string::npos && item.damageBonus > 15)
            {
                result.warnings.push_back("9mm with high damage bonus may be unrealistic");
                result.qualityScore -= 10.0f;
            }
            else if (item.caliber.find("7.62") != std::string::npos && item.damageBonus < 5 && !item.armorPiercing)
            {
                result.warnings.push_back("7.62mm with low damage bonus may be underpowered");
                result.qualityScore -= 5.0f;
            }
        }

        // Check special properties consistency
        if (item.armorPiercing && item.penetration < 50)
        {
            result.warnings.push_back("Armor piercing ammo should have high penetration");
            result.qualityScore -= 10.0f;
        }

        if (item.hollowPoint && item.penetration > 30)
        {
            result.warnings.push_back("Hollow point ammo should have lower penetration");
            result.qualityScore -= 10.0f;
        }

        if (item.hollowPoint && item.damageBonus < 5)
        {
            result.warnings.push_back("Hollow point ammo should have higher damage bonus");
            result.qualityScore -= 5.0f;
        }

        // Check value vs stats correlation
        if (item.value < 5 && (item.damageBonus > 10 || item.penetration > 50))
        {
            result.warnings.push_back("High-performance ammo with low value may be unbalanced");
            result.qualityScore -= 10.0f;
        }

        // Check rarity vs stats (STRICT - Common + high stats is ERROR)
        if (item.rarity == "Common" && (item.damageBonus > 5 || item.penetration > 40))
        {
            result.errors.push_back("Common ammo with high stats is unbalanced - should be Uncommon or Rare");
            result.isValid = false;
            result.qualityScore -= 30.0f;
        }
        else if (item.rarity == "Uncommon" && (item.damageBonus > 15 || item.penetration > 70))
        {
            result.warnings.push_back("Uncommon ammo with very high stats should be Rare");
            result.qualityScore -= 10.0f;
        }

        // Check value vs performance (STRICT - high-performance + low value is ERROR)
        bool isHighPerformance = (item.damageBonus > 10 || item.penetration > 50);
        if (isHighPerformance && item.value < 10)
        {
            result.errors.push_back("High-performance ammo with low value is unbalanced - value should be at least 10");
            result.isValid = false;
            result.qualityScore -= 25.0f;
        }
        else if (isHighPerformance && item.value < 15)
        {
            result.warnings.push_back("High-performance ammo should have higher value (recommended: 15-30)");
            result.qualityScore -= 5.0f;
        }

        // Clamp quality score
        result.qualityScore = std::max(0.0f, std::min(100.0f, result.qualityScore));

        return result;
    }

    QualityResult CheckFoodQuality(const ItemFoodData& item)
    {
        QualityResult result;
        result.isValid = true;
        result.qualityScore = 100.0f;

        // Check total power
        int totalPower = item.hungerRestore + item.thirstRestore + item.healthRestore;
        if (totalPower > 100)
        {
            result.warnings.push_back("Very high total restore value");
            result.qualityScore -= 10.0f;
        }
        else if (totalPower < 5)
        {
            result.warnings.push_back("Very low total restore value");
            result.qualityScore -= 5.0f;
        }

        // Check food characteristics
        if (item.hungerRestore < item.thirstRestore)
        {
            result.warnings.push_back("Food should restore more hunger than thirst");
            result.qualityScore -= 5.0f;
        }

        // Check rarity vs power
        if (item.rarity == "Common" && totalPower > 50)
        {
            result.warnings.push_back("Common food with high restore value may be unbalanced");
            result.qualityScore -= 5.0f;
        }

        // Check spoilage consistency (aligned with prompt guidelines)
        if (item.spoils && item.spoilTimeMinutes < 60)
        {
            result.warnings.push_back("Very short spoil time is unrealistic (minimum 60 minutes)");
            result.qualityScore -= 10.0f;
        }
        else if (item.spoils && item.spoilTimeMinutes < 240 && item.rarity != "Common")
        {
            result.warnings.push_back("Non-common food with very short spoil time may be inconsistent");
            result.qualityScore -= 5.0f;
        }

        // Clamp quality score
        result.qualityScore = std::max(0.0f, std::min(100.0f, result.qualityScore));

        return result;
    }

    QualityResult CheckDrinkQuality(const ItemDrinkData& item)
    {
        QualityResult result;
        result.isValid = true;
        result.qualityScore = 100.0f;

        // Check drink characteristics
        if (item.thirstRestore < item.hungerRestore)
        {
            result.warnings.push_back("Drink should restore more thirst than hunger");
            result.qualityScore -= 5.0f;
        }

        if (item.thirstRestore < 10)
        {
            result.warnings.push_back("Drink with very low thirst restore");
            result.qualityScore -= 10.0f;
        }

        // Check total power (aligned with prompt guidelines)
        int totalPower = item.hungerRestore + item.thirstRestore + item.healthRestore;
        if (totalPower > 100)
        {
            result.warnings.push_back("Very high total restore value for drink (breaks game balance)");
            result.qualityScore -= 15.0f;
        }
        else if (totalPower > 60 && item.rarity == "Common")
        {
            result.warnings.push_back("Common drink with high total restore value may be unbalanced");
            result.qualityScore -= 10.0f;
        }
        else if (totalPower > 80 && item.rarity == "Uncommon")
        {
            result.warnings.push_back("Uncommon drink with very high total restore value should be Rare");
            result.qualityScore -= 5.0f;
        }

        // Clamp quality score
        result.qualityScore = std::max(0.0f, std::min(100.0f, result.qualityScore));

        return result;
    }

    QualityResult CheckMaterialQuality(const ItemMaterialData& item)
    {
        QualityResult result;
        result.isValid = true;
        result.qualityScore = 100.0f;

        // Check hardness vs flammability (usually inverse relationship)
        if (item.hardness > 80 && item.flammability > 70)
        {
            result.warnings.push_back("Hard materials are usually less flammable");
            result.qualityScore -= 5.0f;
        }

        // Check value vs properties
        if (item.value < 5 && (item.hardness > 70 || item.flammability < 20))
        {
            result.warnings.push_back("High-quality material with low value may be unbalanced");
            result.qualityScore -= 5.0f;
        }

        // Clamp quality score
        result.qualityScore = std::max(0.0f, std::min(100.0f, result.qualityScore));

        return result;
    }

    // ============================================================================
    // SECTION 2: Utility Functions
    // ============================================================================

    void PrintQualityResult(const QualityResult& result, const std::string& itemId)
    {
        if (result.errors.empty() && result.warnings.empty() && result.qualityScore >= 90.0f)
        {
            return; // Skip printing if quality is good
        }

        std::cout << "[QualityChecker] Item: " << itemId << " (Score: " 
            << std::fixed << std::setprecision(1) << result.qualityScore << "/100)\n";

        for (const auto& error : result.errors)
        {
            std::cout << "  [ERROR] " << error << "\n";
        }

        for (const auto& warning : result.warnings)
        {
            std::cout << "  [WARNING] " << warning << "\n";
        }
    }
}

