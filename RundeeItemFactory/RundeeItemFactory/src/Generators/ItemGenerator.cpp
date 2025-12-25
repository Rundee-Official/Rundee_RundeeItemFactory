/**
 * @file ItemGenerator.cpp
 * @brief Implementation of high-level item generation using LLM (Ollama)
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * This file implements the core item generation logic for RundeeItemFactory.
 * It supports parallel batch processing, quality checking, duplicate detection,
 * and registry-based ID tracking across multiple item types.
 * 
 * Key Features:
 * - Parallel batch processing for efficient LLM interaction
 * - Thread-safe file I/O using per-file mutexes
 * - Quality checking and validation for generated items
 * - Registry-based duplicate ID prevention
 * - Support for all item types: Food, Drink, Material, Weapon, WeaponComponent, Ammo, Armor, Clothing
 */

#include "Generators/ItemGenerator.h"
#include "Generators/ItemGeneratorRegistry.h"
#include "Helpers/CommandLineParser.h"
#include "Parsers/DynamicItemJsonParser.h"
#include "Writers/DynamicItemJsonWriter.h"
#include "Prompts/DynamicPromptBuilder.h"
#include "Clients/OllamaClient.h"
#include "Data/ItemProfileManager.h"
#include "Data/PlayerProfileManager.h"
#include "Helpers/AppConfig.h"
#include <fstream>
#include <map>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <memory>
#include <chrono>
#include <map>
#include <algorithm>
#include <cctype>
#include <thread>
#include <future>
#include <mutex>
#include <functional>
#include <random>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#endif

/**
 * @namespace Anonymous namespace for internal helper functions and constants
 */
namespace
{
    /** @brief Banned words that should not appear in generated items */
    const std::vector<std::string> kBannedWords = { "dummy", "lorem", "ipsum", "placeholder", "test item", "badword" };
    
    /** @brief Directory name for ID registry files */
    const std::string kRegistryDir = "Registry";
    
    /** @brief Map of file paths to their corresponding mutexes for thread-safe I/O */
    static std::map<std::string, std::unique_ptr<std::mutex>> fileMutexes;
    
    /** @brief Mutex protecting the fileMutexes map itself */
    static std::mutex fileMutexesMutex;
    
    /**
     * @brief Get or create a mutex for a specific file path
     * @param filePath The file path to get a mutex for
     * @return Reference to the mutex for the given file path
     * @details Thread-safe: Uses fileMutexesMutex to protect the map access
     */
    std::mutex& GetFileMutex(const std::string& filePath)
    {
        std::lock_guard<std::mutex> lock(fileMutexesMutex);
        auto it = fileMutexes.find(filePath);
        if (it == fileMutexes.end())
        {
            fileMutexes[filePath] = std::make_unique<std::mutex>();
            return *fileMutexes[filePath];
        }
        return *it->second;
    }

    /**
     * @brief Convert a string to lowercase
     * @param s Input string
     * @return Lowercase version of the input string
     */
    std::string ToLower(const std::string& s)
    {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    }

    /**
     * @brief Count occurrences of banned words in text
     * @param text Text to search
     * @return Number of banned word hits found
     */
    int CountBanHits(const std::string& text)
    {
        int hits = 0;
        std::string low = ToLower(text);
        for (const auto& w : kBannedWords)
        {
            if (low.find(w) != std::string::npos)
                ++hits;
        }
        return hits;
    }

    /**
     * @brief Count occurrences of each rarity value
     * @param rarities Vector of rarity strings
     * @return Map of rarity name to count
     */
    std::map<std::string, int> CountRarity(const std::vector<std::string>& rarities)
    {
        std::map<std::string, int> counts;
        for (const auto& r : rarities)
            ++counts[r];
        return counts;
    }

