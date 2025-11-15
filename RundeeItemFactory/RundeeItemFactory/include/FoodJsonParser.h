// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodJsonParser.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Parser for converting JSON text to ItemFoodData list.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <vector>
#include <string>
#include "ItemFoodData.h"
#include "json.hpp"

class FoodJsonParser
{
public:
    // jsonText: LLM이 뱉은 JSON 문자열(배열)
    // outItems: 파싱된 아이템들이 들어감
    // 성공 시 true, 실패 시 false
    static bool ParseFromJsonText(const std::string& jsonText, std::vector<ItemFoodData>& outItems);
};