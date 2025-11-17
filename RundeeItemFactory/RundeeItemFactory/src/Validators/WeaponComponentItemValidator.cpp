// ===============================
// Project Name: RundeeItemFactory
// File Name: WeaponComponentItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation for ItemWeaponComponentData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/WeaponComponentItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    std::string ToLowerCopy(const std::string& value)
    {
        std::string lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return lower;
    }

    bool MatchesType(const std::string& typeLower, std::initializer_list<const char*> names)
    {
        for (const char* name : names)
        {
            if (typeLower == name)
            {
                return true;
            }
        }
        return false;
    }

    void ApplyRealismProfile(ItemWeaponComponentData& item)
    {
        const std::string typeLower = ToLowerCopy(item.componentType);

        auto zeroBallisticBonuses = [&]()
        {
            item.damageModifier = 0;
            item.penetrationModifier = 0;
            item.muzzleVelocityModifier = 0;
        };

        auto zeroRangeBonuses = [&]()
        {
            item.effectiveRangeModifier = 0;
        };

        if (typeLower.empty())
        {
            return;
        }

        if (MatchesType(typeLower, { "scope", "sight" }))
        {
            zeroBallisticBonuses();
            item.recoilModifier = 0;
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 5, 25);
            item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, 10, 150);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -15, 5);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 30, 800);
        }
        else if (MatchesType(typeLower, { "muzzle" }))
        {
            item.damageModifier = JsonUtils::ClampInt(item.damageModifier, -5, 3);
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 5, 25);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -10, 5);
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, -3, 8);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 50, 400);
            item.muzzleVelocityModifier = JsonUtils::ClampInt(item.muzzleVelocityModifier, -20, 10);
            item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, 0, 15);
            item.penetrationModifier = JsonUtils::ClampInt(item.penetrationModifier, 0, 5);
        }
        else if (MatchesType(typeLower, { "grip", "foregrip", "pistolgrip" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 10);
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 5, 20);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, 5, 20);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 20, 250);
        }
        else if (MatchesType(typeLower, { "stock" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 5, 20);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -5, 10);
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 10);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 150, 700);
        }
        else if (MatchesType(typeLower, { "barrel" }))
        {
            item.damageModifier = JsonUtils::ClampInt(item.damageModifier, -2, 8);
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, -5, 10);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -15, 5);
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 15);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 150, 900);
            item.muzzleVelocityModifier = JsonUtils::ClampInt(item.muzzleVelocityModifier, -50, 120);
            item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, 0, 150);
            item.penetrationModifier = JsonUtils::ClampInt(item.penetrationModifier, 0, 10);
        }
        else if (MatchesType(typeLower, { "handguard" }))
        {
            zeroBallisticBonuses();
            item.damageModifier = 0;
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 0, 10);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -5, 15);
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 10);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 80, 400);
            item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, 0, 20);
        }
        else if (MatchesType(typeLower, { "rail" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 0, 5);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -5, 5);
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 8);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 50, 300);
        }
        else if (MatchesType(typeLower, { "flashlight" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.recoilModifier = 0;
            item.accuracyModifier = 0;
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -5, 3);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 40, 250);
        }
        else if (MatchesType(typeLower, { "laser", "tacticaldevice" }))
        {
            zeroBallisticBonuses();
            item.damageModifier = 0;
            item.recoilModifier = 0;
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 3, 12);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -5, 5);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 30, 250);
            item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, 0, 20);
        }
        else if (MatchesType(typeLower, { "magazine" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.accuracyModifier = 0;
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, -5, 5);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -10, 5);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 50, 800);
        }
        else if (MatchesType(typeLower, { "charginghandle" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.accuracyModifier = 0;
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 0, 8);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, 5, 15);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 20, 120);
        }
        else if (MatchesType(typeLower, { "gasblock" }))
        {
            item.damageModifier = 0;
            item.penetrationModifier = 0;
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 10);
            item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, 0, 10);
            item.muzzleVelocityModifier = JsonUtils::ClampInt(item.muzzleVelocityModifier, 0, 20);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 80, 300);
            item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, 0, 20);
        }
        else if (MatchesType(typeLower, { "mount", "adapter" }))
        {
            zeroBallisticBonuses();
            zeroRangeBonuses();
            item.damageModifier = 0;
            item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, 0, 5);
            item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -5, 5);
            item.weightModifier = JsonUtils::ClampInt(item.weightModifier, 30, 200);
        }
    }
}

