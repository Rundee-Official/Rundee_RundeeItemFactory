// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemGenerator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of high-level item generation.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Helpers/ItemGenerator.h"
#include "Helpers/CommandLineParser.h"
#include "Helpers/QualityChecker.h"
#include "Parsers/ItemJsonParser.h"
#include "Writers/ItemJsonWriter.h"
#include "Prompts/PromptBuilder.h"
#include "Prompts/CustomPreset.h"
#include "Clients/OllamaClient.h"
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
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace
{
    const std::vector<std::string> kBannedWords = { "dummy", "lorem", "ipsum", "placeholder", "test item", "badword" };
    const std::string kRegistryDir = "TempState";

    std::string ToLower(const std::string& s)
    {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    }

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

    std::map<std::string, int> CountRarity(const std::vector<std::string>& rarities)
    {
        std::map<std::string, int> counts;
        for (const auto& r : rarities)
            ++counts[r];
        return counts;
    }

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

// ------------------------------------------------------------
// ID Registry (cross-run dedupe)
// ------------------------------------------------------------
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

static void LogRegistryEvent(const std::string& typeName, size_t beforeCount, size_t addedCount, size_t afterCount)
{
    // #region agent log
    static int dbgCount = 0;
    if (dbgCount < 40)
    {
        ++dbgCount;
        auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
        if (dbg.is_open())
        {
            dbg << R"({"sessionId":"debug-session","runId":"dedupe-registry","hypothesisId":"H_REG","location":"ItemGenerator.cpp","message":"registry update","data":{"type":")"
                << typeName << R"(","before":)" << beforeCount << R"(,"added":)" << addedCount << R"(,"after":)" << afterCount << R"(},"timestamp":)" << ts << "})" << "\n";
        }
    }
    // #endregion
}

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

