// ===============================
// Project Name: RundeeItemFactory
// File Name: ArmorItemValidator.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Validation and balancing helpers for ItemArmorData.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

// Project Includes
#include "Data/ItemArmorData.h"

// ============================================================================
// SECTION 1: ArmorItemValidator Class
// ============================================================================

class ArmorItemValidator
{
public:
    // Validate and balance armor item data
    static void Validate(ItemArmorData& item);
};

