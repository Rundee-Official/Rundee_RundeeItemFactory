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
#include "Helpers/ItemGenerateParams.h"
#include "Data/ItemFoodData.h"
#include "Data/ItemDrinkData.h"
#include "Data/ItemMaterialData.h"

// Forward declaration
struct CommandLineArgs;

namespace ItemGenerator
{
    // Generate items using LLM
    int GenerateWithLLM(const CommandLineArgs& args);

    // Generate items using dummy data
    int GenerateDummy(const CommandLineArgs& args);
}

