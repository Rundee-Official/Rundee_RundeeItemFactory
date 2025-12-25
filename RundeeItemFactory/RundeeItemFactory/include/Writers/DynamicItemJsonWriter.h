/**
 * @file DynamicItemJsonWriter.h
 * @brief Dynamic JSON writer for profile-based items
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides methods to write dynamically parsed items (JSON objects) to JSON files.
 */

#pragma once

#include <vector>
#include <string>
#include <set>
#include <json.hpp>

/**
 * @class DynamicItemJsonWriter
 * @brief Static class for writing dynamic item JSON to files
 */
class DynamicItemJsonWriter
{
public:
    /**
     * @brief Write dynamic items (JSON objects) to JSON file
     * @param items Vector of JSON objects representing items
     * @param path Output file path
     * @param mergeWithExisting If true, merge with existing items in file (skip duplicates)
     * @return true on success, false on failure
     */
    static bool WriteItemsToFile(
        const std::vector<nlohmann::json>& items,
        const std::string& path,
        bool mergeWithExisting = true);
    
    /**
     * @brief Get existing item IDs from JSON file
     * @param path JSON file path
     * @return Set of existing item IDs
     */
    static std::set<std::string> GetExistingIds(const std::string& path);
    
    /**
     * @brief Merge items with existing file, skipping duplicates
     * @param newItems New items to add
     * @param path File path
     * @return true on success, false on failure
     */
    static bool MergeItemsWithFile(
        const std::vector<nlohmann::json>& newItems,
        const std::string& path);
};