static int ProcessLLMResponse_Food(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset)
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

    std::vector<ItemFoodData> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!ItemJsonParser::ParseFoodFromJsonText(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Items From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckFoodQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary("Food", warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

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

    // Filter out duplicates (should be rare since we told LLM to avoid them, but just in case)
    std::vector<ItemFoodData> newItems;
    std::set<std::string> newIds = excludeIds; // Start with existing IDs
    for (const auto& item : items)
    {
        if (newIds.find(item.id) == newIds.end())
        {
            newItems.push_back(item);
            newIds.insert(item.id);
        }
        else
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
        }
    }

    int addedCount = static_cast<int>(newItems.size());
    int skippedCount = static_cast<int>(items.size()) - addedCount;

    if (skippedCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << skippedCount << " duplicate items (LLM generated duplicates despite exclusion list).\n";
    }

    // Check if we need more items
    // requiredCount is the maximum number of NEW items to add (not total count)
    // Limit newItems to requiredCount if we got more than requested
    int actualAddedCount = addedCount;
    if (addedCount > requiredCount)
    {
        std::cout << "[ItemGenerator] Generated " << addedCount << " items but only " << requiredCount << " requested. Limiting to " << requiredCount << " items.\n";
        newItems.resize(requiredCount);
        actualAddedCount = requiredCount;
    }
    
    int stillNeeded = requiredCount - actualAddedCount;

    // Write new items (merge with existing if file exists)
    std::set<std::string> currentExistingIds = ItemJsonWriter::GetExistingFoodIds(outputPath);
    if (!currentExistingIds.empty())
    {
        // Read existing items and merge
        std::vector<ItemFoodData> existingItems;
        std::ifstream ifs(outputPath);
        if (ifs.is_open())
        {
            std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            if (!existingContent.empty())
            {
                ItemJsonParser::ParseFoodFromJsonText(existingContent, existingItems);
            }
        }
        
        // Combine existing and new (only add non-duplicate new items)
        std::vector<ItemFoodData> allItems = existingItems;
        std::set<std::string> existingIdSet;
        for (const auto& item : existingItems)
        {
            existingIdSet.insert(item.id);
        }
        
        // Only add new items that don't already exist
        for (const auto& newItem : newItems)
        {
            if (existingIdSet.find(newItem.id) == existingIdSet.end())
            {
                allItems.push_back(newItem);
                existingIdSet.insert(newItem.id);
            }
        }
        
        if (!ItemJsonWriter::WriteFoodToFile(allItems, outputPath))
        {
            std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
            return 1;
        }
        
        std::cout << "[ItemGenerator] Merged " << existingItems.size() << " existing items with " << newItems.size() << " new items.\n";
    }
    else
    {
        // No existing file, just write new items
        if (!ItemJsonWriter::WriteFoodToFile(newItems, outputPath))
        {
            std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
            return 1;
        }
    }

    // Append registry for this batch before potentially recursing for more
    AppendIdsToRegistry("food", newItems);

    // If we still need more items, generate additional ones with updated exclusion list
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        // Get updated existing IDs (including newly added ones)
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingFoodIds(outputPath);
        
        // Build prompt with exclusion list
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildFoodJsonPrompt(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        
        // Generate additional items
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            return ProcessLLMResponse_Food(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset);
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << actualAddedCount << " new food items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_Drink(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset)
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

    std::vector<ItemDrinkData> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!ItemJsonParser::ParseDrinkFromJsonText(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Items From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckDrinkQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary("Drink", warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

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

    // Filter out duplicates (should be rare since we told LLM to avoid them, but just in case)
    std::vector<ItemDrinkData> newItems;
    std::set<std::string> newIds = excludeIds; // Start with existing IDs
    for (const auto& item : items)
    {
        if (newIds.find(item.id) == newIds.end())
        {
            newItems.push_back(item);
            newIds.insert(item.id);
        }
        else
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
        }
    }

    int addedCount = static_cast<int>(newItems.size());
    int skippedCount = static_cast<int>(items.size()) - addedCount;

    if (skippedCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << skippedCount << " duplicate items (LLM generated duplicates despite exclusion list).\n";
    }

    // Check if we need more items
    int stillNeeded = requiredCount - addedCount;

    // Merge or write
    std::set<std::string> currentExistingIds = ItemJsonWriter::GetExistingDrinkIds(outputPath);
    if (!currentExistingIds.empty() || !newItems.empty())
    {
        if (!currentExistingIds.empty())
        {
            // Read existing items and merge
            std::vector<ItemDrinkData> existingItems;
            std::ifstream ifs(outputPath);
            if (ifs.is_open())
            {
                std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
                if (!existingContent.empty())
                {
                    ItemJsonParser::ParseDrinkFromJsonText(existingContent, existingItems);
                }
            }
            
            // Combine existing and new
            std::vector<ItemDrinkData> allItems = existingItems;
            allItems.insert(allItems.end(), newItems.begin(), newItems.end());
            
            if (!ItemJsonWriter::WriteDrinkToFile(allItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
        }
        else
        {
            if (!ItemJsonWriter::WriteDrinkToFile(newItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
                return 1;
            }
        }
    }

    // Append registry for this batch before potentially recursing for more
    AppendIdsToRegistry("drink", newItems);

    // If we still need more items, generate additional ones with updated exclusion list
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        // Get updated existing IDs (including newly added ones)
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingDrinkIds(outputPath);
        
        // Build prompt with exclusion list
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildDrinkJsonPrompt(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        
        // Generate additional items
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            return ProcessLLMResponse_Drink(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset);
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new drink items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_Material(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset)
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

    std::vector<ItemMaterialData> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!ItemJsonParser::ParseMaterialFromJsonText(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse material JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Material Items (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckMaterialQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary("Material", warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

    for (const auto& item : items)
    {
        std::cout << "- " << item.id
            << " / " << item.displayName
            << " / category: " << item.category
            << " / materialType: " << item.materialType
            << " / value: " << item.value
            << "\n";
    }

    // Filter out duplicates (should be rare since we told LLM to avoid them, but just in case)
    std::vector<ItemMaterialData> newItems;
    std::set<std::string> newIds = excludeIds; // Start with existing IDs
    for (const auto& item : items)
    {
        if (newIds.find(item.id) == newIds.end())
        {
            newItems.push_back(item);
            newIds.insert(item.id);
        }
        else
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
        }
    }

    int addedCount = static_cast<int>(newItems.size());
    int skippedCount = static_cast<int>(items.size()) - addedCount;

    if (skippedCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << skippedCount << " duplicate items (LLM generated duplicates despite exclusion list).\n";
    }

    // Check if we need more items
    int stillNeeded = requiredCount - addedCount;

    // Merge or write
    std::set<std::string> currentExistingIds = ItemJsonWriter::GetExistingMaterialIds(outputPath);
    if (!currentExistingIds.empty() || !newItems.empty())
    {
        if (!currentExistingIds.empty())
        {
            // Read existing items and merge
            std::vector<ItemMaterialData> existingItems;
            std::ifstream ifs(outputPath);
            if (ifs.is_open())
            {
                std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
                if (!existingContent.empty())
                {
                    ItemJsonParser::ParseMaterialFromJsonText(existingContent, existingItems);
                }
            }
            
            // Combine existing and new
            std::vector<ItemMaterialData> allItems = existingItems;
            allItems.insert(allItems.end(), newItems.begin(), newItems.end());
            
            if (!ItemJsonWriter::WriteMaterialToFile(allItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
        }
        else
        {
            if (!ItemJsonWriter::WriteMaterialToFile(newItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write material JSON file.\n";
                return 1;
            }
        }
    }

    // Append registry for this batch before potentially recursing for more
    AppendIdsToRegistry("material", newItems);

    // If we still need more items, generate additional ones with updated exclusion list
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        // Get updated existing IDs (including newly added ones)
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingMaterialIds(outputPath);
        
        // Build prompt with exclusion list
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildMaterialJsonPrompt(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        
        // Generate additional items
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            return ProcessLLMResponse_Material(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset);
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new material items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_Weapon(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset)
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

    std::vector<ItemWeaponData> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!ItemJsonParser::ParseWeaponFromJsonText(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Weapons From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckWeaponQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary("Weapon", warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

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

    // Filter out duplicates
    std::vector<ItemWeaponData> newItems;
    std::set<std::string> newIds = excludeIds;
    for (const auto& item : items)
    {
        if (newIds.find(item.id) == newIds.end())
        {
            newItems.push_back(item);
            newIds.insert(item.id);
        }
        else
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
        }
    }

    int addedCount = static_cast<int>(newItems.size());
    int stillNeeded = requiredCount - addedCount;

    // Merge or write
    std::set<std::string> currentExistingIds = ItemJsonWriter::GetExistingWeaponIds(outputPath);
    if (!currentExistingIds.empty() || !newItems.empty())
    {
        if (!currentExistingIds.empty())
        {
            std::vector<ItemWeaponData> existingItems;
            std::ifstream ifs(outputPath);
            if (ifs.is_open())
            {
                std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
                if (!existingContent.empty())
                {
                    ItemJsonParser::ParseWeaponFromJsonText(existingContent, existingItems);
                }
            }
            
            std::vector<ItemWeaponData> allItems = existingItems;
            allItems.insert(allItems.end(), newItems.begin(), newItems.end());
            
            if (!ItemJsonWriter::WriteWeaponToFile(allItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
        }
        else
        {
            if (!ItemJsonWriter::WriteWeaponToFile(newItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
                return 1;
            }
        }
    }

    // Append registry for this batch before potentially recursing for more
    AppendIdsToRegistry("weapon", newItems);

    // If we still need more items, generate additional ones
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingWeaponIds(outputPath);
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildWeaponJsonPrompt(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            return ProcessLLMResponse_Weapon(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset);
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new weapon items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_WeaponComponent(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset)
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

    std::vector<ItemWeaponComponentData> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!ItemJsonParser::ParseWeaponComponentFromJsonText(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Weapon Components From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckWeaponComponentQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary("WeaponComponent", warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

    for (const auto& item : items)
    {
        std::cout << "- " << item.id << " / " << item.displayName
            << " / " << item.componentType << "\n";
    }

    // Filter out duplicates
    std::vector<ItemWeaponComponentData> newItems;
    std::set<std::string> newIds = excludeIds;
    for (const auto& item : items)
    {
        if (newIds.find(item.id) == newIds.end())
        {
            newItems.push_back(item);
            newIds.insert(item.id);
        }
        else
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
        }
    }

    int addedCount = static_cast<int>(newItems.size());
    int stillNeeded = requiredCount - addedCount;

    // Merge or write
    std::set<std::string> currentExistingIds = ItemJsonWriter::GetExistingWeaponComponentIds(outputPath);
    if (!currentExistingIds.empty() || !newItems.empty())
    {
        if (!currentExistingIds.empty())
        {
            std::vector<ItemWeaponComponentData> existingItems;
            std::ifstream ifs(outputPath);
            if (ifs.is_open())
            {
                std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
                if (!existingContent.empty())
                {
                    ItemJsonParser::ParseWeaponComponentFromJsonText(existingContent, existingItems);
                }
            }
            
            std::vector<ItemWeaponComponentData> allItems = existingItems;
            allItems.insert(allItems.end(), newItems.begin(), newItems.end());
            
            if (!ItemJsonWriter::WriteWeaponComponentToFile(allItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
        }
        else
        {
            if (!ItemJsonWriter::WriteWeaponComponentToFile(newItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
                return 1;
            }
        }
    }

    // Append registry for this batch before potentially recursing for more
    AppendIdsToRegistry("weaponcomponent", newItems);

    // If we still need more items, generate additional ones
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingWeaponComponentIds(outputPath);
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildWeaponComponentJsonPrompt(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            return ProcessLLMResponse_WeaponComponent(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset);
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new weapon component items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_Ammo(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset)
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

    std::vector<ItemAmmoData> items;
    std::string cleanedJson = CleanJsonTrailingCommas(jsonResponse);
    if (!ItemJsonParser::ParseAmmoFromJsonText(cleanedJson, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Ammo From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    int warnCount = 0;
    int errorCount = 0;
    int banHits = 0;
    std::vector<std::string> rarities;
    rarities.reserve(items.size());

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckAmmoQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        warnCount += static_cast<int>(qualityResult.warnings.size());
        errorCount += static_cast<int>(qualityResult.errors.size());
        banHits += CountBanHits(item.id) + CountBanHits(item.displayName) + CountBanHits(item.description);
        rarities.push_back(item.rarity);
    }
    PrintGuardrailSummary("Ammo", warnCount, errorCount, banHits, CountRarity(rarities), static_cast<int>(rarities.size()));

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

    // Filter out duplicates
    std::vector<ItemAmmoData> newItems;
    std::set<std::string> newIds = excludeIds;
    for (const auto& item : items)
    {
        if (newIds.find(item.id) == newIds.end())
        {
            newItems.push_back(item);
            newIds.insert(item.id);
        }
        else
        {
            std::cout << "[ItemGenerator] Filtered out duplicate ID: " << item.id << "\n";
        }
    }

    int addedCount = static_cast<int>(newItems.size());
    int stillNeeded = requiredCount - addedCount;

    // Merge or write
    std::set<std::string> currentExistingIds = ItemJsonWriter::GetExistingAmmoIds(outputPath);
    if (!currentExistingIds.empty() || !newItems.empty())
    {
        if (!currentExistingIds.empty())
        {
            std::vector<ItemAmmoData> existingItems;
            std::ifstream ifs(outputPath);
            if (ifs.is_open())
            {
                std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                ifs.close();
                if (!existingContent.empty())
                {
                    ItemJsonParser::ParseAmmoFromJsonText(existingContent, existingItems);
                }
            }
            
            std::vector<ItemAmmoData> allItems = existingItems;
            allItems.insert(allItems.end(), newItems.begin(), newItems.end());
            
            if (!ItemJsonWriter::WriteAmmoToFile(allItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
        }
        else
        {
            if (!ItemJsonWriter::WriteAmmoToFile(newItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
                return 1;
            }
        }
    }

    // Append registry for this batch before potentially recursing for more
    AppendIdsToRegistry("ammo", newItems);

    // If we still need more items, generate additional ones
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingAmmoIds(outputPath);
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildAmmoJsonPrompt(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            return ProcessLLMResponse_Ammo(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset);
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new ammo items to " << outputPath << "\n";
    }

    return 0;
}

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

    int GenerateWithLLM(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in LLM mode.\n";

        std::string prompt;
        std::string jsonResponse;

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
        else
        {
            std::cout << "[ItemGenerator] Unknown item type.\n";
            return 1;
        }

        // Append user-defined additional prompt if provided
        if (!args.additionalPrompt.empty())
        {
            prompt += "\n\nAdditional User Instructions:\n";
            prompt += args.additionalPrompt;
            prompt += "\n";
            std::cout << "[ItemGenerator] Added user-defined additional prompt.\n";
        }

        // Use retry logic for more reliable LLM calls
        jsonResponse = OllamaClient::RunWithRetry(args.modelName, prompt, 0, 0);
        
        if (jsonResponse.empty())
        {
            std::cerr << "[ItemGenerator] Failed to get LLM response after retries.\n";
            return 1;
        }
        
        std::cout << jsonResponse << "\n";

        // Process response based on item type
        if (args.itemType == ItemType::Food)
        {
            return ProcessLLMResponse_Food(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset);
        }
        else if (args.itemType == ItemType::Drink)
        {
            return ProcessLLMResponse_Drink(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset);
        }
        else if (args.itemType == ItemType::Material)
        {
            return ProcessLLMResponse_Material(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset);
        }
        else if (args.itemType == ItemType::Weapon)
        {
            return ProcessLLMResponse_Weapon(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset);
        }
        else if (args.itemType == ItemType::WeaponComponent)
        {
            return ProcessLLMResponse_WeaponComponent(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset);
        }
        else if (args.itemType == ItemType::Ammo)
        {
            return ProcessLLMResponse_Ammo(jsonResponse, args.params.outputPath, args.params.count, existingIds, args.modelName, args.params, args.preset);
        }

        return 1;
    }

    int GenerateBatch(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in Batch mode.\n";
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

        for (size_t i = 0; i < args.batchItems.size(); ++i)
        {
            const auto& batchItem = args.batchItems[i];
            
            std::cout << "\n";
            std::cout << "========================================\n";
            std::cout << "  Batch Item " << (i + 1) << " / " << args.batchItems.size() << "\n";
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
                // Generate default output path based on item type
                std::string itemTypeName = CommandLineParser::GetItemTypeName(batchItem.itemType);
                itemArgs.params.outputPath = "ItemJson/items_" + itemTypeName + ".json";
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