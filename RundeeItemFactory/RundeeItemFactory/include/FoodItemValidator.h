// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodItemValidator.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Validation and balancing helpers for ItemFoodData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "ItemFoodData.h"

class FoodItemValidator
{
public:
    // 게임에 사용하기 전에 아이템 한 개를 검증/보정한다.
    // - 값 범위 클램프
    // - 카테고리(Food/Drink)에 맞는 기본 형태 보정
    // - rarity가 비어있으면 power 기반으로 설정
    // - rarity에 따라 power를 대략적인 목표 범위 안으로 클램프
    static void Validate(ItemFoodData& item);
};
