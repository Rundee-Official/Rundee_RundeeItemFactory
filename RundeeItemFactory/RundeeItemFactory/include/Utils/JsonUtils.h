// ===============================
// Project Name: RundeeItemFactory
// File Name: JsonUtils.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: JSON utility functions for safe parsing.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include "json.hpp"

using nlohmann::json;

namespace JsonUtils
{
    // Safe getters that return default values on error
    int GetIntSafe(const json& j, const char* key, int defaultValue = 0);
    bool GetBoolSafe(const json& j, const char* key, bool defaultValue = false);
    std::string GetStringSafe(const json& j, const char* key, const std::string& defaultValue = "");

    // Clamp integer value between min and max
    int ClampInt(int v, int minV, int maxV);
}












