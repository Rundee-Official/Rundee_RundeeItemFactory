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
     * @see GenerateBatch for generating multiple item types in one operation
     */
    int GenerateWithLLM(const CommandLineArgs& args);

    /**
     * @brief Generate multiple item types in batch mode
     * 
     * Generates items for multiple types sequentially. Each item type is processed independently
     * with its own output file (if specified) or default location.
     * 
     * @param args Command line arguments containing batch configuration:
     *   - batchItems: Vector of BatchItem structures, each specifying:
     *     * itemType: Type of items to generate
     *     * count: Number of items to generate for this type
     *     * outputPath: Optional custom output path (empty uses default)
     *   - modelName: LLM model name (shared across all batch items)
     *   - preset: Preset type (shared across all batch items)
     * 
     * @return 0 if all batch items succeeded, 1 if any failed
     * 
     * @note Batch items are processed sequentially, not in parallel.
     * @note Each batch item can have its own output file or use default location.
     * 
     * @see GenerateWithLLM for single item type generation
     */
    int GenerateBatch(const CommandLineArgs& args);
}

