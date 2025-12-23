/**
 * @file ItemGeneratorRegistry.h
 * @brief Registry management for item ID tracking
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides functions to load, save, and manage item ID registries.
 * Registries track all generated item IDs to prevent duplicates.
 */

#pragma once

#include <set>
#include <string>
#include <vector>

namespace ItemGeneratorRegistry
{
    /**
     * @brief Load item IDs from registry file
     * @param typeName Item type name (e.g., "Food", "Weapon")
     * @return Set of item IDs from registry
     */
    std::set<std::string> LoadRegistryIds(const std::string& typeName);
    
    /**
     * @brief Save item IDs to registry file
     * @param typeName Item type name
     * @param ids Set of item IDs to save
     * @return True on success, false on failure
     */
    bool SaveRegistryIds(const std::string& typeName, const std::set<std::string>& ids);
    
    /**
     * @brief Append item IDs to registry (loads existing, adds new, saves back)
     * @tparam T Item type with an 'id' member
     * @param typeName Item type name
     * @param items Vector of items to extract IDs from
     */
    template <typename T>
    void AppendIdsToRegistry(const std::string& typeName, const std::vector<T>& items);
    
    /**
     * @brief Log registry event (for debugging)
     * @param typeName Item type name
     * @param beforeCount Count before update
     * @param addedCount Number of IDs added
     * @param afterCount Count after update
     */
    void LogRegistryEvent(const std::string& typeName, size_t beforeCount, size_t addedCount, size_t afterCount);
}

// Template implementation
template <typename T>
void ItemGeneratorRegistry::AppendIdsToRegistry(const std::string& typeName, const std::vector<T>& items)
{
    if (items.empty())
        return;
    
    std::set<std::string> ids = LoadRegistryIds(typeName);
    size_t before = ids.size();
    for (const auto& item : items)
    {
        ids.insert(item.id);
    }
    size_t after = ids.size();
    size_t added = (after >= before) ? (after - before) : 0;
    if (SaveRegistryIds(typeName, ids))
    {
        LogRegistryEvent(typeName, before, added, after);
    }
}






