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
#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <map>

static bool SaveTextFile(const std::string& path, const std::string& text)
{
    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[SaveTextFile] Failed to open file: " << path << "\n";
        return false;
    }
    ofs << text;
    return true;
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
    if (!ItemJsonParser::ParseFoodFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Items From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckFoodQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
    }

    for (const auto& item : items)
    {
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

    // If we still need more items, generate additional ones with updated exclusion list
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        // Get updated existing IDs (including newly added ones)
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingFoodIds(outputPath);
        
        // Build prompt with exclusion list
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildFoodJsonPrompt(additionalParams, preset, updatedExistingIds);
        
        // Generate additional items
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 3, 120);
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
    if (!ItemJsonParser::ParseDrinkFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Items From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckDrinkQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
    }

    for (const auto& item : items)
    {
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

    // If we still need more items, generate additional ones with updated exclusion list
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        // Get updated existing IDs (including newly added ones)
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingDrinkIds(outputPath);
        
        // Build prompt with exclusion list
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildDrinkJsonPrompt(additionalParams, preset, updatedExistingIds);
        
        // Generate additional items
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 3, 120);
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
    if (!ItemJsonParser::ParseMaterialFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse material JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Material Items (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckMaterialQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
    }

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

    // If we still need more items, generate additional ones with updated exclusion list
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        
        // Get updated existing IDs (including newly added ones)
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingMaterialIds(outputPath);
        
        // Build prompt with exclusion list
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildMaterialJsonPrompt(additionalParams, preset, updatedExistingIds);
        
        // Generate additional items
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 3, 120);
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
    if (!ItemJsonParser::ParseWeaponFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Weapons From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckWeaponQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
    }

    for (const auto& item : items)
    {
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

    // If we still need more items, generate additional ones
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingWeaponIds(outputPath);
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildWeaponJsonPrompt(additionalParams, preset, updatedExistingIds);
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 3, 120);
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
    if (!ItemJsonParser::ParseWeaponComponentFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Weapon Components From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckWeaponComponentQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
    }

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

    // If we still need more items, generate additional ones
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingWeaponComponentIds(outputPath);
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildWeaponComponentJsonPrompt(additionalParams, preset, updatedExistingIds);
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 3, 120);
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
    if (!ItemJsonParser::ParseAmmoFromJsonText(jsonResponse, items))
    {
        std::cout << "[ItemGenerator] Failed to parse LLM JSON.\n";
        return 1;
    }

    std::cout << "=== Parsed Ammo From LLM (" << items.size() << ") ===\n";
    
    // Quality check before filtering
    std::cout << "\n=== Quality Check ===\n";
    for (const auto& item : items)
    {
        auto qualityResult = QualityChecker::CheckAmmoQuality(item);
        QualityChecker::PrintQualityResult(qualityResult, item.id);
    }

    for (const auto& item : items)
    {
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

    // If we still need more items, generate additional ones
    if (stillNeeded > 0)
    {
        std::cout << "[ItemGenerator] Still need " << stillNeeded << " more items. Generating additional items...\n";
        std::set<std::string> updatedExistingIds = ItemJsonWriter::GetExistingAmmoIds(outputPath);
        FoodGenerateParams additionalParams = params;
        additionalParams.count = stillNeeded;
        std::string additionalPrompt = PromptBuilder::BuildAmmoJsonPrompt(additionalParams, preset, updatedExistingIds);
        std::string additionalResponse = OllamaClient::RunWithRetry(modelName, additionalPrompt, 3, 120);
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
        if (args.itemType == ItemType::Food)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildFoodJsonPrompt(args.params, customPreset, existingIds);
            else
                prompt = PromptBuilder::BuildFoodJsonPrompt(args.params, args.preset, existingIds);
            std::cout << "=== Ollama Food JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Drink)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildDrinkJsonPrompt(args.params, customPreset, existingIds);
            else
                prompt = PromptBuilder::BuildDrinkJsonPrompt(args.params, args.preset, existingIds);
            std::cout << "=== Ollama Drink JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Material)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildMaterialJsonPrompt(args.params, customPreset, existingIds);
            else
                prompt = PromptBuilder::BuildMaterialJsonPrompt(args.params, args.preset, existingIds);
            std::cout << "=== Ollama Material JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Weapon)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildWeaponJsonPrompt(args.params, customPreset, existingIds);
            else
                prompt = PromptBuilder::BuildWeaponJsonPrompt(args.params, args.preset, existingIds);
            std::cout << "=== Ollama Weapon JSON Response ===\n";
        }
        else if (args.itemType == ItemType::WeaponComponent)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildWeaponComponentJsonPrompt(args.params, customPreset, existingIds);
            else
                prompt = PromptBuilder::BuildWeaponComponentJsonPrompt(args.params, args.preset, existingIds);
            std::cout << "=== Ollama Weapon Component JSON Response ===\n";
        }
        else if (args.itemType == ItemType::Ammo)
        {
            if (useCustomPreset)
                prompt = PromptBuilder::BuildAmmoJsonPrompt(args.params, customPreset, existingIds);
            else
                prompt = PromptBuilder::BuildAmmoJsonPrompt(args.params, args.preset, existingIds);
            std::cout << "=== Ollama Ammo JSON Response ===\n";
        }
        else
        {
            std::cout << "[ItemGenerator] Unknown item type.\n";
            return 1;
        }

        // Use retry logic for more reliable LLM calls
        jsonResponse = OllamaClient::RunWithRetry(args.modelName, prompt, 3, 120);
        
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

    static std::vector<ItemFoodData> GenerateDummyFood(const FoodGenerateParams& params)
    {
        std::vector<ItemFoodData> items;
        items.reserve(params.count);

        for (int i = 0; i < params.count; ++i)
        {
            ItemFoodData item;
            item.id = "Food_item_dummy_food_" + std::to_string(i);
            item.displayName = "Dummy Food " + std::to_string(i);
            item.category = "Food";
            item.rarity = "Common";
            item.maxStack = 10;

            item.hungerRestore = 10 + i;
            item.thirstRestore = 5;
            item.healthRestore = 0;

            item.spoils = true;
            item.spoilTimeMinutes = 30;

            item.description = "Placeholder food item generated without LLM.";

            items.push_back(item);
        }

        return items;
    }

    static std::vector<ItemDrinkData> GenerateDummyDrink(const FoodGenerateParams& params)
    {
        std::vector<ItemDrinkData> items;
        items.reserve(params.count);

        for (int i = 0; i < params.count; ++i)
        {
            ItemDrinkData item;
            item.id = "Drink_item_dummy_drink_" + std::to_string(i);
            item.displayName = "Dummy Drink " + std::to_string(i);
            item.category = "Drink";
            item.rarity = "Common";
            item.maxStack = 10;

            item.hungerRestore = 0;
            item.thirstRestore = 15 + i;
            item.healthRestore = 0;

            item.spoils = false;
            item.spoilTimeMinutes = 0;

            item.description = "Placeholder drink item generated without LLM.";

            items.push_back(item);
        }

        return items;
    }

    static std::vector<ItemMaterialData> GenerateDummyMaterial(const FoodGenerateParams& params)
    {
        std::vector<ItemMaterialData> items;
        items.reserve(params.count);

        for (int i = 0; i < params.count; ++i)
        {
            ItemMaterialData item;
            item.id = "Material_item_dummy_material_" + std::to_string(i);
            item.displayName = "Dummy Material " + std::to_string(i);
            item.category = "Material";
            item.rarity = "Common";
            item.maxStack = 50;

            item.materialType = "Unknown";
            item.hardness = 10 + i;
            item.flammability = 20 + i;
            item.value = 5 + i;

            item.description = "Placeholder material item generated without LLM.";

            items.push_back(item);
        }

        return items;
    }

    int GenerateDummy(const CommandLineArgs& args)
    {
        std::cout << "[ItemGenerator] Running in Dummy mode.\n";

        if (args.itemType == ItemType::Food)
        {
            std::vector<ItemFoodData> items = GenerateDummyFood(args.params);

            std::cout << "=== Generated Dummy Food Items (" << items.size() << ") ===\n";
            for (const auto& item : items)
            {
                std::cout << "- " << item.displayName
                    << " (Hunger +" << item.hungerRestore
                    << ", Thirst +" << item.thirstRestore << ")\n";
            }

            if (!args.params.outputPath.empty())
            {
                if (!ItemJsonWriter::WriteFoodToFile(items, args.params.outputPath))
                {
                    std::cout << "[ItemGenerator] Failed to write dummy JSON file.\n";
                    return 1;
                }
            }
        }
        else if (args.itemType == ItemType::Drink)
        {
            std::vector<ItemDrinkData> items = GenerateDummyDrink(args.params);

            std::cout << "=== Generated Dummy Drink Items (" << items.size() << ") ===\n";
            for (const auto& item : items)
            {
                std::cout << "- " << item.displayName
                    << " (Hunger +" << item.hungerRestore
                    << ", Thirst +" << item.thirstRestore << ")\n";
            }

            if (!args.params.outputPath.empty())
            {
                if (!ItemJsonWriter::WriteDrinkToFile(items, args.params.outputPath))
                {
                    std::cout << "[ItemGenerator] Failed to write dummy JSON file.\n";
                    return 1;
                }
            }
        }
        else if (args.itemType == ItemType::Material)
        {
            std::vector<ItemMaterialData> items = GenerateDummyMaterial(args.params);

            std::cout << "=== Generated Dummy Material Items (" << items.size() << ") ===\n";
            for (const auto& item : items)
            {
                std::cout << "- " << item.displayName
                    << " (value: " << item.value
                    << ", hardness: " << item.hardness << ")\n";
            }

            if (!args.params.outputPath.empty())
            {
                if (!ItemJsonWriter::WriteMaterialToFile(items, args.params.outputPath))
                {
                    std::cout << "[ItemGenerator] Failed to write dummy JSON file.\n";
                    return 1;
                }
            }
        }
        else
        {
            std::cout << "[ItemGenerator] Unknown item type.\n";
            return 1;
        }

        return 0;
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
                itemArgs.params.outputPath = "items_" + itemTypeName + ".json";
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
                std::cout << "[ItemGenerator] ✓ Successfully generated " << batchItem.count 
                    << " " << CommandLineParser::GetItemTypeName(batchItem.itemType)
                    << " items (" << FormatSeconds(durationSeconds) << ").\n";
            }
            else
            {
                totalFailed++;
                std::cerr << "[ItemGenerator] ✗ Failed to generate " << batchItem.count 
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


