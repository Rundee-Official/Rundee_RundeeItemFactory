/**
 * @file DynamicItemJsonWriter.cpp
 * @brief Implementation of dynamic JSON writer
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Writers/DynamicItemJsonWriter.h"
#include "Utils/StringUtils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

using nlohmann::json;

bool DynamicItemJsonWriter::WriteItemsToFile(
    const std::vector<nlohmann::json>& items,
    const std::string& path,
    bool mergeWithExisting)
{
    if (items.empty())
    {
        std::cerr << "[DynamicItemJsonWriter] Warning: No items to write.\n";
        return false;
    }
    
    // Ensure parent directory exists
    std::filesystem::path filePath(path);
    if (filePath.has_parent_path())
    {
        std::filesystem::create_directories(filePath.parent_path());
    }
    
    json outputArray = json::array();
    
    // If merging, load existing items first
    if (mergeWithExisting && std::filesystem::exists(path))
    {
        try
        {
            std::ifstream file(path);
            if (file.is_open())
            {
                json existingJson;
                file >> existingJson;
                file.close();
                
                if (existingJson.is_array())
                {
                    outputArray = existingJson;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[DynamicItemJsonWriter] Warning: Failed to read existing file: " << e.what() << "\n";
            // Continue with empty array
        }
    }
    
    // Collect existing IDs for duplicate checking
    std::set<std::string> existingIds;
    for (const auto& item : outputArray)
    {
        if (item.is_object() && item.contains("id") && item["id"].is_string())
        {
            existingIds.insert(item["id"].get<std::string>());
        }
    }
    
    // Add new items, skipping duplicates
    int addedCount = 0;
    for (const auto& item : items)
    {
        if (!item.is_object())
            continue;
        
        if (!item.contains("id") || !item["id"].is_string())
        {
            std::cerr << "[DynamicItemJsonWriter] Warning: Item missing 'id' field, skipping.\n";
            continue;
        }
        
        std::string itemId = item["id"].get<std::string>();
        if (existingIds.find(itemId) != existingIds.end())
        {
            continue; // Skip duplicate
        }
        
        outputArray.push_back(item);
        existingIds.insert(itemId);
        addedCount++;
    }
    
    // Write to file
    try
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            std::cerr << "[DynamicItemJsonWriter] Error: Failed to open file for writing: " << path << "\n";
            return false;
        }
        
        file << outputArray.dump(2); // Pretty print with 2 spaces
        file.close();
        
        std::cout << "[DynamicItemJsonWriter] Wrote " << addedCount << " new items to " << path 
                  << " (total: " << outputArray.size() << " items)\n";
        
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[DynamicItemJsonWriter] Error: Failed to write file: " << e.what() << "\n";
        return false;
    }
}

std::set<std::string> DynamicItemJsonWriter::GetExistingIds(const std::string& path)
{
    std::set<std::string> ids;
    
    if (!std::filesystem::exists(path))
    {
        return ids;
    }
    
    try
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return ids;
        }
        
        json jsonData;
        file >> jsonData;
        file.close();
        
        if (jsonData.is_array())
        {
            for (const auto& item : jsonData)
            {
                if (item.is_object() && item.contains("id") && item["id"].is_string())
                {
                    ids.insert(item["id"].get<std::string>());
                }
            }
        }
    }
    catch (const std::exception&)
    {
        // Return empty set on error
    }
    
    return ids;
}

bool DynamicItemJsonWriter::MergeItemsWithFile(
    const std::vector<nlohmann::json>& newItems,
    const std::string& path)
{
    return WriteItemsToFile(newItems, path, true);
}

