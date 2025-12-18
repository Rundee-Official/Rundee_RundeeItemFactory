// ===============================
// Project Name: RundeeItemFactory
// File Name: MaterialItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation for ItemMaterialData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <chrono>
#include <fstream>
#include <iostream>

// Project Includes
#include "Validators/MaterialItemValidator.h"
#include "Utils/JsonUtils.h"

// ============================================================================
// SECTION 1: MaterialItemValidator Namespace
// ============================================================================

namespace MaterialItemValidator
{
    void Validate(ItemMaterialData& item)
    {
        std::string originalId = item.id;

        // Normalize prefix: strip any leading material_ (any case, repeated), then add canonical "Material_"
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

            stripPrefix(item.id, "material_");
            item.id = "Material_" + item.id;
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
                dbg << R"({"sessionId":"debug-session","runId":"prefix-debug","hypothesisId":"H1","location":"MaterialItemValidator.cpp:Validate","message":"id prefix normalization","data":{"before":")"
                    << originalId << R"(","after":")" << item.id << R"("},"timestamp":)" << ts << "})" << "\n";
            }
        }
        // #endregion

        // Default category/materialType
        if (item.category.empty())
            item.category = "Material";

        if (item.materialType.empty())
            item.materialType = "Unknown";

        // rarity validation
        if (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare")
        {
            // Auto-classify based on value
            if (item.value <= 25)      item.rarity = "Common";
            else if (item.value <= 60) item.rarity = "Uncommon";
            else                       item.rarity = "Rare";
        }

        // Clamp value ranges
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 100);
        item.hardness = JsonUtils::ClampInt(item.hardness, 0, 100);
        item.flammability = JsonUtils::ClampInt(item.flammability, 0, 100);
        item.value = JsonUtils::ClampInt(item.value, 0, 100);

        // Ensure description is not empty
        if (item.description.empty())
        {
            item.description = "A " + item.displayName + " used for crafting.";
            std::cout << "[MaterialItemValidator] Warning: Item " << item.id << " has empty description, using default.\n";
        }
    }
}


