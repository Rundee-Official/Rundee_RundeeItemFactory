// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemGenerator.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of high-level item generation.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Generators/ItemGenerator.h"
#include "Helpers/CommandLineParser.h"
#include "Helpers/QualityChecker.h"
#include "Parsers/ItemJsonParser.h"
#include "Writers/ItemJsonWriter.h"
#include "Prompts/PromptBuilder.h"
#include "Prompts/CustomPreset.h"
#include "Clients/OllamaClient.h"
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

// ============================================================================
// SECTION 3: ProcessLLMResponse Functions (Alphabetical Order)
// ============================================================================
// These functions process LLM JSON responses and generate item data
// All functions now use qualityResults for unified invalid item filtering

// ============================================================================
// SECTION 1: Forward Declarations
// ============================================================================
// All ProcessLLMResponse_* functions now use qualityResults for unified processing
static int ProcessLLMResponse_Ammo(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Armor(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Clothing(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Drink(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Food(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Material(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_Weapon(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);
static int ProcessLLMResponse_WeaponComponent(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults);

// ============================================================================
// SECTION 2: Common Template-Based Processing Logic
// ============================================================================
// REFACTORED: This template function extracts the common logic from ProcessLLMResponse_* functions
// Type-specific operations are passed as function objects/lambdas
// All item types now use qualityResults for unified invalid item filtering

template<typename ItemType, typename ParseFunc, typename QualityCheckFunc, typename WriteFunc, typename GetExistingIdsFunc, typename BuildPromptFunc, typename ProcessRecursiveFunc, typename PostProcessFunc = std::function<void(std::vector<ItemType>&)>>
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
    PostProcessFunc postProcessFunc = [](std::vector<ItemType>&) {})
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

    // Limit to requiredCount
    int actualAddedCount = addedCount;
    if (addedCount > requiredCount)
    {
        std::cout << "[ItemGenerator] Generated " << addedCount << " items but only " << requiredCount << " requested. Limiting to " << requiredCount << " items.\n";
        newItems.resize(requiredCount);
        actualAddedCount = requiredCount;
    }
    
    int stillNeeded = requiredCount - actualAddedCount;

    // Merge or write
    std::set<std::string> currentExistingIds = getExistingIdsFunc(outputPath);
    if (!currentExistingIds.empty())
    {
        std::vector<ItemType> existingItems;
        std::ifstream ifs(outputPath);
        if (ifs.is_open())
        {
            std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            if (!existingContent.empty())
            {
                parseFunc(existingContent, existingItems);
            }
        }
        
        std::vector<ItemType> allItems = existingItems;
        std::set<std::string> existingIdSet;
        for (const auto& item : existingItems)
        {
            existingIdSet.insert(item.id);
        }
        
        int addedToExisting = 0;
        for (const auto& newItem : newItems)
        {
            if (existingIdSet.find(newItem.id) == existingIdSet.end())
            {
                allItems.push_back(newItem);
                existingIdSet.insert(newItem.id);
                addedToExisting++;
            }
        }
        
        int totalAfterMerge = static_cast<int>(allItems.size());
        int originalCount = static_cast<int>(existingItems.size());
        int targetTotal = originalCount + requiredCount;
        if (totalAfterMerge > targetTotal)
        {
            std::cout << "[ItemGenerator] Warning: Total items (" << totalAfterMerge << ") exceeds target (" << targetTotal << "). Limiting to " << targetTotal << " items.\n";
            allItems.resize(targetTotal);
        }
        
        if (!writeFunc(allItems, outputPath))
        {
            std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
            return 1;
        }
        
        std::cout << "[ItemGenerator] Merged " << existingItems.size() << " existing items with " << addedToExisting << " new items (total: " << allItems.size() << ").\n";
    }
    else
    {
        if (!writeFunc(newItems, outputPath))
        {
            std::cout << "[ItemGenerator] Failed to write LLM-generated JSON file.\n";
            return 1;
        }
    }

    AppendIdsToRegistry(registrySlug, newItems);

    // Regeneration logic
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        std::set<std::string> updatedExistingIds = getExistingIdsFunc(outputPath);
        
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = buildPromptFunc(
            additionalParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 0, 0);
        if (!additionalResponse.empty())
        {
            int recurseResult = processRecursiveFunc(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset, qualityResults);
            if (recurseResult == 0)
            {
                std::set<std::string> finalIds = getExistingIdsFunc(outputPath);
                int finalCount = static_cast<int>(finalIds.size());
                int originalCount = static_cast<int>(updatedExistingIds.size());
                int actuallyAdded = finalCount - originalCount;
                
                if (actuallyAdded < stillNeeded)
                {
                    std::cout << "[ItemGenerator] Warning: Only added " << actuallyAdded << " of " << stillNeeded << " needed items. Attempting one more regeneration...\n";
                    int remaining = stillNeeded - actuallyAdded;
                    std::set<std::string> finalExistingIds = getExistingIdsFunc(outputPath);
                    FoodGenerateParams finalParams = params;
                    finalParams.count = remaining;
                    std::string finalPrompt = buildPromptFunc(
                        finalParams,
                        preset,
                        finalExistingIds,
                        modelName,
                        GetCurrentTimestamp(),
                        static_cast<int>(finalExistingIds.size()));
                    std::string finalResponse = OllamaClient::RunWithRetry(modelName, finalPrompt, 0, 0);
                    if (!finalResponse.empty())
                    {
                        return processRecursiveFunc(finalResponse, outputPath, remaining, finalExistingIds, modelName, finalParams, preset, qualityResults);
                    }
                }
            }
            return recurseResult;
        }
        else
        {
            std::cout << "[ItemGenerator] Warning: Failed to get additional response for " << stillNeeded << " more items.\n";
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << actualAddedCount << " new " << itemTypeName << " items to " << outputPath << "\n";
    }

    return 0;
}

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
        }
    );
}

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
        }
    );
}

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
        }
    );
}

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
        }
    );
}

