// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Implementation of validation and balancing for ItemFoodData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "FoodItemValidator.h"
#include <algorithm>
#include <iostream>

static int ClampInt(int v, int minV, int maxV)
{
    return std::max(minV, std::min(maxV, v));
}

static float ComputePower(const ItemFoodData& item)
{
    return static_cast<float>(
        item.hungerRestore +
        item.thirstRestore +
        item.healthRestore);
}

static void EnsureCategoryShape(ItemFoodData& item)
{
    // category가 비어 있으면 기본값: Food
    if (item.category.empty())
        item.category = "Food";

    // 일관성을 위해 소문자로 정규화해도 됨 (원하면)
    // 여기서는 displayName은 그대로 두고 category만 대소문자 고정한다고 가정
    if (item.category == "drink" || item.category == "DRINK")
        item.category = "Drink";
    else if (item.category == "food" || item.category == "FOOD")
        item.category = "Food";

    if (item.category == "Drink")
    {
        // 음료는 갈증 회복이 기본적으로 더 크거나 최소 5 이상
        if (item.thirstRestore < 5)
            item.thirstRestore = 5;

        if (item.thirstRestore < item.hungerRestore)
        {
            item.thirstRestore = item.hungerRestore + 5;
        }
    }
    else if (item.category == "Food")
    {
        // 음식은 배고픔 회복 우선
        if (item.hungerRestore < 5)
            item.hungerRestore = 5;

        if (item.hungerRestore < item.thirstRestore)
        {
            item.hungerRestore = item.thirstRestore + 5;
        }
    }
    // 그 외 카테고리는 아직 특별 처리 X
}

static void EnsureSpoilage(ItemFoodData& item)
{
    // spoils == false 면 유통기한 0
    if (!item.spoils)
    {
        item.spoilTimeMinutes = 0;
        return;
    }

    // spoils == true 면 최소/최대 범위를 갖게 한다 (5분 ~ 7일)
    int minMinutes = 5;
    int maxMinutes = 7 * 24 * 60; // 7 days
    item.spoilTimeMinutes = ClampInt(item.spoilTimeMinutes, minMinutes, maxMinutes);
}

static void EnsureRarity(ItemFoodData& item)
{
    float power = ComputePower(item);

    // rarity가 비어있거나 이상한 값이면 power 기준으로 설정
    if (item.rarity.empty() ||
        (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare"))
    {
        if (power <= 25.0f)      item.rarity = "Common";
        else if (power <= 55.0f) item.rarity = "Uncommon";
        else                     item.rarity = "Rare";
        return;
    }

    // rarity별 대략적인 파워 상한
    float maxPower = 999.0f;
    if (item.rarity == "Common")
    {
        maxPower = 40.0f;
    }
    else if (item.rarity == "Uncommon")
    {
        maxPower = 65.0f;
    }
    else if (item.rarity == "Rare")
    {
        maxPower = 100.0f;
    }

    if (power <= maxPower)
        return; // 이미 범위 안에 있음

    // power가 너무 크면, 비율 유지하면서 스케일 다운
    float scale = maxPower / power;
    if (scale <= 0.0f)
        return;

    auto scaleAndClamp = [&](int& v)
        {
            int scaled = static_cast<int>(v * scale + 0.5f);
            v = ClampInt(scaled, 0, 100);
        };

    scaleAndClamp(item.hungerRestore);
    scaleAndClamp(item.thirstRestore);
    scaleAndClamp(item.healthRestore);
}

void FoodItemValidator::Validate(ItemFoodData& item)
{
    // 1) 기본 범위 클램프
    item.hungerRestore = ClampInt(item.hungerRestore, 0, 100);
    item.thirstRestore = ClampInt(item.thirstRestore, 0, 100);
    item.healthRestore = ClampInt(item.healthRestore, 0, 100);

    item.maxStack = ClampInt(item.maxStack, 1, 999);

    // 2) 카테고리 형태 정리 (Food / Drink)
    EnsureCategoryShape(item);

    // 3) 부패 관련 처리
    EnsureSpoilage(item);

    // 4) rarity 관련 처리
    EnsureRarity(item);

    // (원하면 여기에서 로그 찍어서 디버깅 가능)
    // std::cout << "[FoodItemValidator] Validated item: " << item.id << "\n";
}
