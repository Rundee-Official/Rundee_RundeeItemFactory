// ===============================
// Project Name: RundeeItemFactory
// File Name: ClothingItemValidator.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Validation and balancing helpers for ItemClothingData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

// Project Includes
#include "Data/ItemClothingData.h"

// ============================================================================
// SECTION 1: ClothingItemValidator Class
// ============================================================================

class ClothingItemValidator
{
public:
    // Validate and balance clothing item data
    static void Validate(ItemClothingData& item);
};