    /**
     * @brief Print guardrail summary statistics
     * @param typeName Item type name
     * @param warnings Number of warnings
     * @param errors Number of errors
     * @param banHits Number of banned word hits
     * @param rarityCounts Map of rarity counts
     * @param total Total number of items
     */
    void PrintGuardrailSummary(const std::string& typeName, int warnings, int errors, int banHits, const std::map<std::string, int>& rarityCounts, int total)
    {
        std::cout << "[Guardrail] Type=" << typeName
            << " warnings=" << warnings
            << " errors=" << errors
            << " bannedHits=" << banHits;
        if (total > 0)
        {
            for (const auto& kv : rarityCounts)
            {
                double pct = (static_cast<double>(kv.second) / static_cast<double>(total)) * 100.0;
                std::cout << " rarity[" << kv.first << "]=" << kv.second << " (" << std::fixed << std::setprecision(1) << pct << "%)";
            }
        }
        std::cout << "\n";
    }
}

/**
 * @brief Ensure parent directory exists for a given file path
 * @param path File path whose parent directory should be created
 * @details Creates the parent directory if it doesn't exist (cross-platform)
 */
static void EnsureParentDir(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        std::string dir = path.substr(0, pos);
#ifdef _WIN32
        _mkdir(dir.c_str());
#else
        mkdir(dir.c_str(), 0755);
#endif
    }
}

int ItemGenerator::GenerateWithLLM(CommandLineArgs& args)
{
    // Get executable directory - this is the base for all paths
    std::string exeDir;
    
#ifdef _WIN32
    char exePath[MAX_PATH];
    DWORD pathLen = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (pathLen > 0 && pathLen < MAX_PATH)
    {
        exeDir = exePath;
        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash != std::string::npos)
        {
            exeDir = exeDir.substr(0, lastSlash + 1);
        }
    }
    else
    {
        exeDir = "";
    }
#else
    exeDir = "";
