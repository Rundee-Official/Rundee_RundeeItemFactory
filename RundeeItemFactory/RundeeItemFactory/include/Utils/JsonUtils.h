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

// Standard Library Includes
#include <string>

// Third-Party Includes
#include "json.hpp"
using nlohmann::json;

// ============================================================================
// SECTION 1: JsonUtils Namespace
// ============================================================================

namespace JsonUtils
{
    // ========================================================================
    // Safe Getter Functions
    // ========================================================================
    
    // Safe getters that return default values on error
    int GetIntSafe(const json& j, const char* key, int defaultValue = 0);
    bool GetBoolSafe(const json& j, const char* key, bool defaultValue = false);
    std::string GetStringSafe(const json& j, const char* key, const std::string& defaultValue = "");
    
    // ========================================================================
    // Utility Functions
    // ========================================================================
    
    // Clamp integer value between min and max
    int ClampInt(int v, int minV, int maxV);
}












