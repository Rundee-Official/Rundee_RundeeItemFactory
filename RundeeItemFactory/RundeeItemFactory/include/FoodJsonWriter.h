// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodJsonWriter.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Helper for writing food items to a JSON file.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include "ItemFoodData.h"

class FoodJsonWriter
{
public:
    // items를 JSON 배열로 outputPath에 저장.
    // 성공: true, 실패: false
    static bool WriteToFile(const std::vector<ItemFoodData>& items,
        const std::string& outputPath);
};