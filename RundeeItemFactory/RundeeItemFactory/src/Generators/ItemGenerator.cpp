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
#include "Prompts/CustomPreset.h"
#include "Clients/OllamaClient.h"
#include "Data/ItemProfileManager.h"
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
#include <thread>
#include <future>
#include <mutex>
#include <functional>
#include <random>
#ifdef _WIN32
#include <direct.h>
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

int ItemGenerator::GenerateWithLLM(const CommandLineArgs& args)
{
    // Initialize profile manager
    if (!ItemProfileManager::Initialize("profiles/"))
    {
        std::cerr << "[ItemGenerator] Failed to initialize ItemProfileManager\n";
        return 1;
    }

    // Load profile
    ItemProfile profile;
    if (!args.profileId.empty())
    {
        profile = ItemProfileManager::LoadProfile(args.profileId);
        if (profile.id.empty())
        {
            std::cerr << "[ItemGenerator] Failed to load profile: " << args.profileId << "\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Loaded profile: " << profile.id << " (" << profile.displayName << ")\n";
    }
    else
    {
        // Use default profile based on item type
        std::string defaultProfileId = "default_" + CommandLineParser::GetItemTypeName(args.itemType);
        std::transform(defaultProfileId.begin(), defaultProfileId.end(), defaultProfileId.begin(), ::tolower);
        
        profile = ItemProfileManager::LoadProfile(defaultProfileId);
        if (profile.id.empty())
        {
            std::cerr << "[ItemGenerator] Failed to load default profile for item type: " << defaultProfileId << "\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Using default profile: " << profile.id << "\n";
    }

    // Load custom preset
    CustomPreset customPreset;
    if (!args.customPresetPath.empty())
    {
        if (!CustomPresetManager::LoadPresetFromFile(args.customPresetPath, customPreset))
        {
            std::cerr << "[ItemGenerator] Failed to load custom preset from: " << args.customPresetPath << "\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Loaded custom preset: " << customPreset.displayName << "\n";
    }
    else if (!args.presetName.empty())
    {
        if (!CustomPresetManager::CreatePresetFromName(args.presetName, customPreset))
        {
            std::cerr << "[ItemGenerator] Failed to create preset from name: " << args.presetName << "\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Using preset: " << customPreset.displayName << "\n";
    }
    else if (!profile.customContext.empty())
    {
        // Use profile's custom context as preset
        customPreset.id = profile.id + "_context";
        customPreset.displayName = profile.displayName + " Context";
        customPreset.flavorText = profile.customContext;
        std::cout << "[ItemGenerator] Using profile custom context as preset\n";
    }
    else
    {
        // Use default preset
        if (!CustomPresetManager::CreatePresetFromName("default", customPreset))
        {
            std::cerr << "[ItemGenerator] Failed to create default preset\n";
            return 1;
        }
        std::cout << "[ItemGenerator] Using default preset\n";
    }

    // Get existing IDs
    std::set<std::string> existingIds = DynamicItemJsonWriter::GetExistingIds(args.params.outputPath);
    std::cout << "[ItemGenerator] Found " << existingIds.size() << " existing items in " << args.params.outputPath << "\n";

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

    // Build prompt
    std::string prompt = DynamicPromptBuilder::BuildPromptFromProfile(
        profile,
        args.params,
        customPreset,
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
    if (!DynamicItemJsonParser::ParseItemsFromJsonText(response, profile, items))
    {
        std::cerr << "[ItemGenerator] Failed to parse LLM response\n";
        return 1;
    }

    std::cout << "[ItemGenerator] Parsed " << items.size() << " items from LLM response\n";

    // Filter out duplicates
    std::vector<nlohmann::json> newItems;
    for (const auto& item : items)
    {
        if (item.contains("id") && item["id"].is_string())
        {
            std::string id = item["id"].get<std::string>();
            if (existingIds.find(id) == existingIds.end())
            {
                newItems.push_back(item);
                existingIds.insert(id);
            }
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

    // Log registry event
    size_t beforeCount = existingIds.size();
    size_t afterCount = beforeCount + newItems.size();
    ItemGeneratorRegistry::LogRegistryEvent(profile.itemTypeName, beforeCount, static_cast<size_t>(newItems.size()), afterCount);

    return 0;
}






