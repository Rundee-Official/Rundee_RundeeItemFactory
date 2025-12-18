// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemGenerator.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: High-level item generation helpers.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include <set>
#include "Helpers/ItemGenerateParams.h"
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMaterialData.h"

// Forward declarations
struct CommandLineArgs;
struct CustomPreset;

namespace ItemGenerator
{
    // Generate items using LLM (with automatic parallel batching for large requests)
    int GenerateWithLLM(const CommandLineArgs& args);

    // Generate multiple item types in batch mode (parallel)
    int GenerateBatch(const CommandLineArgs& args);
    
    // Internal: Generate single batch (used by parallel processing)
    int GenerateWithLLM_SingleBatch(const CommandLineArgs& args, const std::set<std::string>& existingIds, bool useCustomPreset, const CustomPreset& customPreset);
}