static int ProcessLLMResponse_WeaponComponent(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
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
    // qualityResults is passed by reference and accumulated across batches

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckWeaponComponentQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        qualityResults[item.id] = qualityResult; // Store for filtering
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

    // Filter out duplicates and invalid items (isValid = false)
    std::vector<ItemWeaponComponentData> newItems;
    std::set<std::string> newIds = excludeIds;
    int filteredInvalidCount = 0;
    std::cout << "[ItemGenerator] DEBUG: Starting filter loop for " << items.size() << " items, qualityResults map size: " << qualityResults.size() << "\n";
    
    // First, collect all ERROR items BEFORE filtering
    std::vector<std::string> errorItemIds;
    std::vector<std::string> errorMessages;
    std::cout << "[ItemGenerator] DEBUG: Checking " << items.size() << " items against qualityResults map (size: " << qualityResults.size() << ")\n";
    for (const auto& item : items)
    {
        auto it = qualityResults.find(item.id);
        if (it != qualityResults.end())
        {
            if (!it->second.isValid)
            {
                errorItemIds.push_back(item.id);
                for (const auto& err : it->second.errors)
                {
                    errorMessages.push_back(err);
                }
                std::cout << "[ItemGenerator] ERROR item found: " << item.id << " (isValid=false, errors=" << it->second.errors.size() << ") (will be filtered/regenerated)\n";
            }
        }
        else
        {
            std::cout << "[ItemGenerator] WARNING: Quality result not found for item in error collection: " << item.id << "\n";
        }
    }
        
    if (!errorItemIds.empty())
    {
        std::cout << "[ItemGenerator] ERROR items detected BEFORE filtering: " << errorItemIds.size() << " items\n";
        std::cout << "[ItemGenerator] ERROR item IDs: ";
        for (const auto& id : errorItemIds)
        {
            std::cout << id << " ";
        }
        std::cout << "\n";
    }
    else
    {
        std::cout << "[ItemGenerator] No ERROR items detected in quality check results.\n";
    }
    
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
        else
        {
            // This should not happen, but log it for debugging
            std::cout << "[ItemGenerator] WARNING: Quality result not found for item: " << item.id << " (skipping quality filter)\n";
        }
        
        newItems.push_back(item);
        newIds.insert(item.id);
    }

    if (filteredInvalidCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << filteredInvalidCount << " invalid items (quality check failed).\n";
    }

    // errorItemIds and errorMessages are already collected above during filtering

    // CRITICAL: Remove ERROR items from newItems BEFORE regeneration
    // This ensures ERROR items are never saved, even if regeneration fails
    std::vector<ItemWeaponComponentData> validItemsOnly;
    for (const auto& item : newItems)
    {
        auto it = qualityResults.find(item.id);
        if (it != qualityResults.end() && !it->second.isValid)
        {
            std::cout << "[ItemGenerator] Removing ERROR item from newItems before regeneration: " << item.id << "\n";
            continue;
        }
        validItemsOnly.push_back(item);
    }
    newItems = validItemsOnly;

    // CRITICAL: If we have error items, regenerate them BEFORE saving anything
    // This ensures ERROR items are never saved to the file
    static int retryCountWeaponComponent = 0;
    const int MAX_RETRIES = 3;
    if (!errorItemIds.empty() && retryCountWeaponComponent < MAX_RETRIES)
    {
        retryCountWeaponComponent++;
        std::cout << "[ItemGenerator] ERROR items detected (" << errorItemIds.size() << " items). Regenerating with feedback (retry " << retryCountWeaponComponent << "/" << MAX_RETRIES << ")...\n";
        
        // Build feedback message
        std::stringstream feedback;
        feedback << "\n\nCRITICAL: Previous generation had ERRORS that caused items to be REJECTED:\n";
        feedback << "- " << errorItemIds.size() << " items were rejected due to quality check failures\n";
        feedback << "Common errors found:\n";
        std::set<std::string> uniqueErrors(errorMessages.begin(), errorMessages.end());
        for (const auto& err : uniqueErrors)
        {
            feedback << "  * " << err << "\n";
        }
        feedback << "\nYou MUST fix these issues in the new generation:\n";
        feedback << "1. EVERY component MUST have at least 2-3 non-zero stat modifiers (weightModifier alone is NOT enough)\n";
        feedback << "2. Components with ALL stat modifiers at 0 will be REJECTED\n";
        feedback << "3. Magazine components MUST have at least one non-weight modifier (effectiveRangeModifier, ergonomicsModifier, or recoilModifier)\n";
        feedback << "4. Verify ALL items meet the CRITICAL RULES before outputting JSON\n";
        feedback << "\nDO NOT repeat the same mistakes. Check each item carefully before including it in the JSON array.\n";

        // Regenerate with feedback
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingWeaponComponentIds(outputPath);
        FoodGenerateParams retryParams = params;
        retryParams.count = static_cast<int>(errorItemIds.size());
        std::string retryPrompt = PromptBuilder::BuildWeaponComponentJsonPrompt(
            retryParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        retryPrompt += feedback.str();
        
        std::string retryResponse = OllamaClient::RunWithRetry(modelName, retryPrompt, 0, 0);
        if (!retryResponse.empty())
        {
            // Reset retry count on successful regeneration attempt
            retryCountWeaponComponent = 0; // Reset for next batch
            // Pass the accumulated qualityResults to the recursive call
            int result = ProcessLLMResponse_WeaponComponent(retryResponse, outputPath, retryParams.count, updatedExistingIds, modelName, retryParams, preset, qualityResults);
            if (result == 0)
            {
                std::cout << "[ItemGenerator] Successfully regenerated ERROR items.\n";
            }
            return result;
        }
        else
        {
            retryCountWeaponComponent = 0; // Reset on failure
            std::cout << "[ItemGenerator] Failed to regenerate ERROR items (LLM response empty).\n";
            // ERROR items already removed from newItems, so we can continue with valid items only
        }
    }
    else if (retryCountWeaponComponent >= MAX_RETRIES)
    {
        std::cout << "[ItemGenerator] WARNING: Maximum retry count reached. ERROR items have been removed from newItems.\n";
        retryCountWeaponComponent = 0; // Reset for next batch
    }
    else
    {
        retryCountWeaponComponent = 0; // Reset if no errors
    }

    // CRITICAL: Final check - ensure no ERROR items in newItems before saving
    // This is a safety check in case regeneration logic failed
    if (!errorItemIds.empty())
    {
        std::cout << "[ItemGenerator] WARNING: ERROR items detected but regeneration failed or reached limit. Only valid items will be saved.\n";
        std::cout << "[ItemGenerator] ERROR item IDs that were removed: ";
        for (const auto& id : errorItemIds)
        {
            std::cout << id << " ";
        }
        std::cout << "\n";
    }

    // CRITICAL: Double-check that newItems does not contain any ERROR items
    // This is a safety check in case filtering logic failed
    std::vector<ItemWeaponComponentData> finalItems;
    for (const auto& item : newItems)
    {
        auto it = qualityResults.find(item.id);
        if (it != qualityResults.end() && !it->second.isValid)
        {
            std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in newItems after filtering: " << item.id << " - removing it!\n";
            continue;
        }
        finalItems.push_back(item);
    }
    newItems = finalItems;

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
            
            // CRITICAL: Filter out ERROR items from existing items before merging
            std::vector<ItemWeaponComponentData> validExistingItems;
            for (const auto& item : existingItems)
            {
                auto it = qualityResults.find(item.id);
                if (it != qualityResults.end() && !it->second.isValid)
                {
                    std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in existing items: " << item.id << " - removing it!\n";
                    continue;
                }
                validExistingItems.push_back(item);
            }
            
            std::vector<ItemWeaponComponentData> allItems = validExistingItems;
            std::set<std::string> existingIdSet;
            for (const auto& item : validExistingItems)
            {
                existingIdSet.insert(item.id);
            }
            
            // Only add new items that don't already exist
            int addedToExisting = 0;
            for (const auto& newItem : newItems)
            {
                if (existingIdSet.find(newItem.id) == existingIdSet.end())
                {
                    allItems.push_back(newItem);
                    existingIdSet.insert(newItem.id);
                    addedToExisting++;
                }
            }
            
            // CRITICAL: Final validation - ensure no ERROR items in allItems before writing
            std::vector<ItemWeaponComponentData> finalAllItems;
            for (const auto& item : allItems)
            {
                auto it = qualityResults.find(item.id);
                if (it != qualityResults.end() && !it->second.isValid)
                {
                    std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in allItems before writing: " << item.id << " - removing it!\n";
                    continue;
                }
                finalAllItems.push_back(item);
            }
            
            // Ensure total doesn't exceed what we need
            int totalAfterMerge = static_cast<int>(finalAllItems.size());
            int originalCount = static_cast<int>(validExistingItems.size());
            int targetTotal = originalCount + requiredCount;
            if (totalAfterMerge > targetTotal)
            {
                std::cout << "[ItemGenerator] Warning: Total items (" << totalAfterMerge << ") exceeds target (" << targetTotal << "). Limiting to " << targetTotal << " items.\n";
                finalAllItems.resize(targetTotal);
            }
            
            if (!ItemJsonWriter::WriteWeaponComponentToFile(finalAllItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
        }
        else
        {
            // CRITICAL: Final validation - ensure no ERROR items in newItems before writing
            std::vector<ItemWeaponComponentData> finalNewItems;
            for (const auto& item : newItems)
            {
                auto it = qualityResults.find(item.id);
                if (it != qualityResults.end() && !it->second.isValid)
                {
                    std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in newItems before writing: " << item.id << " - removing it!\n";
                    continue;
                }
                finalNewItems.push_back(item);
            }
            
            if (!ItemJsonWriter::WriteWeaponComponentToFile(finalNewItems, outputPath))
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
            // Pass the accumulated qualityResults to the recursive call
            int recurseResult = ProcessLLMResponse_WeaponComponent(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset, qualityResults);
            if (recurseResult == 0)
            {
                // Verify we actually got the needed items
                std::set<std::string> finalIds = ItemJsonWriter::GetExistingWeaponComponentIds(outputPath);
                int finalCount = static_cast<int>(finalIds.size());
                int originalCount = static_cast<int>(updatedExistingIds.size());
                int actuallyAdded = finalCount - originalCount;
                
                if (actuallyAdded < stillNeeded)
                {
                    std::cout << "[ItemGenerator] Warning: Only added " << actuallyAdded << " of " << stillNeeded << " needed items. Attempting one more regeneration...\n";
                    int remaining = stillNeeded - actuallyAdded;
                    std::set<std::string> finalExistingIds = ItemJsonWriter::GetExistingWeaponComponentIds(outputPath);
                    FoodGenerateParams finalParams = params;
                    finalParams.count = remaining;
                    std::string finalPrompt = PromptBuilder::BuildWeaponComponentJsonPrompt(
                        finalParams,
                        preset,
                        finalExistingIds,
                        modelName,
                        GetCurrentTimestamp(),
                        static_cast<int>(finalExistingIds.size()));
                    std::string finalResponse = OllamaClient::RunWithRetry(modelName, finalPrompt, 0, 0);
                    if (!finalResponse.empty())
                    {
                        return ProcessLLMResponse_WeaponComponent(finalResponse, outputPath, remaining, finalExistingIds, modelName, finalParams, preset, qualityResults);
                    }
                }
            }
            return recurseResult;
        }
        else
        {
            std::cout << "[ItemGenerator] Warning: Failed to get additional response for " << stillNeeded << " more items.\n";
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new weapon component items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_Ammo(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
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
    // qualityResults is passed by reference and accumulated across batches

    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckAmmoQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
        qualityResults[item.id] = qualityResult; // Store for filtering
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

    // First, fix value-related errors before filtering
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

    // Filter out duplicates and invalid items (isValid = false)
    std::vector<ItemAmmoData> newItems;
    std::set<std::string> newIds = excludeIds;
    int filteredInvalidCount = 0;
    std::cout << "[ItemGenerator] DEBUG: Starting filter loop for " << items.size() << " items, qualityResults map size: " << qualityResults.size() << "\n";
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
            std::cout << "[ItemGenerator] DEBUG: Found quality result for " << item.id << ", isValid=" << (it->second.isValid ? "true" : "false") << ", errors=" << it->second.errors.size() << "\n";
            if (!it->second.isValid)
            {
                std::cout << "[ItemGenerator] Filtered out invalid item (quality check failed): " << item.id << "\n";
                filteredInvalidCount++;
                continue;
            }
        }
        else
        {
            // This should not happen, but log it for debugging
            std::cout << "[ItemGenerator] WARNING: Quality result not found for item: " << item.id << " (skipping quality filter)\n";
        }
        
        newItems.push_back(item);
        newIds.insert(item.id);
    }
    
    if (filteredInvalidCount > 0)
    {
        std::cout << "[ItemGenerator] Filtered out " << filteredInvalidCount << " invalid items (quality check failed).\n";
    }

    // Collect critical error items (not value-related) for regeneration feedback
    std::vector<std::string> errorItemIds;
    std::vector<std::string> criticalErrorMessages;
    
    for (const auto& item : items)
    {
        auto it = qualityResults.find(item.id);
        if (it != qualityResults.end() && !it->second.isValid)
        {
            bool hasCriticalError = false;
            for (const auto& err : it->second.errors)
            {
                // Only collect non-value errors for regeneration
                if (err.find("value") == std::string::npos && err.find("Value") == std::string::npos)
                {
                    hasCriticalError = true;
                    criticalErrorMessages.push_back(err);
                }
            }
            
            if (hasCriticalError)
            {
                errorItemIds.push_back(item.id);
            }
        }
    }

    // If we have critical error items (not just value errors), regenerate them with feedback (max 3 retries)
    static int retryCountAmmo = 0;
    const int MAX_RETRIES = 3;
    if (!errorItemIds.empty() && retryCountAmmo < MAX_RETRIES)
    {
        retryCountAmmo++;
        std::cout << "[ItemGenerator] ERROR items detected (" << errorItemIds.size() << " items). Regenerating with feedback (retry " << retryCountAmmo << "/" << MAX_RETRIES << ")...\n";
        
        // Build feedback message (only for critical errors, not value errors)
        std::stringstream feedback;
        feedback << "\n\nCRITICAL: Previous generation had ERRORS that caused items to be REJECTED:\n";
        feedback << "- " << errorItemIds.size() << " items were rejected due to quality check failures\n";
        feedback << "Common errors found:\n";
        std::set<std::string> uniqueErrors(criticalErrorMessages.begin(), criticalErrorMessages.end());
        for (const auto& err : uniqueErrors)
        {
            feedback << "  * " << err << "\n";
        }
        feedback << "\nYou MUST fix these issues in the new generation:\n";
        feedback << "1. Common ammo MUST have damageBonus <= 5 AND penetration <= 40 (NO EXCEPTIONS)\n";
        feedback << "2. High-performance ammo MUST be Uncommon or Rare rarity (NOT Common)\n";
        feedback << "3. Value MUST match performance: Common (1-5), Uncommon (5-15), Rare (15-50)\n";
        feedback << "\nDO NOT repeat the same mistakes. Check each item carefully before including it in the JSON array.\n";

        // Regenerate with feedback
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingAmmoIds(outputPath);
        FoodGenerateParams retryParams = params;
        retryParams.count = static_cast<int>(errorItemIds.size());
        std::string retryPrompt = PromptBuilder::BuildAmmoJsonPrompt(
            retryParams,
            preset,
            updatedExistingIds,
            modelName,
            GetCurrentTimestamp(),
            static_cast<int>(updatedExistingIds.size()));
        retryPrompt += feedback.str();
        
        std::string retryResponse = OllamaClient::RunWithRetry(modelName, retryPrompt, 0, 0);
        if (!retryResponse.empty())
        {
            // Reset retry count on successful regeneration attempt
            retryCountAmmo = 0; // Reset for next batch
            // Pass the accumulated qualityResults to the recursive call
            int result = ProcessLLMResponse_Ammo(retryResponse, outputPath, retryParams.count, updatedExistingIds, modelName, retryParams, preset, qualityResults);
            if (result == 0)
            {
                std::cout << "[ItemGenerator] Successfully regenerated ERROR items.\n";
            }
            return result;
        }
        else
        {
            retryCountAmmo = 0; // Reset on failure
            std::cout << "[ItemGenerator] Failed to regenerate ERROR items (LLM response empty).\n";
        }
    }
    else if (retryCountAmmo >= MAX_RETRIES)
    {
        std::cout << "[ItemGenerator] WARNING: Maximum retry count reached. Some ERROR items may not be fixed.\n";
        retryCountAmmo = 0; // Reset for next batch
    }
    else
    {
        retryCountAmmo = 0; // Reset if no errors
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
            
            // CRITICAL: Filter out ERROR items from existing items before merging
            std::vector<ItemAmmoData> validExistingItems;
            for (const auto& item : existingItems)
            {
                auto it = qualityResults.find(item.id);
                if (it != qualityResults.end() && !it->second.isValid)
                {
                    std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in existing items: " << item.id << " - removing it!\n";
                    continue;
                }
                validExistingItems.push_back(item);
            }
            
            std::vector<ItemAmmoData> allItems = validExistingItems;
            std::set<std::string> existingIdSet;
            for (const auto& item : validExistingItems)
            {
                existingIdSet.insert(item.id);
            }
            
            // Only add new items that don't already exist
            int addedToExisting = 0;
            for (const auto& newItem : newItems)
            {
                if (existingIdSet.find(newItem.id) == existingIdSet.end())
                {
                    allItems.push_back(newItem);
                    existingIdSet.insert(newItem.id);
                    addedToExisting++;
                }
            }
            
            // CRITICAL: Final validation - ensure no ERROR items in allItems before writing
            std::vector<ItemAmmoData> finalAllItems;
            for (const auto& item : allItems)
            {
                auto it = qualityResults.find(item.id);
                if (it != qualityResults.end() && !it->second.isValid)
                {
                    std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in allItems before writing: " << item.id << " - removing it!\n";
                    continue;
                }
                finalAllItems.push_back(item);
            }
            
            // Ensure total doesn't exceed what we need
            int totalAfterMerge = static_cast<int>(finalAllItems.size());
            int originalCount = static_cast<int>(validExistingItems.size());
            int targetTotal = originalCount + requiredCount;
            if (totalAfterMerge > targetTotal)
            {
                std::cout << "[ItemGenerator] Warning: Total items (" << totalAfterMerge << ") exceeds target (" << targetTotal << "). Limiting to " << targetTotal << " items.\n";
                finalAllItems.resize(targetTotal);
            }
            
            if (!ItemJsonWriter::WriteAmmoToFile(finalAllItems, outputPath))
            {
                std::cout << "[ItemGenerator] Failed to write merged JSON file.\n";
                return 1;
            }
            
            std::cout << "[ItemGenerator] Merged " << validExistingItems.size() << " existing items with " << addedToExisting << " new items (total: " << finalAllItems.size() << ").\n";
        }
        else
        {
            // CRITICAL: Final validation - ensure no ERROR items in newItems before writing
            std::vector<ItemAmmoData> finalNewItems;
            for (const auto& item : newItems)
            {
                auto it = qualityResults.find(item.id);
                if (it != qualityResults.end() && !it->second.isValid)
                {
                    std::cout << "[ItemGenerator] CRITICAL: Found ERROR item in newItems before writing: " << item.id << " - removing it!\n";
                    continue;
                }
                finalNewItems.push_back(item);
            }
            
            if (!ItemJsonWriter::WriteAmmoToFile(finalNewItems, outputPath))
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
            // Pass the accumulated qualityResults to the recursive call
            int recurseResult = ProcessLLMResponse_Ammo(additionalResponse, outputPath, stillNeeded, updatedExistingIds, modelName, additionalParams, preset, qualityResults);
            if (recurseResult == 0)
            {
                // Verify we actually got the needed items
                std::set<std::string> finalIds = ItemJsonWriter::GetExistingAmmoIds(outputPath);
                int finalCount = static_cast<int>(finalIds.size());
                int originalCount = static_cast<int>(updatedExistingIds.size());
                int actuallyAdded = finalCount - originalCount;
                
                if (actuallyAdded < stillNeeded)
                {
                    std::cout << "[ItemGenerator] Warning: Only added " << actuallyAdded << " of " << stillNeeded << " needed items. Attempting one more regeneration...\n";
                    int remaining = stillNeeded - actuallyAdded;
                    std::set<std::string> finalExistingIds = ItemJsonWriter::GetExistingAmmoIds(outputPath);
                    FoodGenerateParams finalParams = params;
                    finalParams.count = remaining;
                    std::string finalPrompt = PromptBuilder::BuildAmmoJsonPrompt(
                        finalParams,
                        preset,
                        finalExistingIds,
                        modelName,
                        GetCurrentTimestamp(),
                        static_cast<int>(finalExistingIds.size()));
                    std::string finalResponse = OllamaClient::RunWithRetry(modelName, finalPrompt, 0, 0);
                    if (!finalResponse.empty())
                    {
                        return ProcessLLMResponse_Ammo(finalResponse, outputPath, remaining, finalExistingIds, modelName, finalParams, preset, qualityResults);
                    }
                }
            }
            return recurseResult;
        }
        else
        {
            std::cout << "[ItemGenerator] Warning: Failed to get additional response for " << stillNeeded << " more items.\n";
        }
    }
    else
    {
        std::cout << "[ItemGenerator] Successfully added " << addedCount << " new ammo items to " << outputPath << "\n";
    }

    return 0;
}

