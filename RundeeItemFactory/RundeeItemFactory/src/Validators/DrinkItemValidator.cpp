// ===============================
// Project Name: RundeeItemFactory
// File Name: DrinkItemValidator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of validation and balancing for ItemDrinkData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Validators/DrinkItemValidator.h"
#include "Utils/JsonUtils.h"
#include <iostream>

static float ComputePower(const ItemDrinkData& item)
{
    return static_cast<float>(
        item.hungerRestore +
        item.thirstRestore +
        item.healthRestore);
}

static void EnsureDrinkShape(ItemDrinkData& item)
{
    // category 기본값: Drink
    if (item.category.empty())
        item.category = "Drink";

    // 소문자로 들어온 경우 대문자로 변환
    if (item.category == "drink" || item.category == "DRINK")
        item.category = "Drink";

    // Drink는 thirstRestore가 우선
    if (item.thirstRestore < 5)
        item.thirstRestore = 5;

    // Drink는 thirstRestore가 hungerRestore보다 커야 함
    if (item.thirstRestore < item.hungerRestore)
    {
        item.thirstRestore = item.hungerRestore + 5;
    }
}

static void EnsureSpoilage(ItemDrinkData& item)
{
    // spoils == false 면 spoilTimeMinutes는 0
    if (!item.spoils)
    {
        item.spoilTimeMinutes = 0;
        return;
    }

    // spoils == true 면 최소/최대 범위를 설정한다 (5분 ~ 7일)
    int minMinutes = 5;
    int maxMinutes = 7 * 24 * 60; // 7 days
    item.spoilTimeMinutes = JsonUtils::ClampInt(item.spoilTimeMinutes, minMinutes, maxMinutes);
}

static void EnsureRarity(ItemDrinkData& item)
{
    float power = ComputePower(item);

    // rarity가 비어있거나 잘못된 값이면 power 기반으로 자동 분류
    if (item.rarity.empty() ||
        (item.rarity != "Common" && item.rarity != "Uncommon" && item.rarity != "Rare"))
    {
        if (power <= 25.0f)      item.rarity = "Common";
        else if (power <= 55.0f) item.rarity = "Uncommon";
        else                     item.rarity = "Rare";
        return;
    }

    // rarity에 맞는 범위로 조정
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

    // power가 너무 크면, 비율을 유지하면서 범위로 맞춤
    float scale = maxPower / power;
    if (scale <= 0.0f)
        return;

    auto scaleAndClamp = [&](int& v)
        {
            int scaled = static_cast<int>(v * scale + 0.5f);
            v = JsonUtils::ClampInt(scaled, 0, 100);
        };

    scaleAndClamp(item.hungerRestore);
    scaleAndClamp(item.thirstRestore);
    scaleAndClamp(item.healthRestore);
}

void DrinkItemValidator::Validate(ItemDrinkData& item)
{
    // 1) 기본 수치 클램핑
    item.hungerRestore = JsonUtils::ClampInt(item.hungerRestore, 0, 100);
    item.thirstRestore = JsonUtils::ClampInt(item.thirstRestore, 0, 100);
    item.healthRestore = JsonUtils::ClampInt(item.healthRestore, 0, 100);

    item.maxStack = JsonUtils::ClampInt(item.maxStack, 1, 999);

    // 2) Drink 특성 보장 (thirstRestore 우선)
    EnsureDrinkShape(item);

    // 3) 부패 처리
    EnsureSpoilage(item);

    // 4) rarity 밸런스 처리
    EnsureRarity(item);
}


