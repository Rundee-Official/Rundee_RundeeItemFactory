/**
 * @file ItemGenerator.h
 * @brief High-level item generation interface
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * This header provides the main entry points for generating game items using LLM.
 * Supports single item type generation and batch generation of multiple item types.
 */

#pragma once

#include "Helpers/CommandLineParser.h"

/**
 * @namespace ItemGenerator
 * @brief Main namespace for item generation functionality
 * 
 * Provides high-level functions to generate game items (Food, Drink, Material, Weapon, WeaponComponent, Ammo)
 * using local LLM (Ollama) with quality checking and validation.
 */
namespace ItemGenerator
{
    /**
     * @brief Generate items of a single type using LLM
     * 
     * This function generates items of the specified type using the configured LLM model.
     * It handles prompt building, LLM communication, JSON parsing, validation, quality checking,
     * and file writing. Supports retry logic and quality-based regeneration.
     * 
     * @param args Command line arguments containing generation parameters:
     *   - itemType: Type of items to generate (Food, Drink, Material, Weapon, WeaponComponent, Ammo)
     *   - count: Number of items to generate
     *   - modelName: LLM model name (e.g., "llama3", "mistral")
     *   - preset: Preset type (Default, Forest, Desert, Coast, City) or custom preset
     *   - outputPath: Path to output JSON file
     *   - params: Item-specific generation parameters (maxHunger, maxThirst, etc.)
     * 
     * @return 0 on success, non-zero on failure
     * 
     * @note The function automatically merges new items with existing items in the output file,
     *       skipping duplicates based on item ID.
     * 
     */
    int GenerateWithLLM(const CommandLineArgs& args);
}

