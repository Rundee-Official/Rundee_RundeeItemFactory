// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodJsonParser.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Implementation for parsing JSON text into ItemFoodData list.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "FoodJsonParser.h"
#include "FoodItemValidator.h"
#include <iostream>

using nlohmann::json;

// --------------------------------------------------------
// Helper: 문자열 앞뒤 공백 제거
// --------------------------------------------------------
static void TrimString(std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        str.clear();
        return;
    }
    str = str.substr(start, end - start + 1);
}

// --------------------------------------------------------
// Helper: LLM이 뱉은 JSON 배열 문자열을 "살짝 고쳐주는" 함수
// - 마지막에 쓸데없는 ',' 제거
// - '[' 개수 > ']' 개수면 부족한 만큼 ']'를 뒤에 붙여줌
// --------------------------------------------------------
static std::string CleanJsonArrayText(const std::string& input)
{
    std::string s = input;

    // 1) 양쪽 공백 제거
    TrimString(s);
    if (s.empty())
        return s;

    // 2) 마지막에 남아있는 콤마 제거
    //    예: [...},   \n,  \n  → 끝 콤마 제거
    {
        int idx = static_cast<int>(s.size()) - 1;
        // 뒤에서부터 공백 아닌 문자 찾기
        while (idx >= 0 && (s[idx] == ' ' || s[idx] == '\t' || s[idx] == '\r' || s[idx] == '\n'))
            --idx;

        if (idx >= 0 && s[idx] == ',')
        {
            s.erase(static_cast<size_t>(idx), 1);
        }
    }

    // 3) 배열 괄호 개수 맞춰주기
    int bracketBalance = 0;
    for (char c : s)
    {
        if (c == '[') bracketBalance++;
        else if (c == ']') bracketBalance--;
    }

    // '[' 가 더 많으면 부족한 ']'를 그만큼 뒤에 붙인다.
    while (bracketBalance > 0)
    {
        s += "\n]";
        bracketBalance--;
    }

    return s;
}

// --------------------------------------------------------
// Safe getters
// --------------------------------------------------------
static int GetIntSafe(const json& j, const char* key, int defaultValue = 0)
{
    if (!j.contains(key))
        return defaultValue;
    if (j[key].is_number_integer())
        return j[key].get<int>();
    if (j[key].is_number())
        return static_cast<int>(j[key].get<double>());
    return defaultValue;
}

static bool GetBoolSafe(const json& j, const char* key, bool defaultValue = false)
{
    if (!j.contains(key))
        return defaultValue;
    if (j[key].is_boolean())
        return j[key].get<bool>();
    if (j[key].is_number_integer())
        return j[key].get<int>() != 0;
    return defaultValue;
}

static std::string GetStringSafe(const json& j, const char* key, const std::string& defaultValue = "")
{
    if (!j.contains(key))
        return defaultValue;
    if (j[key].is_string())
        return j[key].get<std::string>();
    return defaultValue;
}

// --------------------------------------------------------
// Main parse function
// --------------------------------------------------------
bool FoodJsonParser::ParseFromJsonText(const std::string& jsonText,
    std::vector<ItemFoodData>& outItems)
{
    outItems.clear();

    // 0) 먼저 LLM이 뱉은 문자열을 한 번 정리해준다.
    std::string cleaned = CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[FoodJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[FoodJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[FoodJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemFoodData item;
        item.id = GetStringSafe(jItem, "id");
        item.displayName = GetStringSafe(jItem, "displayName");
        item.category = GetStringSafe(jItem, "category");
        item.rarity = GetStringSafe(jItem, "rarity");

        item.maxStack = GetIntSafe(jItem, "maxStack", 1);
        item.hungerRestore = GetIntSafe(jItem, "hungerRestore", 0);
        item.thirstRestore = GetIntSafe(jItem, "thirstRestore", 0);
        item.healthRestore = GetIntSafe(jItem, "healthRestore", 0);
        item.spoils = GetBoolSafe(jItem, "spoils", false);
        item.spoilTimeMinutes = GetIntSafe(jItem, "spoilTimeMinutes", 0);
        item.description = GetStringSafe(jItem, "description");

        // 최소한의 유효성 체크: id/displayName 없으면 스킵
        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[FoodJsonParser] Skipping item at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        // 밸런싱/검증 단계
        FoodItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[FoodJsonParser] Parsed " << outItems.size()
        << " items from JSON.\n";

    return !outItems.empty();
}