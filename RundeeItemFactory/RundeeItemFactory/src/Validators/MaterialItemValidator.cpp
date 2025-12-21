// ===============================
// Project Name: RundeeItemFactory
// File Name: MaterialItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation for ItemMaterialData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/MaterialItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>
#include <fstream>
#include <chrono>

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

        // Debug logging disabled for release

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


