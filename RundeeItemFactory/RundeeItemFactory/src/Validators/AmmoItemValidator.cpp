// ===============================
// Project Name: RundeeItemFactory
// File Name: AmmoItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation for ItemAmmoData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/AmmoItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <fstream>
#include <chrono>

namespace AmmoItemValidator
{
    void Validate(ItemAmmoData& item)
    {
        std::string originalId = item.id;

        // Normalize prefix: strip any leading ammo_ (any case, repeated), then add canonical "Ammo_"
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

            stripPrefix(item.id, "ammo_");
            item.id = "Ammo_" + item.id;
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
                dbg << R"({"sessionId":"debug-session","runId":"prefix-debug","hypothesisId":"H1","location":"AmmoItemValidator.cpp:Validate","message":"id prefix normalization","data":{"before":")"
                    << originalId << R"(","after":")" << item.id << R"("},"timestamp":)" << ts << "})" << "\n";
            }
        }
        // #endregion

        // Default category
        if (item.category.empty())
            item.category = "Ammo";

        // Clamp stat values
        item.damageBonus = JsonUtils::ClampInt(item.damageBonus, -50, 50);
        item.penetration = JsonUtils::ClampInt(item.penetration, 0, 100);
        item.accuracyBonus = JsonUtils::ClampInt(item.accuracyBonus, -50, 50);
        item.recoilModifier = JsonUtils::ClampInt(item.recoilModifier, -50, 50);
        item.value = JsonUtils::ClampInt(item.value, 0, 100);
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

        // Special property logic
        if (item.armorPiercing)
        {
            // Armor piercing ammo should have high penetration
            if (item.penetration < 50)
                item.penetration = 50;
        }

        if (item.hollowPoint)
        {
            // Hollow point has higher damage but lower penetration
            if (item.damageBonus < 5)
                item.damageBonus = 5;
            if (item.penetration > 30)
                item.penetration = 30;
        }

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " ammunition.";
            std::cout << "[AmmoItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}











