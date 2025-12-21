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
#include "Helpers/CommandLineParser.h"
#include "Helpers/QualityChecker.h"
#include "Parsers/ItemJsonParser.h"
#include "Writers/ItemJsonWriter.h"
#include "Prompts/PromptBuilder.h"
#include "Prompts/CustomPreset.h"
#include "Clients/OllamaClient.h"
#include "Validators/AmmoItemValidator.h"
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

    /**
     * @brief Fix value-related errors for Ammo items
     * @param items Vector of ammo items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes value errors and re-checks quality for affected items
     */
    void FixValueErrors_Ammo(std::vector<ItemAmmoData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            auto it = qualityResults.find(item.id);
            if (it != qualityResults.end() && !it->second.isValid)
            {
                bool hasValueError = false;
                bool hasCriticalError = false;
                
                for (const auto& err : it->second.errors)
                {
                    if (err.find("value") != std::string::npos || err.find("Value") != std::string::npos)
                    {
                        hasValueError = true;
                    }
                    else
                    {
                        hasCriticalError = true;
                    }
                }
                
                // If only value error, fix value directly
                if (hasValueError && !hasCriticalError)
                {
                    // Calculate appropriate value based on performance
                    bool isHighPerformance = (item.damageBonus > 10 || item.penetration > 50);
                    if (isHighPerformance)
                    {
                        // High-performance: value should be at least 10, preferably 15-30
                        int calculatedValue = std::max(10, std::min(50, 
                            static_cast<int>((std::abs(item.damageBonus) + item.penetration) / 5.0f)));
                        if (item.rarity == "Rare")
                        {
                            calculatedValue = std::max(15, calculatedValue);
                        }
                        item.value = calculatedValue;
                        std::cout << "[ItemGenerator] Fixed value for " << item.id << ": " << item.value << " (was low/0)\n";
                    }
                    else
                    {
                        // Normal performance: rarity-based value
                        if (item.rarity == "Common")
                            item.value = std::max(1, std::min(5, static_cast<int>((std::abs(item.damageBonus) + item.penetration) / 10.0f + 1)));
                        else if (item.rarity == "Uncommon")
                            item.value = std::max(5, std::min(15, static_cast<int>((std::abs(item.damageBonus) + item.penetration) / 8.0f + 5)));
                        else // Rare
                            item.value = std::max(15, std::min(50, static_cast<int>((std::abs(item.damageBonus) + item.penetration) / 5.0f + 15)));
                        std::cout << "[ItemGenerator] Fixed value for " << item.id << ": " << item.value << " (based on rarity and performance)\n";
                    }
                    
                    // Re-check quality after value fix
                    auto fixedQualityResult = QualityChecker::CheckAmmoQuality(item);
                    qualityResults[item.id] = fixedQualityResult;
                    if (fixedQualityResult.isValid)
                    {
                        std::cout << "[ItemGenerator] Item " << item.id << " is now valid after value fix.\n";
                    }
                }
            }
        }
    }

    /**
     * @brief Post-process Ammo items (clamp Common rarity stats, log items)
     * @param items Vector of ammo items to process (modified in-place)
     */
    void PostProcess_Ammo(std::vector<ItemAmmoData>& items)
    {
        for (auto& item : items)
        {
            if (item.rarity == "Common")
            {
                if (item.damageBonus > 15) item.damageBonus = 15;
                if (item.penetration > 25) item.penetration = 25;
            }
            std::cout << "- " << item.id << " / " << item.displayName
                << " / " << item.caliber << " / Damage: +" << item.damageBonus
                << " / Penetration: " << item.penetration << "\n";
        }
    }

    /**
     * @brief Fix WeaponComponent quality errors (scope/sight damageModifier)
     * @param items Vector of weapon component items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes scope/sight damageModifier != 0 errors
     */
    void FixWeaponComponentErrors(std::vector<ItemWeaponComponentData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            auto it = qualityResults.find(item.id);
            if (it != qualityResults.end() && !it->second.isValid)
            {
                bool needsFix = false;
                for (const auto& err : it->second.errors)
                {
                    if (err.find("Scopes/Sights cannot modify damage") != std::string::npos)
                    {
                        needsFix = true;
                        break;
                    }
                }
                
                if (needsFix)
                {
                    std::string typeLower = item.componentType;
                    std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
                    
                    if (typeLower.find("scope") != std::string::npos || typeLower.find("sight") != std::string::npos)
                    {
                        if (item.damageModifier != 0)
                        {
                            item.damageModifier = 0;
                            std::cout << "[ItemGenerator] Fixed scope/sight damageModifier for " << item.id << " (set to 0)\n";
                            
                            // Re-check quality after fix
                            auto fixedQualityResult = QualityChecker::CheckWeaponComponentQuality(item);
                            qualityResults[item.id] = fixedQualityResult;
                            if (fixedQualityResult.isValid)
                            {
                                std::cout << "[ItemGenerator] Item " << item.id << " is now valid after damageModifier fix.\n";
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Fix Material value warnings (value < 5 && high stats)
     * @param items Vector of material items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes value < 5 when hardness > 70 or flammability < 20
     */
    void FixMaterialValueWarnings(std::vector<ItemMaterialData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            if (item.value < 5 && (item.hardness > 70 || item.flammability < 20))
            {
                // High-quality material should have appropriate value
                int calculatedValue = 10;
                if (item.rarity == "Rare")
                {
                    calculatedValue = std::max(15, static_cast<int>((item.hardness + (100 - item.flammability)) / 10.0f));
                }
                else if (item.rarity == "Uncommon")
                {
                    calculatedValue = std::max(5, static_cast<int>((item.hardness + (100 - item.flammability)) / 15.0f));
                }
                else // Common
                {
                    calculatedValue = std::max(1, static_cast<int>((item.hardness + (100 - item.flammability)) / 20.0f));
                }
                item.value = std::min(100, calculatedValue); // Material value is clamped to 0-100
                std::cout << "[ItemGenerator] Fixed value for " << item.id << ": " << item.value << " (was too low for high-quality material)\n";
                
                // Re-check quality after fix
                auto fixedQualityResult = QualityChecker::CheckMaterialQuality(item);
                qualityResults[item.id] = fixedQualityResult;
            }
        }
    }

    /**
     * @brief Fix Food quality warnings (hungerRestore < thirstRestore)
     * @param items Vector of food items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes food characteristics: hungerRestore should be >= thirstRestore
     */
    void FixFoodQualityWarnings(std::vector<ItemFoodData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            bool needsFix = false;
            
            // Check if hungerRestore < thirstRestore
            if (item.hungerRestore < item.thirstRestore)
            {
                // Food should restore more hunger than thirst
                item.thirstRestore = std::max(0, item.hungerRestore - 1);
                needsFix = true;
                std::cout << "[ItemGenerator] Fixed food characteristic for " << item.id << ": hungerRestore=" << item.hungerRestore << ", thirstRestore=" << item.thirstRestore << "\n";
            }
            
            if (needsFix)
            {
                // Re-check quality after fix
                auto fixedQualityResult = QualityChecker::CheckFoodQuality(item);
                qualityResults[item.id] = fixedQualityResult;
            }
        }
    }

    /**
     * @brief Fix Drink quality warnings (thirstRestore < hungerRestore, thirstRestore < 10)
     * @param items Vector of drink items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes drink characteristics: thirstRestore should be >= hungerRestore and >= 10
     */
    void FixDrinkQualityWarnings(std::vector<ItemDrinkData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            bool needsFix = false;
            
            // Check if thirstRestore < hungerRestore
            if (item.thirstRestore < item.hungerRestore)
            {
                // Drink should restore more thirst than hunger
                item.hungerRestore = std::max(0, item.thirstRestore - 1);
                needsFix = true;
            }
            
            // Check if thirstRestore < 10
            if (item.thirstRestore < 10)
            {
                item.thirstRestore = 10;
                needsFix = true;
            }
            
            if (needsFix)
            {
                std::cout << "[ItemGenerator] Fixed drink characteristic for " << item.id << ": thirstRestore=" << item.thirstRestore << ", hungerRestore=" << item.hungerRestore << "\n";
                
                // Re-check quality after fix
                auto fixedQualityResult = QualityChecker::CheckDrinkQuality(item);
                qualityResults[item.id] = fixedQualityResult;
            }
        }
    }

    /**
     * @brief Fix Medicine quality warnings (healthRestore < 10, healthRestore < 20 && Rare)
     * @param items Vector of medicine items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes medicine characteristics: healthRestore should be >= 10, and >= 20 for Rare
     */
    void FixMedicineQualityWarnings(std::vector<ItemMedicineData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            bool needsFix = false;
            
            // Check if healthRestore < 10
            if (item.healthRestore < 10)
            {
                item.healthRestore = 10;
                needsFix = true;
            }
            
            // Check if Rare medicine has healthRestore < 20
            if (item.rarity == "Rare" && item.healthRestore < 20)
            {
                item.healthRestore = 20;
                needsFix = true;
            }
            
            if (needsFix)
            {
                std::cout << "[ItemGenerator] Fixed medicine characteristic for " << item.id << ": healthRestore=" << item.healthRestore << " (rarity: " << item.rarity << ")\n";
                
                // Re-check quality after fix
                auto fixedQualityResult = QualityChecker::CheckMedicineQuality(item);
                qualityResults[item.id] = fixedQualityResult;
            }
        }
    }

    /**
     * @brief Fix Weapon quality errors (Melee fireRate)
     * @param items Vector of weapon items to fix
     * @param qualityResults Map of quality check results (modified in-place)
     * @details Fixes Melee weapons with fireRate != 0
     */
    void FixWeaponErrors(std::vector<ItemWeaponData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
    {
        for (auto& item : items)
        {
            auto it = qualityResults.find(item.id);
            if (it != qualityResults.end() && !it->second.isValid)
            {
                bool needsFix = false;
                for (const auto& err : it->second.errors)
                {
                    if (err.find("Melee weapons cannot have fireRate") != std::string::npos)
                    {
                        needsFix = true;
                        break;
                    }
                }
                
                if (needsFix && item.weaponCategory == "Melee" && item.fireRate != 0)
                {
                    item.fireRate = 0;
                    std::cout << "[ItemGenerator] Fixed Melee weapon fireRate for " << item.id << " (set to 0)\n";
                    
                    // Re-check quality after fix
                    auto fixedQualityResult = QualityChecker::CheckWeaponQuality(item);
                    qualityResults[item.id] = fixedQualityResult;
                    if (fixedQualityResult.isValid)
                    {
                        std::cout << "[ItemGenerator] Item " << item.id << " is now valid after fireRate fix.\n";
                    }
                }
            }
            // Also fix proactively (even if not in qualityResults yet)
            else if (item.weaponCategory == "Melee" && item.fireRate != 0)
            {
                item.fireRate = 0;
                std::cout << "[ItemGenerator] Fixed Melee weapon fireRate for " << item.id << " (set to 0, proactive fix)\n";
            }
        }
    }

    /**
     * @brief Check and warn about duplicate/too-similar item names (Food/Clothing)
     * @param items Vector of items to check
     * @param itemTypeName Type name for logging
     * @details Warns about items with identical or very similar displayNames
     */
    template<typename ItemType>
    void CheckDuplicateNames(const std::vector<ItemType>& items, const std::string& itemTypeName)
    {
        std::map<std::string, std::vector<std::string>> nameToIds;
        for (const auto& item : items)
        {
            std::string nameLower = item.displayName;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            nameToIds[nameLower].push_back(item.id);
        }
        
        for (const auto& pair : nameToIds)
        {
            if (pair.second.size() > 1)
            {
                std::cout << "[ItemGenerator] WARNING: Duplicate displayName '" << pair.first 
                          << "' found in " << itemTypeName << " items: ";
                for (size_t i = 0; i < pair.second.size(); i++)
                {
                    if (i > 0) std::cout << ", ";
                    std::cout << pair.second[i];
                }
                std::cout << "\n";
            }
        }
        
        // Check for very similar names (simple heuristic: same first word or same last word)
        for (size_t i = 0; i < items.size(); i++)
        {
            for (size_t j = i + 1; j < items.size(); j++)
            {
                std::string name1 = items[i].displayName;
                std::string name2 = items[j].displayName;
                std::transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
                std::transform(name2.begin(), name2.end(), name2.begin(), ::tolower);
                
                // Extract first word
                size_t space1 = name1.find(' ');
                size_t space2 = name2.find(' ');
                if (space1 != std::string::npos && space2 != std::string::npos)
                {
                    std::string first1 = name1.substr(0, space1);
                    std::string first2 = name2.substr(0, space2);
                    if (first1 == first2 && first1.length() > 3) // Only warn if first word is meaningful
                    {
                        // Check if rest is very similar (e.g., "Wild Berry" vs "Wild Berries")
                        std::string rest1 = name1.substr(space1 + 1);
                        std::string rest2 = name2.substr(space2 + 1);
                        if (rest1 == rest2 || rest1 + "s" == rest2 || rest1 == rest2 + "s" || 
                            rest1 + "es" == rest2 || rest1 == rest2 + "es")
                        {
                            std::cout << "[ItemGenerator] WARNING: Very similar " << itemTypeName 
                                      << " names detected: '" << items[i].displayName 
                                      << "' (" << items[i].id << ") vs '" 
                                      << items[j].displayName << "' (" << items[j].id << ")\n";
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Ensure variety in Ammo items (accuracyBonus and special properties)
     * @param items Vector of ammo items to enhance (modified in-place)
     * @details Enhances items to ensure at least 20% have accuracyBonus and 20% have special properties
     */
    void EnsureVariety_Ammo(std::vector<ItemAmmoData>& items)
    {
        int itemsWithAccuracyBonus = 0;
        int itemsWithSpecialProperties = 0;
        for (const auto& item : items)
        {
            if (item.accuracyBonus != 0) itemsWithAccuracyBonus++;
            if (item.armorPiercing || item.hollowPoint || item.tracer || item.incendiary) itemsWithSpecialProperties++;
        }
        
        int totalItems = static_cast<int>(items.size());
        if (totalItems == 0) return;
        
        float accuracyBonusRatio = static_cast<float>(itemsWithAccuracyBonus) / totalItems;
        float specialPropertiesRatio = static_cast<float>(itemsWithSpecialProperties) / totalItems;
        
        // If variety is too low, enhance some items
        if (accuracyBonusRatio < 0.2f || specialPropertiesRatio < 0.2f)
        {
            std::cout << "[ItemGenerator] Low variety detected: accuracyBonus=" << accuracyBonusRatio 
                      << ", specialProperties=" << specialPropertiesRatio 
                      << ". Enhancing some items to ensure variety...\n";
            
            int enhancedCount = 0;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, totalItems - 1);
            
            // Enhance items to ensure at least 20% have accuracyBonus and 20% have special properties
            int targetAccuracyBonus = static_cast<int>(totalItems * 0.25f);
            int targetSpecialProperties = static_cast<int>(totalItems * 0.25f);
            
            // Add accuracyBonus to some items
            for (int i = itemsWithAccuracyBonus; i < targetAccuracyBonus && i < totalItems; i++)
            {
                int attempts = 0;
                while (attempts < 20)
                {
                    int idx = dis(gen);
                    if (items[idx].accuracyBonus == 0)
                    {
                        // Add positive or negative accuracyBonus based on rarity
                        if (items[idx].rarity == "Rare" || items[idx].rarity == "Uncommon")
                        {
                            items[idx].accuracyBonus = 5 + (idx % 10); // +5 to +14
                        }
                        else
                        {
                            items[idx].accuracyBonus = (idx % 2 == 0) ? (3 + (idx % 5)) : -(2 + (idx % 5)); // +3 to +7 or -2 to -6
                        }
                        items[idx].accuracyBonus = std::max(-15, std::min(15, items[idx].accuracyBonus));
                        enhancedCount++;
                        break;
                    }
                    attempts++;
                }
            }
            
            // Add special properties to some items
            for (int i = itemsWithSpecialProperties; i < targetSpecialProperties && i < totalItems; i++)
            {
                int attempts = 0;
                while (attempts < 20)
                {
                    int idx = dis(gen);
                    if (!items[idx].armorPiercing && !items[idx].hollowPoint && !items[idx].tracer && !items[idx].incendiary)
                    {
                        // Assign special property based on rarity and stats
                        if (items[idx].penetration >= 50 || items[idx].rarity == "Rare")
                        {
                            items[idx].armorPiercing = true;
                            if (items[idx].penetration < 50) items[idx].penetration = 50;
                        }
                        else if (items[idx].damageBonus >= 5 || items[idx].rarity == "Uncommon")
                        {
                            items[idx].hollowPoint = true;
                            if (items[idx].damageBonus < 5) items[idx].damageBonus = 5;
                            if (items[idx].penetration > 30) items[idx].penetration = 30;
                        }
                        else if (items[idx].rarity == "Uncommon" || items[idx].rarity == "Rare")
                        {
                            items[idx].tracer = true;
                        }
                        else
                        {
                            items[idx].tracer = true; // Default to tracer for common items
                        }
                        enhancedCount++;
                        break;
                    }
                    attempts++;
                }
            }
            
            if (enhancedCount > 0)
            {
                std::cout << "[ItemGenerator] Enhanced " << enhancedCount << " items to ensure variety.\n";
                // Re-validate enhanced items
                for (auto& item : items)
                {
                    AmmoItemValidator::Validate(item);
                }
            }
        }
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

// ============================================================================
// SECTION: ID Registry Management (Cross-Run Duplicate Prevention)
// ============================================================================

/**
 * @brief Get the file path for a registry file
 * @param typeName Item type name (e.g., "food", "drink")
 * @return Full path to the registry file
 */
static std::string GetRegistryPath(const std::string& typeName)
{
    return kRegistryDir + "/id_registry_" + typeName + ".json";
}

static std::set<std::string> LoadRegistryIds(const std::string& typeName)
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

/**
 * @brief Save IDs to registry file
 * @param typeName Item type name
 * @param ids Set of item IDs to save
 * @return true if saved successfully, false otherwise
 */
static bool SaveRegistryIds(const std::string& typeName, const std::set<std::string>& ids)
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

/**
 * @brief Log registry event (currently disabled for release builds)
 * @param typeName Item type name
 * @param beforeCount Count before adding
 * @param addedCount Number of IDs added
 * @param afterCount Count after adding
 */
static void LogRegistryEvent(const std::string& typeName, size_t beforeCount, size_t addedCount, size_t afterCount)
{
    // Registry event logging (currently disabled for release)
    // Can be re-enabled for debugging if needed
}

/**
 * @brief Append item IDs to registry file
 * @tparam T Item type with an 'id' member
 * @param typeName Item type name
 * @param items Vector of items to extract IDs from
 * @details Loads existing registry, adds new IDs, and saves back
 */
template <typename T>
static void AppendIdsToRegistry(const std::string& typeName, const std::vector<T>& items)
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

/**
 * @brief Clean trailing commas from JSON text
 * @param text JSON text that may contain trailing commas
 * @return Cleaned JSON text without trailing commas
 */
static std::string CleanJsonTrailingCommas(const std::string& text)
{
    std::string s = text;
    auto replace_all = [&](const std::string& from, const std::string& to)
    {
        size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos)
        {
            s.replace(pos, from.size(), to);
        }
    };

    replace_all(",\r\n]", "\r\n]");
    replace_all(",\n]", "\n]");
    replace_all(", ]", " ]");
    replace_all(",]", "]");
    return s;
}

/**
 * @brief Save text content to a file
 * @param path File path to save to
 * @param text Text content to save
 * @return true if saved successfully, false otherwise
 */
static bool SaveTextFile(const std::string& path, const std::string& text)
{
    EnsureParentDir(path);
    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[SaveTextFile] Failed to open file: " << path << "\n";
        return false;
    }
    ofs << text;
    return true;
}

/**
 * @brief Get current timestamp as formatted string
 * @return Timestamp string in format "YYYY-MM-DD HH:MM:SS"
 */
static std::string GetCurrentTimestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm localTm{};
#ifdef _WIN32
    localtime_s(&localTm, &now);
#else
    localtime_r(&now, &localTm);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTm);
    return buffer;
}

// ============================================================================
// SECTION: Forward Declarations
// ============================================================================
/**
 * @brief Forward declarations for ProcessLLMResponse functions
 * @details All ProcessLLMResponse_* functions use qualityResults for unified invalid item filtering
 */
static int ProcessLLMResponse_Ammo(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Armor(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Clothing(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Drink(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Medicine(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Food(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Material(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Weapon(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_WeaponComponent(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);

/**
 * @namespace ItemGenerator
 * @brief Main namespace for item generation functions
 */
namespace ItemGenerator
{
    /**
     * @brief Generate items using LLM in single batch mode
     * @param args Command line arguments
     * @param existingIds Set of existing IDs to exclude
     * @param useCustomPreset Whether to use custom preset
     * @param customPreset Custom preset configuration
     * @return 0 on success, non-zero on failure
     */
    int GenerateWithLLM_SingleBatch(const CommandLineArgs& args, const std::set<std::string>& existingIds, bool useCustomPreset, const CustomPreset& customPreset);
}

// ============================================================================
// SECTION: Common Template-Based Processing Logic
// ============================================================================
/**
 * @brief Common template-based processing logic for LLM responses
 * 
 * This template function extracts the common logic from ProcessLLMResponse_* functions.
 * Type-specific operations are passed as function objects/lambdas. All item types now
 * use qualityResults for unified invalid item filtering.
 * 
 * @tparam ItemType The item data type (ItemFoodData, ItemDrinkData, etc.)
 * @tparam ParseFunc Function type: bool(const std::string&, std::vector<ItemType>&)
 * @tparam QualityCheckFunc Function type: QualityChecker::QualityResult(const ItemType&)
 * @tparam WriteFunc Function type: bool(const std::string&, const std::vector<ItemType>&)
 * @tparam GetExistingIdsFunc Function type: std::set<std::string>(const std::string&)
 * @tparam BuildPromptFunc Function type: std::string(...)
 * @tparam ProcessRecursiveFunc Function type: int(...)
 * @tparam PostProcessFunc Function type: void(std::vector<ItemType>&) - optional, called before quality check
 * @tparam PostQualityPostProcessFunc Function type: void(std::vector<ItemType>&, std::map<std::string, QualityChecker::QualityResult>&) - optional, called after quality check
 * 
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude (duplicates)
 * @param modelName LLM model name for recursive calls
 * @param params Generation parameters (maxHunger, maxThirst, etc.)
 * @param preset Preset type (Default, Forest, Desert, etc.)
 * @param itemTypeName Human-readable item type name (for logging)
 * @param registrySlug Registry slug for ID tracking
 * @param parseFunc Function to parse JSON into items
 * @param qualityCheckFunc Function to perform quality checking
 * @param writeFunc Function to write items to file
 * @param getExistingIdsFunc Function to load existing IDs from file
 * @param buildPromptFunc Function to build LLM prompt
 * @param processRecursiveFunc Function to recursively generate more items if needed
 * @param[in,out] qualityResults Map to store quality check results (used for filtering)
 * @param postProcessFunc Optional function to post-process items before validation (e.g., variety enhancement)
 * @param postQualityPostProcessFunc Optional function to post-process items after quality check (e.g., value error fixes)
 * 
 * @return Number of items successfully added, or negative on error
 * 
 * @note Saves raw JSON response to .raw.json file
 * @note Filters out duplicate IDs and invalid items (based on qualityResults)
 * @note Automatically merges with existing items in output file
 * @note Recursively generates more items if count is insufficient
 */
template<typename ItemType, typename ParseFunc, typename QualityCheckFunc, typename WriteFunc, typename GetExistingIdsFunc, typename BuildPromptFunc, typename ProcessRecursiveFunc, typename PostProcessFunc = std::function<void(std::vector<ItemType>&)>, typename PostQualityPostProcessFunc = std::function<void(std::vector<ItemType>&, std::map<std::string, QualityChecker::QualityResult>&)>>
static int ProcessLLMResponse_Common(
    const std::string& jsonResponse,
    const std::string& outputPath,
    int requiredCount,
    const std::set<std::string>& excludeIds,
    const std::string& modelName,
    const FoodGenerateParams& params,
    PresetType preset,
    const std::string& itemTypeName,
    const std::string& registrySlug,
    ParseFunc parseFunc,
    QualityCheckFunc qualityCheckFunc,
    WriteFunc writeFunc,
    GetExistingIdsFunc getExistingIdsFunc,
    BuildPromptFunc buildPromptFunc,
    ProcessRecursiveFunc processRecursiveFunc,
    std::map<std::string, QualityChecker::QualityResult>& qualityResults,
    PostProcessFunc postProcessFunc = [](std::vector<ItemType>&) {},
    PostQualityPostProcessFunc postQualityPostProcessFunc = [](std::vector<ItemType>&, std::map<std::string, QualityChecker::QualityResult>&) {},
    int maxRegenerationAttempts = 10)  // Add max regeneration attempts parameter
{
    if (jsonResponse.empty())
    {
        std::cout << "[ItemGenerator] LLM response is empty.\n";
        return 1;
    }

    std::string rawPath = outputPath + ".raw.json";
    if (SaveTextFile(rawPath, jsonResponse))
    {
        std::cout << "[ItemGenerator] Saved LLM raw JSON to " << rawPath << "\n";
    }

    std::vector<ItemType> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!parseFunc(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed " << itemTypeName << " From LLM (" << items.size() << ") ===\n";
    
    // Post-process items (type-specific adjustments)
    postProcessFunc(items);
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = qualityCheckFunc(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        qualityResults[item.id] = qualityResult; // Store for filtering (unified approach)
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary(itemTypeName, warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

    // Post-quality post-process (e.g., fix value errors)
    postQualityPostProcessFunc(items, qualityResults);

    // Filter out duplicates and invalid items (using qualityResults)
    std::vector<ItemType> newItems;
    std::set<std::string> newIds = excludeIds;
    int filteredInvalidCount = 0;
    for (const auto& item : items)
    {
        if (newIds.find(item.id) != newIds.end())
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
            continue;
        }
        
        // Filter out invalid items (isValid = false)
        auto it = qualityResults.find(item.id);
        if (it != qualityResults.end())
        {
            if (!it->second.isValid)
            {
                std::cout << "[ItemGenerator] Filtered out invalid item (quality check failed): " << item.id << "\n";
                filteredInvalidCount++;
                continue;
            }
        }
        
        newItems.push_back(item);
        newIds.insert(item.id);
    }
    
    if (filteredInvalidCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << filteredInvalidCount << " invalid items (quality check failed).\n";
    }

    int addedCount = static_cast<int>(newItems.size());
    int skippedCount = static_cast<int>(items.size()) - addedCount;

    if (skippedCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << skippedCount << " duplicate items.\n";
    }

    // CRITICAL: Always write exactly requiredCount items, ignoring existing file
    // Limit newItems to requiredCount if it exceeds
    int actualAddedCount = static_cast<int>(newItems.size());
    if (actualAddedCount > requiredCount)
    {
        std::cout << "[ItemGenerator] Generated " << actualAddedCount << " items but only " << requiredCount << " requested. Limiting to " << requiredCount << " items.\n";
        newItems.resize(requiredCount);
        actualAddedCount = requiredCount;
    }
    
    // Check if file exists and has items (for parallel batch mode)
    // In parallel mode, multiple batches may write to the same file, so we need to merge
    std::vector<ItemType> currentItems;
    std::vector<ItemType> mergedItems;  // Declare outside if block for later use
    std::ifstream ifs(outputPath);
    if (ifs.is_open())
    {
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!content.empty())
        {
            parseFunc(content, currentItems);
        }
    }
    
    // CRITICAL: Detect parallel batch mode before merging
    // In parallel batch mode, multiple batches write to the same file concurrently.
    // If we regenerate here, each batch will try to reach its own requiredCount,
    // but the file already contains items from other batches, causing over-generation.
    // The final merge in GenerateWithLLM will handle the total count limit.
    // Detection: If requiredCount is exactly BATCH_SIZE (15) and file has items from other batches,
    // or if merged result exceeds requiredCount, we're in parallel batch mode.
    const int BATCH_SIZE = 15;
    bool isParallelBatchMode = false;
    if (!currentItems.empty())
    {
        // If file has items and requiredCount is BATCH_SIZE, likely parallel batch mode
        if (requiredCount == BATCH_SIZE && static_cast<int>(currentItems.size()) >= BATCH_SIZE)
        {
            isParallelBatchMode = true;
        }
    }
    
        // If file has existing items, merge them (for parallel batch mode)
        // Otherwise, write newItems directly (overwrite existing file)
        if (!currentItems.empty())
        {
            // Merge mode: combine existing items with new items, avoiding duplicates
            std::set<std::string> mergedIds;
            
            // Add existing items first
            for (const auto& item : currentItems)
            {
                if (mergedIds.find(item.id) == mergedIds.end())
                {
                    mergedItems.push_back(item);
                    mergedIds.insert(item.id);
                }
            }
            
            // Add new items (avoiding duplicates)
            for (const auto& item : newItems)
            {
                if (mergedIds.find(item.id) == mergedIds.end())
                {
                    mergedItems.push_back(item);
                    mergedIds.insert(item.id);
                }
            }
            
            // Update isParallelBatchMode based on merged result
            // If merged items exceed requiredCount, we're definitely in parallel batch mode
            if (static_cast<int>(mergedItems.size()) > requiredCount)
            {
                isParallelBatchMode = true;
                std::cout << "[ItemGenerator] Detected parallel batch mode: merged " << mergedItems.size() << " items (batch target: " << requiredCount << "). Skipping regeneration.\n";
            }
            
            // CRITICAL: In parallel batch mode, don't limit here!
            // The requiredCount is per batch (e.g., 10), not the total (e.g., 50).
            // Limiting here would cut off items from other parallel batches.
            // The final merge in GenerateWithLLM will handle the total count limit.
            // So we write all merged items without limiting.
            if (!writeFunc(mergedItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write JSON file.\n";
                return 1;
            }
            
            actualAddedCount = static_cast<int>(mergedItems.size()) - static_cast<int>(currentItems.size());
            std::cout << "[ItemGenerator] Merged " << currentItems.size() << " existing items with " << newItems.size() << " new items (total: " << mergedItems.size() << ").\n";
        }
    else
    {
        // Write newItems directly (overwrite existing file)
        // This ensures we always have exactly the requested count, ignoring any existing items
        if (!writeFunc(newItems, outputPath))
        {
            std::cout << "[ItemGenerator] Failed to write JSON file.\n";
            return 1;
        }
        
        std::cout << "[ItemGenerator] Wrote " << actualAddedCount << " " << itemTypeName << " items to " << outputPath << " (target: " << requiredCount << ").\n";
    }
    
    // Calculate stillNeeded: how many more items we need to reach requiredCount
    int stillNeeded = requiredCount - actualAddedCount;
    if (stillNeeded < 0) stillNeeded = 0;  // Safety check

    AppendIdsToRegistry(registrySlug, newItems);
    
    // Regeneration logic with max attempts limit to prevent infinite loops
    // BUT: Skip regeneration in parallel batch mode to prevent over-generation
    static std::map<std::string, int> regenerationAttempts;  // Track attempts per output path
    std::string attemptKey = outputPath + "_" + itemTypeName;
    
    // Loop until we have exactly requiredCount items or max attempts reached
    // BUT: Skip this loop entirely in parallel batch mode
    while (stillNeeded > 0 && !isParallelBatchMode)
    {
        if (regenerationAttempts[attemptKey] >= maxRegenerationAttempts)
        {
            std::cout << "[ItemGenerator] Warning: Maximum regeneration attempts (" << maxRegenerationAttempts << ") reached for " << itemTypeName << ". Stopping to prevent infinite loop.\n";
            std::cout << "[ItemGenerator] Current count: " << (requiredCount - stillNeeded) << ", Required: " << requiredCount << ", Still needed: " << stillNeeded << "\n";
            regenerationAttempts[attemptKey] = 0;  // Reset for next generation
            return 0;  // Return success even if count is not exact
        }
        
        regenerationAttempts[attemptKey]++;
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items... (Attempt " << regenerationAttempts[attemptKey] << "/" << maxRegenerationAttempts << ")\n";
        
        // CRITICAL: For regeneration, we need to exclude IDs from the current file to avoid duplicates
        std::set<std::string> currentFileIds = getExistingIdsFunc(outputPath);
        
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = buildPromptFunc(
            additionalParams,
            preset,
            currentFileIds,  // Exclude IDs from current file to avoid duplicates
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(currentFileIds.size()));
        
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (additionalResponse.empty())
        {
            std::cout << "[ItemGenerator] Warning: Failed to get additional response for " << stillNeeded << " more items.\n";
            regenerationAttempts[attemptKey] = 0;  // Reset on failure
            return 1;
        }
        
        // Process the additional response to get new items
        std::vector<ItemType> additionalItems;
        std::string cleanedJson = CleanJsonTrailingCommas(additionalResponse);
        if (!parseFunc(cleanedJson, additionalItems))
        {
            std::cout << "[ItemGenerator] Failed to parse additional LLM JSON.\n";
            regenerationAttempts[attemptKey] = 0;
            return 1;
        }
        
        // Post-process additional items
        postProcessFunc(additionalItems);
        
        // Filter duplicates and invalid items from additional items
        std::vector<ItemType> validAdditionalItems;
        std::set<std::string> additionalIds = currentFileIds;  // Start with current file IDs
        for (const auto& item : additionalItems)
        {
            if (additionalIds.find(item.id) != additionalIds.end())
            {
                continue;  // Skip duplicates
            }
            
            auto qualityResult = qualityCheckFunc(item);
            if (!qualityResult.isValid)
            {
                continue;  // Skip invalid items
            }
            
            validAdditionalItems.push_back(item);
            additionalIds.insert(item.id);
        }
        
        // CRITICAL: If no valid additional items were generated, we can't make progress
        if (validAdditionalItems.empty())
        {
            std::cout << "[ItemGenerator] Warning: No valid additional items generated (all were duplicates or invalid). Cannot make progress.\n";
            // Don't break the loop yet - let maxRegenerationAttempts handle it
            // But we need to prevent infinite loop by not updating stillNeeded
            // Actually, we should break here to avoid wasting attempts
            regenerationAttempts[attemptKey] = 0;
            return 0;  // Return with current count (may be less than requiredCount)
        }
        
        // CRITICAL: Use file-level mutex to prevent race conditions when multiple batches write to the same file
        std::mutex& fileMutex = GetFileMutex(outputPath);
        std::lock_guard<std::mutex> fileLock(fileMutex);
        
        // Read current file items
        std::vector<ItemType> currentItems;
        std::ifstream ifs(outputPath);
        if (ifs.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            if (!content.empty())
            {
                parseFunc(content, currentItems);
            }
        }
        
        // Merge: current items + new additional items
        std::vector<ItemType> mergedItems = currentItems;
        std::set<std::string> mergedIds;
        for (const auto& item : currentItems)
        {
            mergedIds.insert(item.id);
        }
        
        for (const auto& item : validAdditionalItems)
        {
            if (mergedIds.find(item.id) == mergedIds.end())
            {
                mergedItems.push_back(item);
                mergedIds.insert(item.id);
            }
        }
        
        // CRITICAL: Limit to exactly requiredCount
        // BUT: In parallel batch mode, currentItems may already exceed requiredCount
        // (because other batches added items). In that case, don't limit - let the
        // final merge in GenerateWithLLM handle the total count limit.
        int mergedCount = static_cast<int>(mergedItems.size());
        int currentCount = static_cast<int>(currentItems.size());
        
        // Only limit if we're not in parallel batch mode (detected by currentCount <= requiredCount)
        // If currentCount > requiredCount, we're in parallel mode and other batches have added items
        if (mergedCount > requiredCount && currentCount <= requiredCount)
        {
            std::cout << "[ItemGenerator] Warning: After merging, have " << mergedCount << " items (target: " << requiredCount << "). Limiting to " << requiredCount << " items.\n";
            mergedItems.resize(requiredCount);
            mergedCount = requiredCount;
        }
        else if (mergedCount > requiredCount && currentCount > requiredCount)
        {
            // Parallel batch mode: don't limit here, final merge will handle it
            std::cout << "[ItemGenerator] Note: In parallel batch mode, have " << mergedCount << " items (batch target: " << requiredCount << "). Final merge will handle total count.\n";
        }
        
        // Write merged items (overwrite file)
        if (!writeFunc(mergedItems, outputPath))
        {
            std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
            regenerationAttempts[attemptKey] = 0;
            return 1;
        }
        
        std::cout << "[ItemGenerator] Merged " << currentItems.size() << " existing items with " << validAdditionalItems.size() << " new items (total: " << mergedCount << ", target: " << requiredCount << ").\n";
        
        // Update stillNeeded for next iteration
        stillNeeded = requiredCount - mergedCount;
        if (stillNeeded < 0) stillNeeded = 0;  // Safety check (shouldn't happen, but just in case)
    }
    
    // Success: we have exactly requiredCount items (or reached max attempts)
    regenerationAttempts[attemptKey] = 0;  // Reset on completion
    std::cout << "[ItemGenerator] Successfully generated " << requiredCount << " " << itemTypeName << " items (target: " << requiredCount << ").\n";

    return 0;
}

// ============================================================================
// SECTION: Type-Specific ProcessLLMResponse Implementations
// ============================================================================
// These functions wrap ProcessLLMResponse_Common with type-specific lambdas
// All functions follow the same pattern: parse, quality check, write, merge

/**
 * @brief Process LLM response for Food items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 */
static int ProcessLLMResponse_Food(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    return ProcessLLMResponse_Common<ItemFoodData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Food", "food",
        [](const std::string& json, std::vector<ItemFoodData>& out) { return ItemJsonParser::ParseFoodFromJsonText(json, out); },
        [](const ItemFoodData& item) { return QualityChecker::CheckFoodQuality(item); },
        [](const std::vector<ItemFoodData>& items, const std::string& path) { return ItemJsonWriter::WriteFoodToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingFoodIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildFoodJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Food(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemFoodData>& items) {
            for (auto& item : items)
            {
                if (item.spoils && item.spoilTimeMinutes < 60)
                {
                    item.spoilTimeMinutes = 60;
                }
                std::cout << "- " << item.id
                    << " / " << item.displayName
                    << " / category: " << item.category
                    << " / Hunger +" << item.hungerRestore
                    << ", Thirst +" << item.thirstRestore
                    << "\n";
            }
        },
        [](std::vector<ItemFoodData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            FixFoodQualityWarnings(items, qualityResults);
            CheckDuplicateNames(items, "Food");
        }
    );
}

/**
 * @brief Process LLM response for Drink items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 */
static int ProcessLLMResponse_Drink(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    return ProcessLLMResponse_Common<ItemDrinkData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Drink", "drink",
        [](const std::string& json, std::vector<ItemDrinkData>& out) { return ItemJsonParser::ParseDrinkFromJsonText(json, out); },
        [](const ItemDrinkData& item) { return QualityChecker::CheckDrinkQuality(item); },
        [](const std::vector<ItemDrinkData>& items, const std::string& path) { return ItemJsonWriter::WriteDrinkToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingDrinkIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildDrinkJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Drink(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemDrinkData>& items) {
            for (auto& item : items)
            {
                if (item.spoils && item.spoilTimeMinutes < 60)
                {
                    item.spoilTimeMinutes = 60;
                }
                std::cout << "- " << item.id
                    << " / " << item.displayName
                    << " / category: " << item.category
                    << " / Hunger +" << item.hungerRestore
                    << ", Thirst +" << item.thirstRestore
                    << "\n";
            }
        },
        [](std::vector<ItemDrinkData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            FixDrinkQualityWarnings(items, qualityResults);
        }
    );
}

static int ProcessLLMResponse_Medicine(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    return ProcessLLMResponse_Common<ItemMedicineData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Medicine", "medicine",
        [](const std::string& json, std::vector<ItemMedicineData>& out) { return ItemJsonParser::ParseMedicineFromJsonText(json, out); },
        [](const ItemMedicineData& item) { return QualityChecker::CheckMedicineQuality(item); },
        [](const std::vector<ItemMedicineData>& items, const std::string& path) { return ItemJsonWriter::WriteMedicineToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingMedicineIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildMedicineJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Medicine(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemMedicineData>& items) {
            for (auto& item : items)
            {
                if (item.spoils && item.spoilTimeMinutes < 60)
                {
                    item.spoilTimeMinutes = 60;
                }
                std::cout << "- " << item.id
                    << " / " << item.displayName
                    << " / category: " << item.category
                    << " / Health +" << item.healthRestore
                    << "\n";
            }
        },
        [](std::vector<ItemMedicineData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            FixMedicineQualityWarnings(items, qualityResults);
        }
    );
}

/**
 * @brief Process LLM response for Material items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 */
static int ProcessLLMResponse_Material(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    return ProcessLLMResponse_Common<ItemMaterialData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Material", "material",
        [](const std::string& json, std::vector<ItemMaterialData>& out) { return ItemJsonParser::ParseMaterialFromJsonText(json, out); },
        [](const ItemMaterialData& item) { return QualityChecker::CheckMaterialQuality(item); },
        [](const std::vector<ItemMaterialData>& items, const std::string& path) { return ItemJsonWriter::WriteMaterialToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingMaterialIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildMaterialJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Material(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemMaterialData>& items) {
            for (const auto& item : items)
            {
                std::cout << "- " << item.id
                    << " / " << item.displayName
                    << " / category: " << item.category
                    << " / materialType: " << item.materialType
                    << " / value: " << item.value
                    << "\n";
            }
        },
        [](std::vector<ItemMaterialData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            FixMaterialValueWarnings(items, qualityResults);
        }
    );
}

/**
 * @brief Process LLM response for Weapon items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 */
static int ProcessLLMResponse_Weapon(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    return ProcessLLMResponse_Common<ItemWeaponData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Weapon", "weapon",
        [](const std::string& json, std::vector<ItemWeaponData>& out) { return ItemJsonParser::ParseWeaponFromJsonText(json, out); },
        [](const ItemWeaponData& item) { return QualityChecker::CheckWeaponQuality(item); },
        [](const std::vector<ItemWeaponData>& items, const std::string& path) { return ItemJsonWriter::WriteWeaponToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingWeaponIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildWeaponJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Weapon(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemWeaponData>& items) {
            for (auto& item : items)
            {
                if (item.weaponType.empty())
                {
                    if (item.weaponCategory == "Melee")
                        item.weaponType = "MeleeGeneric";
                    else
                        item.weaponType = "RangedGeneric";
                }
                if (item.weight < 500)
                {
                    item.weight = 500;
                }
                std::cout << "- " << item.id << " / " << item.displayName
                    << " / " << item.weaponType << " / Damage: " << item.minDamage << "-" << item.maxDamage
                    << " / FireRate: " << item.fireRate << "\n";
            }
        },
        [](std::vector<ItemWeaponData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            FixWeaponErrors(items, qualityResults);
        }
    );
}

/**
 * @brief Process LLM response for WeaponComponent items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 */
static int ProcessLLMResponse_WeaponComponent(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    return ProcessLLMResponse_Common<ItemWeaponComponentData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "WeaponComponent", "weaponcomponent",
        [](const std::string& json, std::vector<ItemWeaponComponentData>& out) { return ItemJsonParser::ParseWeaponComponentFromJsonText(json, out); },
        [](const ItemWeaponComponentData& item) { return QualityChecker::CheckWeaponComponentQuality(item); },
        [](const std::vector<ItemWeaponComponentData>& items, const std::string& path) { return ItemJsonWriter::WriteWeaponComponentToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingWeaponComponentIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildWeaponComponentJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_WeaponComponent(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemWeaponComponentData>& items) {
            for (const auto& item : items)
            {
                std::cout << "- " << item.id << " / " << item.displayName
                    << " / " << item.componentType << "\n";
            }
        },
        [](std::vector<ItemWeaponComponentData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            FixWeaponComponentErrors(items, qualityResults);
        }
    );
}

/**
 * @brief Process LLM response for Armor items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 * @note Uses basic validation as QualityChecker::CheckArmorQuality is not yet implemented
 */
static int ProcessLLMResponse_Armor(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    // Note: QualityChecker::CheckArmorQuality not yet implemented, using basic validation
    return ProcessLLMResponse_Common<ItemArmorData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Armor", "armor",
        [](const std::string& json, std::vector<ItemArmorData>& out) { return ItemJsonParser::ParseArmorFromJsonText(json, out); },
        [](const ItemArmorData& item) { 
            QualityChecker::QualityResult result;
            result.isValid = true;
            result.qualityScore = 100.0f;
            // Basic validation - can be enhanced later
            if (item.armorValue < 0 || item.armorValue > 100) {
                result.isValid = false;
                result.errors.push_back("armorValue out of range (0-100)");
            }
            return result;
        },
        [](const std::vector<ItemArmorData>& items, const std::string& path) { return ItemJsonWriter::WriteArmorToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingArmorIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildArmorJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Armor(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemArmorData>& items) {
            for (const auto& item : items)
            {
                std::cout << "- " << item.id
                    << " / " << item.displayName
                    << " / category: " << item.category
                    << " / Armor: " << item.armorValue
                    << ", Durability: " << item.durability
                    << "\n";
            }
        }
    );
}

/**
 * @brief Process LLM response for Clothing items
 * @param jsonResponse Raw JSON response from LLM
 * @param outputPath Path to output JSON file
 * @param requiredCount Target number of items to generate
 * @param excludeIds Set of existing item IDs to exclude
 * @param modelName LLM model name
 * @param params Generation parameters
 * @param preset World preset type
 * @param qualityResults Map to store quality check results
 * @return Number of items successfully added, or negative on error
 * @note Uses basic validation as QualityChecker::CheckClothingQuality is not yet implemented
 */
static int ProcessLLMResponse_Clothing(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults)
    // Note: QualityChecker::CheckClothingQuality not yet implemented, using basic validation
    return ProcessLLMResponse_Common<ItemClothingData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Clothing", "clothing",
        [](const std::string& json, std::vector<ItemClothingData>& out) { return ItemJsonParser::ParseClothingFromJsonText(json, out); },
        [](const ItemClothingData& item) { 
            QualityChecker::QualityResult result;
            result.isValid = true;
            result.qualityScore = 100.0f;
            // Basic validation - can be enhanced later
            if (item.warmth < 0 || item.warmth > 100 || item.style < 0 || item.style > 100) {
                result.isValid = false;
                result.errors.push_back("warmth or style out of range (0-100)");
            }
            return result;
        },
        [](const std::vector<ItemClothingData>& items, const std::string& path) { return ItemJsonWriter::WriteClothingToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingClothingIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildClothingJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Clothing(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemClothingData>& items) {
            for (const auto& item : items)
            {
                std::cout << "- " << item.id
                    << " / " << item.displayName
                    << " / category: " << item.category
                    << " / Warmth: " << item.warmth
                    << ", Style: " << item.style
                    << "\n";
            }
        },
        [](std::vector<ItemClothingData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            CheckDuplicateNames(items, "Clothing");
        }
    );
}

static int ProcessLLMResponse_Ammo(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function with Ammo-specific post-processing
    return ProcessLLMResponse_Common<ItemAmmoData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Ammo", "ammo",
        [](const std::string& json, std::vector<ItemAmmoData>& out) { return ItemJsonParser::ParseAmmoFromJsonText(json, out); },
        [](const ItemAmmoData& item) { return QualityChecker::CheckAmmoQuality(item); },
        [](const std::vector<ItemAmmoData>& items, const std::string& path) { return ItemJsonWriter::WriteAmmoToFile(items, path); },
        [](const std::string& path) { return ItemJsonWriter::GetExistingAmmoIds(path); },
        [](const FoodGenerateParams& p, PresetType pr, const std::set<std::string>& ids, const std::string& m, const std::string& t, int c) {
            return PromptBuilder::BuildAmmoJsonPrompt(p, pr, ids, m, t, c);
        },
        [](const std::string& resp, const std::string& path, int count, const std::set<std::string>& ids, const std::string& m, const FoodGenerateParams& p, PresetType pr, std::map<std::string, QualityChecker::QualityResult>& qr) {
            return ProcessLLMResponse_Ammo(resp, path, count, ids, m, p, pr, qr);
        },
        qualityResults,
        [](std::vector<ItemAmmoData>& items) {
            // Pre-quality post-process: clamp Common rarity stats and log items
            PostProcess_Ammo(items);
        },
        [](std::vector<ItemAmmoData>& items, std::map<std::string, QualityChecker::QualityResult>& qualityResults) {
            // Post-quality post-process: fix value errors and ensure variety
            FixValueErrors_Ammo(items, qualityResults);
            EnsureVariety_Ammo(items);
        }
    );
}

// ============================================================================
// SECTION: Main ItemGenerator Namespace - High-Level Generation Functions
// ============================================================================

namespace ItemGenerator
{
    namespace
    {
        struct BatchRunResult
        {
            BatchItem item;
            std::string outputPath;
            bool success = false;
            int exitCode = 0;
            double durationSeconds = 0.0;
        };

        std::string FormatSeconds(double seconds)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << seconds << "s";
            return oss.str();
        }
    }

    /**
     * @brief Generate items using LLM with parallel batch processing
     * @param args Command line arguments containing item type, count, output path, etc.
     * @return 0 on success, non-zero on failure
     * 
     * @details
     * This function implements the main LLM-based item generation logic:
     * - Loads existing IDs from registry and output files to prevent duplicates
     * - Splits large requests into parallel batches for efficiency
     * - Uses thread-safe file I/O with per-file mutexes
     * - Ensures exactly the requested number of items are generated
     * - Handles retries and additional generation if needed
     * 
     * @note For requests <= 10 items, uses single batch mode. For larger requests,
     *       splits into parallel batches (default: 10 items per batch, max 3 concurrent).
     */
    int GenerateWithLLM(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in LLM mode (Parallel).\n";

        // Get existing IDs to avoid duplicates in prompt
        std::set<std::string> existingIds;
        std::string typeSlug = CommandLineParser::GetItemTypeName(args.itemType);
        std::set<std::string> registryIds = LoadRegistryIds(typeSlug);
        size_t registryCount = registryIds.size();
        if (args.itemType == ItemType::Food)
        {
            existingIds = ItemJsonWriter::GetExistingFoodIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Drink)
        {
            existingIds = ItemJsonWriter::GetExistingDrinkIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Medicine)
        {
            existingIds = ItemJsonWriter::GetExistingMedicineIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Material)
        {
            existingIds = ItemJsonWriter::GetExistingMaterialIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Weapon)
        {
            existingIds = ItemJsonWriter::GetExistingWeaponIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::WeaponComponent)
        {
            existingIds = ItemJsonWriter::GetExistingWeaponComponentIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Ammo)
        {
            existingIds = ItemJsonWriter::GetExistingAmmoIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Armor)
        {
            existingIds = ItemJsonWriter::GetExistingArmorIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Clothing)
        {
            existingIds = ItemJsonWriter::GetExistingClothingIds(args.params.outputPath);
        }

        size_t beforeMerge = existingIds.size();
        existingIds.insert(registryIds.begin(), registryIds.end());
        size_t afterMerge = existingIds.size();
        LogRegistryEvent(typeSlug, beforeMerge, afterMerge - beforeMerge, afterMerge);

        // Load custom preset if specified
        CustomPreset customPreset;
        bool useCustomPreset = !args.customPresetPath.empty();
        if (useCustomPreset)
        {
            if (!CustomPresetManager::LoadPresetFromFile(args.customPresetPath, customPreset))
            {
                std::cerr << "[ItemGenerator] Failed to load custom preset from: " << args.customPresetPath << "\n";
                return 1;
            }
            std::cout << "[ItemGenerator] Using custom preset: " << customPreset.displayName << " (" << customPreset.id << ")\n";
        }

        // Parallel batch processing: Split large requests into parallel batches
        const int BATCH_SIZE = 15; // Items per batch (increased from 10 to reduce number of batches and improve throughput)
        const int MAX_CONCURRENT_BATCHES = 5; // Max concurrent batches (balanced for Ollama server capacity)
        int totalCount = args.params.count;
        
        if (totalCount <= BATCH_SIZE)
        {
            // Small request: process normally (single request)
            return GenerateWithLLM_SingleBatch(args, existingIds, useCustomPreset, customPreset);
        }

        // Large request: split into parallel batches
        std::cout << "[ItemGenerator] Splitting " << totalCount << " items into parallel batches (batch size: " << BATCH_SIZE << ", max concurrent: " << MAX_CONCURRENT_BATCHES << ")\n";
        
        int numBatches = (totalCount + BATCH_SIZE - 1) / BATCH_SIZE; // Ceiling division
        std::vector<std::future<int>> futures;
        std::mutex outputMutex;
        std::mutex idsMutex;
        std::set<std::string> sharedExistingIds = existingIds; // Shared across batches
        
        auto batchStart = std::chrono::steady_clock::now();
        
        // Launch batches with concurrency limit
        for (int batchIdx = 0; batchIdx < numBatches; ++batchIdx)
        {
            // Wait if we've reached max concurrent batches
            while (futures.size() >= MAX_CONCURRENT_BATCHES)
            {
                // Remove completed futures first
                futures.erase(
                    std::remove_if(futures.begin(), futures.end(),
                        [](std::future<int>& f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }),
                    futures.end());
                
                // If still at limit, wait for at least one to complete (more efficient than polling)
                if (futures.size() >= MAX_CONCURRENT_BATCHES)
                {
                    // Wait for the first future to complete (with timeout to avoid infinite wait)
                    auto it = std::find_if(futures.begin(), futures.end(),
                        [](std::future<int>& f) {
                            return f.wait_for(std::chrono::milliseconds(50)) == std::future_status::ready;
                        });
                    
                    // Remove completed futures again after waiting
                    futures.erase(
                        std::remove_if(futures.begin(), futures.end(),
                            [](std::future<int>& f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }),
                        futures.end());
                }
            }
            
            int batchCount = (batchIdx == numBatches - 1) ? (totalCount - batchIdx * BATCH_SIZE) : BATCH_SIZE;
            
            // Create batch args
            CommandLineArgs batchArgs = args;
            batchArgs.params.count = batchCount;
            
            // Launch async batch
            auto future = std::async(std::launch::async, [batchArgs, batchIdx, numBatches, &sharedExistingIds, &idsMutex, &outputMutex, useCustomPreset, customPreset]() -> int {
                // Get current existing IDs (thread-safe)
                std::set<std::string> batchExistingIds;
                {
                    std::lock_guard<std::mutex> lock(idsMutex);
                    batchExistingIds = sharedExistingIds;
                }
                
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << "[ItemGenerator] Starting batch " << (batchIdx + 1) << "/" << numBatches << " (" << batchArgs.params.count << " items)\n";
                }
                
                int result = GenerateWithLLM_SingleBatch(batchArgs, batchExistingIds, useCustomPreset, customPreset);
                
                // Update shared existing IDs (thread-safe)
                if (result == 0)
                {
                    std::set<std::string> newIds;
                    if (batchArgs.itemType == ItemType::Food)
                    {
                        newIds = ItemJsonWriter::GetExistingFoodIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Drink)
                    {
                        newIds = ItemJsonWriter::GetExistingDrinkIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Medicine)
                    {
                        newIds = ItemJsonWriter::GetExistingMedicineIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Material)
                    {
                        newIds = ItemJsonWriter::GetExistingMaterialIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Weapon)
                    {
                        newIds = ItemJsonWriter::GetExistingWeaponIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::WeaponComponent)
                    {
                        newIds = ItemJsonWriter::GetExistingWeaponComponentIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Ammo)
                    {
                        newIds = ItemJsonWriter::GetExistingAmmoIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Armor)
                    {
                        newIds = ItemJsonWriter::GetExistingArmorIds(batchArgs.params.outputPath);
                    }
                    else if (batchArgs.itemType == ItemType::Clothing)
                    {
                        newIds = ItemJsonWriter::GetExistingClothingIds(batchArgs.params.outputPath);
                    }
                    
                    {
                        std::lock_guard<std::mutex> lock(idsMutex);
                        sharedExistingIds.insert(newIds.begin(), newIds.end());
                    }
                    
                    {
                        std::lock_guard<std::mutex> lock(outputMutex);
                        std::cout << "[ItemGenerator] Completed batch " << (batchIdx + 1) << "/" << numBatches << "\n";
                    }
                }
                
                return result;
            });
            
            futures.push_back(std::move(future));
        }
        
        // Wait for all batches to complete and collect results
        int totalSuccess = 0;
        int totalFailed = 0;
        for (auto& future : futures)
        {
            int result = future.get();
            if (result == 0)
                totalSuccess++;
            else
                totalFailed++;
        }
        
        // After all batches complete, verify total count and generate additional items if needed
        // CRITICAL: Resize final file to exactly totalCount
        std::set<std::string> finalExistingIds;
        if (args.itemType == ItemType::Food)
        {
            finalExistingIds = ItemJsonWriter::GetExistingFoodIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Drink)
        {
            finalExistingIds = ItemJsonWriter::GetExistingDrinkIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Medicine)
        {
            finalExistingIds = ItemJsonWriter::GetExistingMedicineIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Material)
        {
            finalExistingIds = ItemJsonWriter::GetExistingMaterialIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Weapon)
        {
            finalExistingIds = ItemJsonWriter::GetExistingWeaponIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::WeaponComponent)
        {
            finalExistingIds = ItemJsonWriter::GetExistingWeaponComponentIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Ammo)
        {
            finalExistingIds = ItemJsonWriter::GetExistingAmmoIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Armor)
        {
            finalExistingIds = ItemJsonWriter::GetExistingArmorIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Clothing)
        {
            finalExistingIds = ItemJsonWriter::GetExistingClothingIds(args.params.outputPath);
        }
        
        int finalCount = static_cast<int>(finalExistingIds.size());
        
        // CRITICAL: Resize final file to exactly totalCount
        if (finalCount > totalCount)
        {
            // Read current items, resize to totalCount, and write back
            std::ifstream ifs(args.params.outputPath);
            if (ifs.is_open())
            {
                std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
                if (!content.empty())
                {
                    if (args.itemType == ItemType::Food)
                    {
                        std::vector<ItemFoodData> allItems;
                        ItemJsonParser::ParseFoodFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteFoodToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Drink)
                    {
                        std::vector<ItemDrinkData> allItems;
                        ItemJsonParser::ParseDrinkFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteDrinkToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Medicine)
                    {
                        std::vector<ItemMedicineData> allItems;
                        ItemJsonParser::ParseMedicineFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteMedicineToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Material)
                    {
                        std::vector<ItemMaterialData> allItems;
                        ItemJsonParser::ParseMaterialFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteMaterialToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Weapon)
                    {
                        std::vector<ItemWeaponData> allItems;
                        ItemJsonParser::ParseWeaponFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteWeaponToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::WeaponComponent)
                    {
                        std::vector<ItemWeaponComponentData> allItems;
                        ItemJsonParser::ParseWeaponComponentFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteWeaponComponentToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Ammo)
                    {
                        std::vector<ItemAmmoData> allItems;
                        ItemJsonParser::ParseAmmoFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteAmmoToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Armor)
                    {
                        std::vector<ItemArmorData> allItems;
                        ItemJsonParser::ParseArmorFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteArmorToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                    else if (args.itemType == ItemType::Clothing)
                    {
                        std::vector<ItemClothingData> allItems;
                        ItemJsonParser::ParseClothingFromJsonText(content, allItems);
                        if (static_cast<int>(allItems.size()) > totalCount)
                        {
                            allItems.resize(totalCount);
                            ItemJsonWriter::WriteClothingToFile(allItems, args.params.outputPath);
                            finalCount = totalCount;
                        }
                    }
                }
            }
        }
        
        int stillNeededAfterBatches = totalCount - finalCount;
        
        // Generate additional items if needed, with retry logic
        int maxRetries = 5;
        int retryCount = 0;
        while (stillNeededAfterBatches > 0 && retryCount < maxRetries)
        {
            std::cout << "[ItemGenerator] After parallel batches: " << finalCount << " items generated, " << stillNeededAfterBatches << " still needed. Generating additional items... (Attempt " << (retryCount + 1) << "/" << maxRetries << ")\n";
            
            // Generate remaining items
            CommandLineArgs additionalArgs = args;
            additionalArgs.params.count = stillNeededAfterBatches;
            int additionalResult = GenerateWithLLM_SingleBatch(additionalArgs, finalExistingIds, useCustomPreset, customPreset);
            
            if (additionalResult == 0)
            {
                // Re-read final count
                std::set<std::string> verifyIds;
                if (args.itemType == ItemType::Food)
                {
                    verifyIds = ItemJsonWriter::GetExistingFoodIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Drink)
                {
                    verifyIds = ItemJsonWriter::GetExistingDrinkIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Medicine)
                {
                    verifyIds = ItemJsonWriter::GetExistingMedicineIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Material)
                {
                    verifyIds = ItemJsonWriter::GetExistingMaterialIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Weapon)
                {
                    verifyIds = ItemJsonWriter::GetExistingWeaponIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::WeaponComponent)
                {
                    verifyIds = ItemJsonWriter::GetExistingWeaponComponentIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Ammo)
                {
                    verifyIds = ItemJsonWriter::GetExistingAmmoIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Armor)
                {
                    verifyIds = ItemJsonWriter::GetExistingArmorIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Clothing)
                {
                    verifyIds = ItemJsonWriter::GetExistingClothingIds(args.params.outputPath);
                }
                
                finalCount = static_cast<int>(verifyIds.size());
                finalExistingIds = verifyIds; // Update for next iteration
                stillNeededAfterBatches = totalCount - finalCount;
                retryCount++;
            }
            else
            {
                std::cout << "[ItemGenerator] Additional generation failed. Retrying...\n";
                retryCount++;
            }
        }
        
        // CRITICAL: Final resize to exactly totalCount
        // Read file, resize to exactly totalCount, and write back
        std::ifstream finalIfs(args.params.outputPath);
        if (finalIfs.is_open())
        {
            std::string finalContent((std::istreambuf_iterator<char>(finalIfs)), std::istreambuf_iterator<char>());
            finalIfs.close();
            if (!finalContent.empty())
            {
                bool needsResize = false;
                if (args.itemType == ItemType::Food)
                {
                    std::vector<ItemFoodData> allItems;
                    ItemJsonParser::ParseFoodFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteFoodToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Drink)
                {
                    std::vector<ItemDrinkData> allItems;
                    ItemJsonParser::ParseDrinkFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteDrinkToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Medicine)
                {
                    std::vector<ItemMedicineData> allItems;
                    ItemJsonParser::ParseMedicineFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteMedicineToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Material)
                {
                    std::vector<ItemMaterialData> allItems;
                    ItemJsonParser::ParseMaterialFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteMaterialToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Weapon)
                {
                    std::vector<ItemWeaponData> allItems;
                    ItemJsonParser::ParseWeaponFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteWeaponToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::WeaponComponent)
                {
                    std::vector<ItemWeaponComponentData> allItems;
                    ItemJsonParser::ParseWeaponComponentFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteWeaponComponentToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Ammo)
                {
                    std::vector<ItemAmmoData> allItems;
                    ItemJsonParser::ParseAmmoFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteAmmoToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Armor)
                {
                    std::vector<ItemArmorData> allItems;
                    ItemJsonParser::ParseArmorFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteArmorToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                else if (args.itemType == ItemType::Clothing)
                {
                    std::vector<ItemClothingData> allItems;
                    ItemJsonParser::ParseClothingFromJsonText(finalContent, allItems);
                    int currentCount = static_cast<int>(allItems.size());
                    if (currentCount != totalCount)
                    {
                        allItems.resize(totalCount);
                        ItemJsonWriter::WriteClothingToFile(allItems, args.params.outputPath);
                        finalCount = totalCount;
                        needsResize = true;
                    }
                }
                
                if (needsResize)
                {
                    std::cout << "[ItemGenerator] Final resize: Adjusted to exactly " << totalCount << " items.\n";
                }
            }
        }
        
        // Final status
        if (finalCount == totalCount)
        {
            std::cout << "[ItemGenerator] Successfully generated exactly " << finalCount << " items (target: " << totalCount << ").\n";
        }
        else if (finalCount > totalCount)
        {
            std::cout << "[ItemGenerator] Warning: Generated " << finalCount << " items (exceeds target: " << totalCount << "). Should have been resized.\n";
        }
        else
        {
            std::cout << "[ItemGenerator] Warning: Only generated " << finalCount << " items (target: " << totalCount << ").\n";
        }
        
        auto batchEnd = std::chrono::steady_clock::now();
        double totalDuration = std::chrono::duration<double>(batchEnd - batchStart).count();
        
        // Update registry with final items
        std::string registryTypeSlug = CommandLineParser::GetItemTypeName(args.itemType);
        if (finalCount > 0)
        {
            // Read final file content
            std::ifstream ifs(args.params.outputPath);
            std::string content;
            if (ifs.is_open())
            {
                content.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
            }
            
            if (!content.empty())
            {
                if (args.itemType == ItemType::Food)
                {
                    std::vector<ItemFoodData> finalItems;
                    ItemJsonParser::ParseFoodFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Drink)
                {
                    std::vector<ItemDrinkData> finalItems;
                    ItemJsonParser::ParseDrinkFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Medicine)
                {
                    std::vector<ItemMedicineData> finalItems;
                    ItemJsonParser::ParseMedicineFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Material)
                {
                    std::vector<ItemMaterialData> finalItems;
                    ItemJsonParser::ParseMaterialFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Weapon)
                {
                    std::vector<ItemWeaponData> finalItems;
                    ItemJsonParser::ParseWeaponFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::WeaponComponent)
                {
                    std::vector<ItemWeaponComponentData> finalItems;
                    ItemJsonParser::ParseWeaponComponentFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Ammo)
                {
                    std::vector<ItemAmmoData> finalItems;
                    ItemJsonParser::ParseAmmoFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Armor)
                {
                    std::vector<ItemArmorData> finalItems;
                    ItemJsonParser::ParseArmorFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
                else if (args.itemType == ItemType::Clothing)
                {
                    std::vector<ItemClothingData> finalItems;
                    ItemJsonParser::ParseClothingFromJsonText(content, finalItems);
                    AppendIdsToRegistry(registryTypeSlug, finalItems);
                }
            }
        }
        
        std::cout << "\n[ItemGenerator] Parallel batch generation completed:\n";
        std::cout << "  Batches: " << numBatches << "\n";
        std::cout << "  Successful: " << totalSuccess << "\n";
        std::cout << "  Failed: " << totalFailed << "\n";
        std::cout << "  Duration: " << FormatSeconds(totalDuration) << "\n";
        
        return (totalFailed > 0) ? 1 : 0;
    }

    // Helper function for single batch generation (original logic)
    int GenerateWithLLM_SingleBatch(const CommandLineArgs& args, const std::set<std::string>& existingIds, bool useCustomPreset, const CustomPreset& customPreset)
    {
        std::string prompt;
        std::string jsonResponse;

        // Prepend user-defined additional prompt (profile/context) if provided
        // This ensures profile constraints are seen first by the LLM
        if (!args.additionalPrompt.empty())
        {
            prompt = "=== CHARACTER PROFILE & GENERATION CONSTRAINTS ===\n";
            prompt += args.additionalPrompt;
            prompt += "\n\n=== ITEM GENERATION PROMPT ===\n";
            std::cout << "[ItemGenerator] Added user-defined profile/context at the beginning of prompt.\n";
        }

        // Build prompt and get LLM response
        std::string generationTimestamp = GetCurrentTimestamp();
        int existingCount = static_cast<int>(existingIds.size());

        if (args.itemType == ItemType::Food)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildFoodJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildFoodJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Food JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Drink)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildDrinkJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildDrinkJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Drink JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Medicine)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildMedicineJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildMedicineJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Medicine JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Material)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildMaterialJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildMaterialJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Material JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Weapon)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildWeaponJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildWeaponJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Weapon JSON Response ===\n";
        }
        else if (args.itemType == ItemType::WeaponComponent)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildWeaponComponentJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildWeaponComponentJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Weapon Component JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Ammo)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildAmmoJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildAmmoJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Ammo JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Armor)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildArmorJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildArmorJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Armor JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Clothing)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildClothingJsonPrompt(args.params, customPreset, existingIds, args.modelName, generationTimestamp, existingCount);
            else
                prompt = PromptBuilder::BuildClothingJsonPrompt(args.params, args.preset, existingIds, args.modelName, generationTimestamp, existingCount);
            std::cout << "=== Ollama Clothing JSON Response ===\n";
        }
        else
        {
            std::cout << "[ItemGenerator] Unknown item type.\n";
            return 1;
        }

        // Use retry logic for more reliable LLM calls
        jsonResponse = OllamaClient::RunWithRetry(args.modelName, prompt, 0, 0);
        
        if (jsonResponse.empty())
        {
            std::cerr << "[ItemGenerator] Failed to get LLM response after retries.\n";
            return 1;
        }
        
        std::cout << jsonResponse << "\n";

        // Process response based on item type (unified with qualityResults)
        std::map<std::string, QualityChecker::QualityResult> qualityResults;
        
        if (args.itemType == ItemType::Food)
        {
            return ProcessLLMResponse_Food(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Drink)
        {
            return ProcessLLMResponse_Drink(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Medicine)
        {
            return ProcessLLMResponse_Medicine(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Material)
        {
            return ProcessLLMResponse_Material(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Weapon)
        {
            return ProcessLLMResponse_Weapon(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::WeaponComponent)
        {
            return ProcessLLMResponse_WeaponComponent(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Ammo)
        {
            return ProcessLLMResponse_Ammo(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Armor)
        {
            return ProcessLLMResponse_Armor(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }
        else if (args.itemType == ItemType::Clothing)
        {
            return ProcessLLMResponse_Clothing(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset, qualityResults);
        }

        return 1;
    }

    int GenerateBatch(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in Batch mode (Parallel).\n";
        std::cout << "[ItemGenerator] Batch items: " << args.batchItems.size() << "\n\n";

        if (args.batchItems.empty())
        {
            std::cerr << "[ItemGenerator] Error: No batch items specified.\n";
            return 1;
        }

        int totalSuccess = 0;
        int totalFailed = 0;
        std::vector<BatchRunResult> batchResults;
        std::map<ItemType, int> successCountsByType;
        auto batchStart = std::chrono::steady_clock::now();

        // Always use parallel processing for batch mode
        if (false)  // Sequential mode disabled
        {
            // Sequential processing (for performance comparison)
            for (size_t i = 0; i < args.batchItems.size(); ++i)
            {
                const auto& batchItem = args.batchItems[i];
                
                std::cout << "\n";
                std::cout << "========================================\n";
                std::cout << "  Batch Item " << (i + 1) << " / " << args.batchItems.size() << " (Sequential)\n";
                std::cout << "  Type: " << CommandLineParser::GetItemTypeName(batchItem.itemType) << "\n";
                std::cout << "  Count: " << batchItem.count << "\n";
                std::cout << "========================================\n\n";

                // Create a new CommandLineArgs for this batch item
                CommandLineArgs itemArgs = args;
                itemArgs.itemType = batchItem.itemType;
                itemArgs.params.count = batchItem.count;
                itemArgs.mode = RunMode::LLM; // Always use LLM for batch items
                
                // Use custom output path if provided, otherwise generate default
                if (!batchItem.outputPath.empty())
                {
                    itemArgs.params.outputPath = batchItem.outputPath;
                }
                else
                {
                    // Generate default output path based on item type and test mode
                    std::string itemTypeName = CommandLineParser::GetItemTypeName(batchItem.itemType);
                    std::string baseDir = args.useTestMode ? "Test" : "ItemJson";
                    itemArgs.params.outputPath = baseDir + "/items_" + itemTypeName + ".json";
                }

                std::cout << "[ItemGenerator] Output: " << itemArgs.params.outputPath << "\n\n";

                auto itemStart = std::chrono::steady_clock::now();
                int result = GenerateWithLLM(itemArgs);
                auto itemEnd = std::chrono::steady_clock::now();
                double durationSeconds = std::chrono::duration<double>(itemEnd - itemStart).count();

                BatchRunResult batchResult;
                batchResult.item = batchItem;
                batchResult.outputPath = itemArgs.params.outputPath;
                batchResult.success = (result == 0);
                batchResult.exitCode = result;
                batchResult.durationSeconds = durationSeconds;
                batchResults.push_back(batchResult);
                
                if (result == 0)
                {
                    totalSuccess++;
                    successCountsByType[batchItem.itemType] += batchItem.count;
                    std::cout << "[ItemGenerator] OK Successfully generated " << batchItem.count 
                        << " " << CommandLineParser::GetItemTypeName(batchItem.itemType)
                        << " items (" << FormatSeconds(durationSeconds) << ").\n";
                }
                else
                {
                    totalFailed++;
                    std::cerr << "[ItemGenerator] FAIL Failed to generate " << batchItem.count
                        << " " << CommandLineParser::GetItemTypeName(batchItem.itemType)
                        << " items (exit code " << result << ").\n";
                }

                double progress = static_cast<double>(i + 1) / static_cast<double>(args.batchItems.size());
                std::cout << std::fixed << std::setprecision(1);
                std::cout << "[Batch] Progress: " << (i + 1) << "/" << args.batchItems.size()
                    << " (" << (progress * 100.0) << "%)\n";
                std::cout.unsetf(std::ios::floatfield);

                if (!batchResult.success)
                {
                    std::cerr << "[Batch] Continuing despite failure. You can rerun this item later.\n";
                }

                std::cout << "\n";
            }
        }
        else
        {
            // Parallel processing (default)
            // Mutex for thread-safe output and result collection
            std::mutex outputMutex;
            std::mutex resultsMutex;

            // Launch all batch items in parallel using async
            std::vector<std::future<BatchRunResult>> futures;
        
        for (size_t i = 0; i < args.batchItems.size(); ++i)
        {
            const auto& batchItem = args.batchItems[i];
            
            // Create a new CommandLineArgs for this batch item
            CommandLineArgs itemArgs = args;
            itemArgs.itemType = batchItem.itemType;
            itemArgs.params.count = batchItem.count;
            itemArgs.mode = RunMode::LLM; // Always use LLM for batch items
            
            // Use custom output path if provided, otherwise generate default
            if (!batchItem.outputPath.empty())
            {
                itemArgs.params.outputPath = batchItem.outputPath;
            }
            else
            {
                // Generate default output path based on item type
                std::string itemTypeName = CommandLineParser::GetItemTypeName(batchItem.itemType);
                itemArgs.params.outputPath = "ItemJson/items_" + itemTypeName + ".json";
            }

            // Launch async task for this batch item
            auto future = std::async(std::launch::async, [itemArgs, batchItem, i, &args, &outputMutex]() -> BatchRunResult {
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << "\n";
                    std::cout << "========================================\n";
                    std::cout << "  Batch Item " << (i + 1) << " / " << args.batchItems.size() << " (Parallel)\n";
                    std::cout << "  Type: " << CommandLineParser::GetItemTypeName(batchItem.itemType) << "\n";
                    std::cout << "  Count: " << batchItem.count << "\n";
                    std::cout << "========================================\n\n";
                    std::cout << "[ItemGenerator] Output: " << itemArgs.params.outputPath << "\n\n";
                }

                auto itemStart = std::chrono::steady_clock::now();
                int result = GenerateWithLLM(itemArgs);
                auto itemEnd = std::chrono::steady_clock::now();
                double durationSeconds = std::chrono::duration<double>(itemEnd - itemStart).count();

                BatchRunResult batchResult;
                batchResult.item = batchItem;
                batchResult.outputPath = itemArgs.params.outputPath;
                batchResult.success = (result == 0);
                batchResult.exitCode = result;
                batchResult.durationSeconds = durationSeconds;

                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    if (result == 0)
                    {
                        std::cout << "[ItemGenerator] OK Successfully generated " << batchItem.count 
                            << " " << CommandLineParser::GetItemTypeName(batchItem.itemType)
                            << " items (" << FormatSeconds(durationSeconds) << ").\n";
                    }
                    else
                    {
                        std::cerr << "[ItemGenerator] FAIL Failed to generate " << batchItem.count
                            << " " << CommandLineParser::GetItemTypeName(batchItem.itemType)
                            << " items (exit code " << result << ").\n";
                    }
                }

                return batchResult;
            });

            futures.push_back(std::move(future));
        }

            // Wait for all tasks to complete and collect results
            std::cout << "\n[Batch] Waiting for all parallel tasks to complete...\n\n";
            
            for (size_t i = 0; i < futures.size(); ++i)
            {
                BatchRunResult batchResult = futures[i].get();
                batchResults.push_back(batchResult);
                
                {
                    std::lock_guard<std::mutex> lock(resultsMutex);
                    if (batchResult.success)
                    {
                        totalSuccess++;
                        successCountsByType[batchResult.item.itemType] += batchResult.item.count;
                    }
                    else
                    {
                        totalFailed++;
                        std::cerr << "[Batch] Continuing despite failure. You can rerun this item later.\n";
                    }
                }

                double progress = static_cast<double>(i + 1) / static_cast<double>(args.batchItems.size());
                std::cout << std::fixed << std::setprecision(1);
                std::cout << "[Batch] Progress: " << (i + 1) << "/" << args.batchItems.size()
                    << " (" << (progress * 100.0) << "%)\n";
                std::cout.unsetf(std::ios::floatfield);
                std::cout << "\n";
            }
        }

        // Summary
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  BATCH GENERATION SUMMARY\n";
        std::cout << "========================================\n";
        auto batchEnd = std::chrono::steady_clock::now();
        double totalDuration = std::chrono::duration<double>(batchEnd - batchStart).count();

        std::cout << "  Total Items: " << args.batchItems.size() << "\n";
        std::cout << "  Successful:  " << totalSuccess << "\n";
        std::cout << "  Failed:      " << totalFailed << "\n";
        std::cout << "  Duration:    " << FormatSeconds(totalDuration) << "\n";
        std::cout << "----------------------------------------\n";
        std::cout << "  Items Generated per Type\n";
        if (successCountsByType.empty())
        {
            std::cout << "    (none)\n";
        }
        else
        {
            for (const auto& entry : successCountsByType)
            {
                std::cout << "    - " << CommandLineParser::GetItemTypeName(entry.first)
                    << ": " << entry.second << "\n";
            }
        }
        std::cout << "----------------------------------------\n";
        std::cout << "  Detailed Results\n";
        if (batchResults.empty())
        {
            std::cout << "    (no items)\n";
        }
        else
        {
            for (size_t i = 0; i < batchResults.size(); ++i)
            {
                const auto& entry = batchResults[i];
                std::cout << "    [" << (i + 1) << "] "
                    << CommandLineParser::GetItemTypeName(entry.item.itemType)
                    << " x" << entry.item.count
                    << " -> " << entry.outputPath
                    << " | " << (entry.success ? "OK" : "FAIL")
                    << " | " << FormatSeconds(entry.durationSeconds) << "\n";
            }
        }
        std::cout << "========================================\n\n";

        return (totalFailed > 0) ? 1 : 0;
    }
}