#endif
    
    // ItemProfiles directory - always use .exe directory/ItemProfiles
    std::string profilesDir = exeDir + "ItemProfiles";
    std::filesystem::path profilesPath(profilesDir);
    profilesDir = profilesPath.string();
    if (profilesDir.back() != '/' && profilesDir.back() != '\\')
    {
        profilesDir += "/";
    }
    
    // Initialize item profile manager
    if (!ItemProfileManager::Initialize(profilesDir))
    {
        std::cerr << "[ItemGenerator] Failed to initialize ItemProfileManager with directory: " << profilesDir << "\n";
        return 1;
    }
    
    std::cout << "[ItemGenerator] Using item profiles directory: " << profilesDir << "\n";

    // PlayerProfiles directory - always use .exe directory/PlayerProfiles
    std::string playerProfilesDir = exeDir + "PlayerProfiles";
    std::filesystem::path playerProfilesPath(playerProfilesDir);
    playerProfilesDir = playerProfilesPath.string();
    if (playerProfilesDir.back() != '/' && playerProfilesDir.back() != '\\')
    {
        playerProfilesDir += "/";
    }
    
    // Load player profile (REQUIRED)
    PlayerProfile playerProfile;
    bool playerProfileLoaded = false;
    
    if (!args.playerProfileId.empty())
    {
        playerProfile = PlayerProfileManager::LoadProfile(args.playerProfileId, playerProfilesDir);
        if (playerProfile.id.empty())
        {
            std::cerr << "[ItemGenerator] Error: Failed to load player profile: " << args.playerProfileId << "\n";
            std::cerr << "[ItemGenerator] Player profile is required. Please create a player profile in PlayerProfiles folder.\n";
            return 1;
        }
        playerProfileLoaded = true;
    }
    else if (!playerProfilesDir.empty())
    {
        // Try to load default player profile
        playerProfile = PlayerProfileManager::GetDefaultProfile(playerProfilesDir);
        if (!playerProfile.id.empty())
        {
            playerProfileLoaded = true;
        }
    }
    
    if (!playerProfileLoaded)
    {
        std::cerr << "[ItemGenerator] Error: No player profile found. Player profile is required.\n";
        std::cerr << "[ItemGenerator] Please create a player profile in: " << playerProfilesDir << "\n";
        return 1;
    }
    
    std::cout << "[ItemGenerator] Loaded player profile: " << playerProfile.id << " (" << playerProfile.displayName << ")\n";
    // Override params with player profile settings
    args.params.maxHunger = playerProfile.playerSettings.maxHunger;
    args.params.maxThirst = playerProfile.playerSettings.maxThirst;
    args.params.maxHealth = playerProfile.playerSettings.maxHealth;
    args.params.maxStamina = playerProfile.playerSettings.maxStamina;
    args.params.maxWeight = playerProfile.playerSettings.maxWeight;
    args.params.maxEnergy = playerProfile.playerSettings.maxEnergy;

    // Load item profile
    ItemProfile itemProfile;
    if (!args.profileId.empty())
    {
        itemProfile = ItemProfileManager::LoadProfile(args.profileId);
        if (itemProfile.id.empty())
        {
            std::cerr << "[ItemGenerator] Failed to load item profile: " << args.profileId << "\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Loaded item profile: " << itemProfile.id << " (" << itemProfile.displayName << ")\n";
    }
    else
    {
        // Use default item profile based on item type
        std::string defaultProfileId = "default_" + CommandLineParser::GetItemTypeName(args.itemType);
        std::transform(defaultProfileId.begin(), defaultProfileId.end(), defaultProfileId.begin(), ::tolower);
        
        itemProfile = ItemProfileManager::LoadProfile(defaultProfileId);
        if (itemProfile.id.empty())
        {
            std::cerr << "[ItemGenerator] Failed to load default item profile for item type: " << defaultProfileId << "\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Using default item profile: " << itemProfile.id << "\n";
    }

    // Use item profile's custom context for world context (Preset system removed)
    // World context is now managed through Item Profile's customContext field

    // Get existing IDs from both JSON file and registry
    std::set<std::string> existingIds = DynamicItemJsonWriter::GetExistingIds(args.params.outputPath);
    std::cout << "[ItemGenerator] Found " << existingIds.size() << " existing items in " << args.params.outputPath << "\n";
    
    // Load IDs from registry (persistent across all generations)
    std::string typeNameLower = itemProfile.itemTypeName;
    std::transform(typeNameLower.begin(), typeNameLower.end(), typeNameLower.begin(), ::tolower);
    std::set<std::string> registryIds = ItemGeneratorRegistry::LoadRegistryIds(typeNameLower);
    std::cout << "[ItemGenerator] Loaded " << registryIds.size() << " IDs from registry for type: " << typeNameLower << "\n";
    
    // Merge registry IDs with existing JSON IDs
    existingIds.insert(registryIds.begin(), registryIds.end());
    std::cout << "[ItemGenerator] Total unique IDs to avoid: " << existingIds.size() << "\n";

    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
#ifdef _WIN32
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
#else
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
#endif
    std::string generationTimestamp = ss.str();

    // Build prompt (world context is taken from itemProfile.customContext)
    std::string prompt = DynamicPromptBuilder::BuildPromptFromProfile(
        itemProfile,
        playerProfile,
        args.params,
        existingIds,
        args.modelName,
        generationTimestamp,
        static_cast<int>(existingIds.size()));

    // Call LLM
    std::cout << "[ItemGenerator] Calling LLM with model: " << args.modelName << "\n";
    std::string response = OllamaClient::RunWithRetry(args.modelName, prompt);
    if (response.empty())
    {
        std::cerr << "[ItemGenerator] LLM generation failed\n";
        return 1;
    }

    // Parse response
    std::vector<nlohmann::json> items;
    if (!DynamicItemJsonParser::ParseItemsFromJsonText(response, itemProfile, items))
    {
        std::cerr << "[ItemGenerator] Failed to parse LLM response\n";
        return 1;
    }

    std::cout << "[ItemGenerator] Parsed " << items.size() << " items from LLM response\n";

    // Filter out duplicates and ensure id/displayName are present
    std::vector<nlohmann::json> newItems;
    for (size_t i = 0; i < items.size(); ++i)
    {
        nlohmann::json item = items[i];
        
        // Ensure displayName is present first (needed for ID generation)
        if (!item.contains("displayName") || item["displayName"].is_null() ||
            (item["displayName"].is_string() && item["displayName"].get<std::string>().empty()))
        {
            // Generate displayName if missing
            item["displayName"] = itemProfile.itemTypeName + " Item " + std::to_string(i + 1);
        }
        
        // Ensure id is present and based on displayName
        if (!item.contains("id") || item["id"].is_null() || 
            (item["id"].is_string() && item["id"].get<std::string>().empty()))
        {
            // Generate id from displayName using the same logic as parser
            std::string itemTypePrefix = itemProfile.itemTypeName;
            std::transform(itemTypePrefix.begin(), itemTypePrefix.end(), itemTypePrefix.begin(), ::tolower);
            itemTypePrefix.erase(std::remove_if(itemTypePrefix.begin(), itemTypePrefix.end(),
                [](char c) { return !std::isalnum(c); }), itemTypePrefix.end());
            
            std::string displayName = item["displayName"].get<std::string>();
            std::string idSuffix = DynamicItemJsonParser::GenerateShortIdFromDisplayName(displayName);
            
            // Limit length
            if (idSuffix.length() > 30)
                idSuffix = idSuffix.substr(0, 30);
            
            if (idSuffix.empty())
                idSuffix = std::to_string(i + 1);
            
            std::ostringstream idStream;
            idStream << itemTypePrefix << "_" << idSuffix;
            item["id"] = idStream.str();
        }
        
        // Check for duplicates
        if (item.contains("id") && item["id"].is_string())
        {
            std::string id = item["id"].get<std::string>();
            if (existingIds.find(id) == existingIds.end())
            {
                newItems.push_back(item);
                existingIds.insert(id);
            }
        }
        else
        {
            // If id is still missing after generation, skip this item
            std::cerr << "[ItemGenerator] Warning: Item at index " << i << " has no valid id, skipping\n";
        }
    }

    std::cout << "[ItemGenerator] " << newItems.size() << " new items (after filtering duplicates)\n";

    if (newItems.empty())
    {
        std::cout << "[ItemGenerator] No new items to write\n";
        return 0;
    }

    // Ensure output directory exists
    EnsureParentDir(args.params.outputPath);

    // Write items to file
    if (!DynamicItemJsonWriter::WriteItemsToFile(newItems, args.params.outputPath, true))
    {
        std::cerr << "[ItemGenerator] Failed to write items to file\n";
        return 1;
    }

    std::cout << "[ItemGenerator] Successfully wrote " << newItems.size() << " items to " << args.params.outputPath << "\n";

    // Append new item IDs to registry (persistent storage)
    if (!newItems.empty())
    {
        // Extract IDs from new items and add to registry
        std::set<std::string> newIds;
        for (const auto& item : newItems)
        {
            if (item.contains("id") && item["id"].is_string())
            {
                newIds.insert(item["id"].get<std::string>());
            }
        }
        
        if (!newIds.empty())
        {
            // Load existing registry IDs
            std::set<std::string> registryIds = ItemGeneratorRegistry::LoadRegistryIds(typeNameLower);
            size_t beforeCount = registryIds.size();
            
            // Merge new IDs
            registryIds.insert(newIds.begin(), newIds.end());
            
            // Save back to registry
            if (ItemGeneratorRegistry::SaveRegistryIds(typeNameLower, registryIds))
            {
                size_t afterCount = registryIds.size();
                size_t addedCount = afterCount - beforeCount;
                std::cout << "[ItemGenerator] Added " << addedCount << " new IDs to registry (total: " << afterCount << ")\n";
                ItemGeneratorRegistry::LogRegistryEvent(itemProfile.itemTypeName, beforeCount, addedCount, afterCount);
            }
        }
    }

    return 0;
}
