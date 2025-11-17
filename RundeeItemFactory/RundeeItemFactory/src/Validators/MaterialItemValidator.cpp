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

namespace MaterialItemValidator
{
    void Validate(ItemMaterialData& item)
    {
        // Default category/materialType
        if (item.category.empty())
            item.category = "Material";

        if (item.materialType.empty())
            item.materialType = "Unknown";

        // rarity validation
        if (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare")
        {
            // value 기반으로 자동 분류
            if (item.value <= 25)      item.rarity = "Common";
            else if (item.value <= 60) item.rarity = "Uncommon";
            else                       item.rarity = "Rare";
        }

        // 수치 범위 클램핑
        item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 100);
        item.hardness = JsonUtils::ClampInt(item.hardness, 0, 100);
        item.flammability = JsonUtils::ClampInt(item.flammability, 0, 100);
        item.value = JsonUtils::ClampInt(item.value, 0, 100);
    }
}


