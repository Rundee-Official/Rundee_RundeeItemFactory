// ===============================
// Project Name: RundeeItemFactory
// File Name: BalanceReporter.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Balance report generator for generated items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include "Helpers/CommandLineParser.h"

namespace BalanceReporter
{
    // Generate balance report for a JSON file
    // Returns 0 on success, 1 on error
    int GenerateReport(const std::string& jsonFilePath, ItemType itemType);
}