namespace WeaponComponentItemValidator
{
    void Validate(ItemWeaponComponentData& item)
    {
        // Add prefix to ID
        if (!item.id.empty() && item.id.find("WeaponComponent_") != 0)
        {
            item.id = "WeaponComponent_" + item.id;
        }

        // Default category
        if (item.category.empty())
            item.category = "WeaponComponent";

        // Clamp stat modifiers (can be negative, but reasonable range)
        item.damageModifier = JsonUtils::ClampInt(item.damageModifier, -50, 50);
        item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, -50, 50);
        item.ergonomicsModifier = JsonUtils::ClampInt(item.ergonomicsModifier, -50, 50);
        item.accuracyModifier = JsonUtils::ClampInt(item.accuracyModifier, -50, 50);
        item.weightModifier = JsonUtils::ClampInt(item.weightModifier, -2000, 2000);
        item.muzzleVelocityModifier = JsonUtils::ClampInt(item.muzzleVelocityModifier, -200, 200);
        item.effectiveRangeModifier = JsonUtils::ClampInt(item.effectiveRangeModifier, -200, 200);
        item.penetrationModifier = JsonUtils::ClampInt(item.penetrationModifier, -50, 50);
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);
        
        // Magazine-specific validation
        auto isMagazine = [](const std::string& type)
        {
            if (type.empty())
                return false;
            std::string lower = type;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            return lower == "magazine";
        };

        if (isMagazine(item.componentType))
        {
            item.magazineCapacity = JsonUtils::ClampInt(item.magazineCapacity, 1, 200);
            if (item.caliber.empty())
            {
                item.caliber = "9mm"; // Default caliber
                std::cout << "[WeaponComponentItemValidator] Warning: Magazine " << item.id << " has empty caliber, using default.\n";
            }
            if (item.magazineType.empty())
            {
                item.magazineType = "Standard";
                std::cout << "[WeaponComponentItemValidator] Warning: Magazine " << item.id << " has empty magazineType, using default.\n";
            }

            // Validate loaded rounds information
            if (!item.loadedRounds.empty())
            {
                // Sort by orderIndex to ensure consistent order
                std::sort(item.loadedRounds.begin(), item.loadedRounds.end(),
                    [](const LoadedRoundSegment& a, const LoadedRoundSegment& b)
                    {
                        if (a.orderIndex == b.orderIndex)
                            return a.roundCount > b.roundCount; // larger groups first for same order
                        return a.orderIndex < b.orderIndex;
                    });

                int totalRounds = 0;
                for (auto& segment : item.loadedRounds)
                {
                    segment.orderIndex = JsonUtils::ClampInt(segment.orderIndex, 0, item.magazineCapacity);
                    segment.roundCount = JsonUtils::ClampInt(segment.roundCount, 1, item.magazineCapacity);
                    totalRounds += segment.roundCount;
                }

                if (totalRounds > item.magazineCapacity)
                {
                    std::cout << "[WeaponComponentItemValidator] Warning: Magazine " << item.id
                        << " loadedRounds exceed capacity. Trimming excess.\n";
                    int remaining = item.magazineCapacity;
                    for (auto& segment : item.loadedRounds)
                    {
                        if (remaining <= 0)
                        {
                            segment.roundCount = 0;
                            continue;
                        }

                        if (segment.roundCount > remaining)
                        {
                            segment.roundCount = remaining;
                        }
                        remaining -= segment.roundCount;
                    }

                    // Remove zero-count segments
                    item.loadedRounds.erase(
                        std::remove_if(item.loadedRounds.begin(), item.loadedRounds.end(),
                            [](const LoadedRoundSegment& seg) { return seg.roundCount <= 0; }),
                        item.loadedRounds.end());
                }

            }
        }
        else
        {
            // Non-magazine components cannot store rounds
            item.loadedRounds.clear();
        }

        ApplyRealismProfile(item);

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " weapon component.";
            std::cout << "[WeaponComponentItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}

