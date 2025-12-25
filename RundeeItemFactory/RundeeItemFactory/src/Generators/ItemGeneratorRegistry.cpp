/**
 * @file ItemGeneratorRegistry.cpp
 * @brief Implementation of registry management
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Generators/ItemGeneratorRegistry.h"
#include <fstream>
#include <filesystem>
#include <json.hpp>
#include <iostream>
#include <algorithm>

namespace
{
    const std::string kRegistryDir = "Registry";
    
    /**
     * @brief Ensure parent directory exists for a file path
     * @param filePath File path
     */
    void EnsureParentDir(const std::string& filePath)
    {
        std::filesystem::path path(filePath);
        if (path.has_parent_path())
        {
            std::filesystem::create_directories(path.parent_path());
        }
    }
    
    /**
     * @brief Get the file path for a registry file
     * @param typeName Item type name (e.g., "food", "drink")
     * @return Full path to the registry file
     */
    std::string GetRegistryPath(const std::string& typeName)
    {
        return kRegistryDir + "/id_registry_" + typeName + ".json";
    }
}

namespace ItemGeneratorRegistry
{
    std::set<std::string> LoadRegistryIds(const std::string& typeName)
    {
        std::set<std::string> ids;
        std::string path = GetRegistryPath(typeName);
        std::ifstream ifs(path);
        if (!ifs.is_open())
            return ids;
        try
        {
            nlohmann::json j;
            ifs >> j;
            if (j.is_object() && j.contains("ids") && j["ids"].is_array())
            {
                for (const auto& v : j["ids"])
                {
                    if (v.is_string())
                        ids.insert(v.get<std::string>());
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[Registry] Failed to parse registry (" << path << "): " << e.what() << "\n";
        }
        return ids;
    }
    
    bool SaveRegistryIds(const std::string& typeName, const std::set<std::string>& ids)
    {
        std::string path = GetRegistryPath(typeName);
        EnsureParentDir(path);
        nlohmann::json j;
        j["ids"] = ids;
        std::ofstream ofs(path);
        if (!ofs.is_open())
        {
            std::cerr << "[Registry] Failed to open registry for write: " << path << "\n";
            return false;
        }
        ofs << j.dump(2);
        return true;
    }
    
    void LogRegistryEvent(const std::string& typeName, size_t beforeCount, size_t addedCount, size_t afterCount)
    {
        // Registry event logging (currently disabled for release)
        // Can be re-enabled for debugging if needed
    }
}







