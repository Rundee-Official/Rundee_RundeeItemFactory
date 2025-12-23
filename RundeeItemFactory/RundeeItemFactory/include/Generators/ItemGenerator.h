/**
 * @file ItemGenerator.h
 * @brief High-level item generation interface
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#pragma once

#include "Helpers/CommandLineParser.h"

/**
 * @class ItemGenerator
 * @brief High-level item generation using LLM
 */
class ItemGenerator
{
public:
    /**
     * @brief Generate items using LLM
     * @param args Command line arguments
     * @return Exit code (0 = success)
     */
    static int GenerateWithLLM(const CommandLineArgs& args);
};
