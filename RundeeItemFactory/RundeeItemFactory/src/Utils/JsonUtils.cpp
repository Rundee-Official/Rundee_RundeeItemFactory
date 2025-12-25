// ===============================
// Project Name: RundeeItemFactory
// File Name: JsonUtils.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of JSON utility functions.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Utils/JsonUtils.h"
#include <algorithm>

namespace JsonUtils
{
    int GetIntSafe(const json& j, const char* key, int defaultValue)
    {
        if (!j.contains(key))
            return defaultValue;
        if (j[key].is_number_integer())
            return j[key].get<int>();
        if (j[key].is_number())
            return static_cast<int>(j[key].get<double>());
        return defaultValue;
    }

    bool GetBoolSafe(const json& j, const char* key, bool defaultValue)
    {
        if (!j.contains(key))
            return defaultValue;
        if (j[key].is_boolean())
            return j[key].get<bool>();
        if (j[key].is_number_integer())
            return j[key].get<int>() != 0;
        return defaultValue;
    }

    std::string GetStringSafe(const json& j, const char* key, const std::string& defaultValue)
    {
        if (!j.contains(key))
            return defaultValue;
        if (j[key].is_string())
            return j[key].get<std::string>();
        return defaultValue;
    }

    int ClampInt(int v, int minV, int maxV)
    {
        return std::max(minV, std::min(maxV, v));
    }
}
