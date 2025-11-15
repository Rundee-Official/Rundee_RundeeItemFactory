// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemFoodGenerator.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Helper class for generating food items (currently dummy data).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include "ItemFoodData.h"

// Parameters for food item generation (나중에 확장 가능)
struct FoodGenerateParams
{
    int count = 1;
    int maxHunger = 100;
    int maxThirst = 100;

    std::string outputPath = "items_food.json"; // 기본값
};

// 현재는 더미 데이터만 만드는 간단한 생성기
class ItemFoodGenerator
{
public:
    // TODO: 나중에 여기 안에서 Ollama를 호출해서 진짜 LLM 기반 아이템을 만들 예정.
    std::vector<ItemFoodData> GenerateDummy(const FoodGenerateParams& params);
};