static int ProcessLLMResponse_Armor(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults, Armor has no quality check)
    return ProcessLLMResponse_Common<ItemArmorData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Armor", "armor",
        [](const std::string& json, std::vector<ItemArmorData>& out) { return ItemJsonParser::ParseArmorFromJsonText(json, out); },
        [](const ItemArmorData&) { 
            QualityChecker::QualityResult result;
            result.isValid = true;
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
        [](std::vector<ItemArmorData>&) {}
    );
}

static int ProcessLLMResponse_Clothing(const std::string& jsonResponse, const std::string& outputPath, int requiredCount, const std::set<std::string>& excludeIds, const std::string& modelName, const FoodGenerateParams& params, PresetType preset, std::map<std::string, QualityChecker::QualityResult>& qualityResults)
{
    // REFACTORED: Use common template function (unified with qualityResults, Clothing has no quality check)
    return ProcessLLMResponse_Common<ItemClothingData>(
        jsonResponse, outputPath, requiredCount, excludeIds, modelName, params, preset,
        "Clothing", "clothing",
        [](const std::string& json, std::vector<ItemClothingData>& out) { return ItemJsonParser::ParseClothingFromJsonText(json, out); },
        [](const ItemClothingData&) { 
            QualityChecker::QualityResult result;
            result.isValid = true;
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
        [](std::vector<ItemClothingData>&) {}
    );
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
        const int BATCH_SIZE = 10; // Items per batch
        const int MAX_CONCURRENT_BATCHES = 3; // Limit concurrent requests to avoid overwhelming Ollama
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
                // Remove completed futures
                futures.erase(
                    std::remove_if(futures.begin(), futures.end(),
                        [](std::future<int>& f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }),
                    futures.end());
                
                if (futures.size() >= MAX_CONCURRENT_BATCHES)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        std::set<std::string> finalExistingIds;
        if (args.itemType == ItemType::Food)
        {
            finalExistingIds = ItemJsonWriter::GetExistingFoodIds(args.params.outputPath);
        }
        else if (args.itemType == ItemType::Drink)
        {
            finalExistingIds = ItemJsonWriter::GetExistingDrinkIds(args.params.outputPath);
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
        int stillNeededAfterBatches = totalCount - finalCount;
        
        if (stillNeededAfterBatches > 0)
        {
            std::cout << "[ItemGenerator] After parallel batches: " << finalCount << " items generated, " << stillNeededAfterBatches << " still needed. Generating additional items...\n";
            
            // Generate remaining items
            CommandLineArgs additionalArgs = args;
            additionalArgs.params.count = stillNeededAfterBatches;
            int additionalResult = GenerateWithLLM_SingleBatch(additionalArgs, finalExistingIds, useCustomPreset, customPreset);
            
            if (additionalResult == 0)
            {
                // Verify final count
                std::set<std::string> verifyIds;
                if (args.itemType == ItemType::Food)
                {
                    verifyIds = ItemJsonWriter::GetExistingFoodIds(args.params.outputPath);
                }
                else if (args.itemType == ItemType::Drink)
                {
                    verifyIds = ItemJsonWriter::GetExistingDrinkIds(args.params.outputPath);
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
                
                int verifyCount = static_cast<int>(verifyIds.size());
                if (verifyCount < totalCount)
                {
                    std::cout << "[ItemGenerator] Warning: Still only " << verifyCount << " items after additional generation (target: " << totalCount << "). Attempting one more time...\n";
                    int remaining = totalCount - verifyCount;
                    CommandLineArgs finalArgs = args;
                    finalArgs.params.count = remaining;
                    GenerateWithLLM_SingleBatch(finalArgs, verifyIds, useCustomPreset, customPreset);
                }
                else
                {
                    std::cout << "[ItemGenerator] Successfully generated " << verifyCount << " items (target: " << totalCount << ").\n";
                }
            }
        }
        else if (finalCount > totalCount)
        {
            std::cout << "[ItemGenerator] Warning: Generated " << finalCount << " items (exceeds target: " << totalCount << ").\n";
        }
        else
        {
            std::cout << "[ItemGenerator] Successfully generated " << finalCount << " items (target: " << totalCount << ").\n";
        }
        
        auto batchEnd = std::chrono::steady_clock::now();
        double totalDuration = std::chrono::duration<double>(batchEnd - batchStart).count();
        
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
        std::cout << "[ItemGenerator] Running in Batch mode (" << (args.useSequentialBatch ? "Sequential" : "Parallel") << ").\n";
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

        if (args.useSequentialBatch)
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