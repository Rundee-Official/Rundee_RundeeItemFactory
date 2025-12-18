// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonWriter.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of unified JSON writer.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>

// Windows Platform Includes
#include <windows.h>

// Third-Party Includes
#include "json.hpp"
using nlohmann::json;

// Project Includes
#include "Parsers/ItemJsonParser.h"
#include "Writers/ItemJsonWriter.h"

// ============================================================================
// SECTION 1: Anonymous Namespace - Internal Helper Functions
// ============================================================================

namespace
{
    // Ensure directory exists for the given file path
    bool EnsureDirectoryExists(const std::string& filePath)
    {
        // Extract directory path from file path
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash == std::string::npos)
        {
            // No directory in path, file is in current directory
            return true;
        }

        std::string dirPath = filePath.substr(0, lastSlash);
        
        // Create directory recursively using Windows API
        std::wstring wDirPath(dirPath.begin(), dirPath.end());
        
        // CreateDirectory can only create one level at a time, so we need to create parent directories first
        size_t pos = 0;
        while ((pos = wDirPath.find_first_of(L"/\\", pos + 1)) != std::wstring::npos)
        {
            std::wstring subDir = wDirPath.substr(0, pos);
            CreateDirectoryW(subDir.c_str(), NULL);
        }
        
        // Create the final directory
        DWORD result = CreateDirectoryW(wDirPath.c_str(), NULL);
        return (result != 0 || GetLastError() == ERROR_ALREADY_EXISTS);
    }
}

bool ItemJsonWriter::WriteFoodToFile(const std::vector<ItemFoodData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;

        jItem["hungerRestore"] = item.hungerRestore;
        jItem["thirstRestore"] = item.thirstRestore;
        jItem["healthRestore"] = item.healthRestore;

        jItem["spoils"] = item.spoils;
        jItem["spoilTimeMinutes"] = item.spoilTimeMinutes;

        jItem["description"] = item.description;

        jArray.push_back(jItem);
    }

    // Ensure directory exists before writing
    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " food items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::WriteDrinkToFile(const std::vector<ItemDrinkData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;

        jItem["hungerRestore"] = item.hungerRestore;
        jItem["thirstRestore"] = item.thirstRestore;
        jItem["healthRestore"] = item.healthRestore;

        jItem["spoils"] = item.spoils;
        jItem["spoilTimeMinutes"] = item.spoilTimeMinutes;

        jItem["description"] = item.description;

        jArray.push_back(jItem);
    }

    // Ensure directory exists before writing
    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " drink items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::WriteMaterialToFile(const std::vector<ItemMaterialData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;

        jItem["materialType"] = item.materialType;
        jItem["hardness"] = item.hardness;
        jItem["flammability"] = item.flammability;
        jItem["value"] = item.value;

        jItem["description"] = item.description;

        jArray.push_back(jItem);
    }

    // Ensure directory exists before writing
    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);

    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " material items to JSON file: " << path << "\n";

    return true;
}

bool ItemJsonWriter::MergeFoodToFile(const std::vector<ItemFoodData>& newItems, const std::string& path)
{
    std::vector<ItemFoodData> existingItems;
    std::set<std::string> existingIds;

    // Read existing file if it exists
    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseFoodFromJsonText(existingContent, existingItems))
            {
                std::cout << "[ItemJsonWriter] Found " << existingItems.size()
                    << " existing food items in " << path << "\n";
                
                // Build set of existing IDs
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    // Merge: keep existing items, add new items that don't have duplicate IDs
    std::vector<ItemFoodData> mergedItems = existingItems;
    int addedCount = 0;
    int skippedCount = 0;

    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
            addedCount++;
        }
        else
        {
            skippedCount++;
            std::cout << "[ItemJsonWriter] Skipping duplicate food item ID: " << newItem.id << "\n";
        }
    }

    // Write merged items
    if (WriteFoodToFile(mergedItems, path))
    {
        std::cout << "[ItemJsonWriter] Merged food items: " << existingItems.size()
            << " existing + " << addedCount << " new = " << mergedItems.size()
            << " total (skipped " << skippedCount << " duplicates)\n";
        return true;
    }

    return false;
}

bool ItemJsonWriter::MergeDrinkToFile(const std::vector<ItemDrinkData>& newItems, const std::string& path)
{
    std::vector<ItemDrinkData> existingItems;
    std::set<std::string> existingIds;

    // Read existing file if it exists
    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseDrinkFromJsonText(existingContent, existingItems))
            {
                std::cout << "[ItemJsonWriter] Found " << existingItems.size()
                    << " existing drink items in " << path << "\n";
                
                // Build set of existing IDs
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    // Merge: keep existing items, add new items that don't have duplicate IDs
    std::vector<ItemDrinkData> mergedItems = existingItems;
    int addedCount = 0;
    int skippedCount = 0;

    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
            addedCount++;
        }
        else
        {
            skippedCount++;
            std::cout << "[ItemJsonWriter] Skipping duplicate drink item ID: " << newItem.id << "\n";
        }
    }

    // Write merged items
    if (WriteDrinkToFile(mergedItems, path))
    {
        std::cout << "[ItemJsonWriter] Merged drink items: " << existingItems.size()
            << " existing + " << addedCount << " new = " << mergedItems.size()
            << " total (skipped " << skippedCount << " duplicates)\n";
        return true;
    }

    return false;
}

bool ItemJsonWriter::MergeMaterialToFile(const std::vector<ItemMaterialData>& newItems, const std::string& path)
{
    std::vector<ItemMaterialData> existingItems;
    std::set<std::string> existingIds;

    // Read existing file if it exists
    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseMaterialFromJsonText(existingContent, existingItems))
            {
                std::cout << "[ItemJsonWriter] Found " << existingItems.size()
                    << " existing material items in " << path << "\n";
                
                // Build set of existing IDs
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    // Merge: keep existing items, add new items that don't have duplicate IDs
    std::vector<ItemMaterialData> mergedItems = existingItems;
    int addedCount = 0;
    int skippedCount = 0;

    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
            addedCount++;
        }
        else
        {
            skippedCount++;
            std::cout << "[ItemJsonWriter] Skipping duplicate material item ID: " << newItem.id << "\n";
        }
    }

    // Write merged items
    if (WriteMaterialToFile(mergedItems, path))
    {
        std::cout << "[ItemJsonWriter] Merged material items: " << existingItems.size()
            << " existing + " << addedCount << " new = " << mergedItems.size()
            << " total (skipped " << skippedCount << " duplicates)\n";
        return true;
    }

    return false;
}

std::set<std::string> ItemJsonWriter::GetExistingFoodIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemFoodData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseFoodFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

std::set<std::string> ItemJsonWriter::GetExistingDrinkIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemDrinkData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseDrinkFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

std::set<std::string> ItemJsonWriter::GetExistingMaterialIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemMaterialData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseMaterialFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

bool ItemJsonWriter::WriteWeaponToFile(const std::vector<ItemWeaponData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;
        jItem["description"] = item.description;

        jItem["weaponCategory"] = item.weaponCategory;
        jItem["weaponType"] = item.weaponType;
        jItem["caliber"] = item.caliber;
        jItem["minDamage"] = item.minDamage;
        jItem["maxDamage"] = item.maxDamage;
        jItem["fireRate"] = item.fireRate;
        jItem["accuracy"] = item.accuracy;
        jItem["recoil"] = item.recoil;
        jItem["ergonomics"] = item.ergonomics;
        jItem["weight"] = item.weight;
        jItem["durability"] = item.durability;
        jItem["muzzleVelocity"] = item.muzzleVelocity;
        jItem["effectiveRange"] = item.effectiveRange;
        jItem["penetrationPower"] = item.penetrationPower;
        jItem["moddingSlots"] = item.moddingSlots;
        jItem["attackSpeed"] = item.attackSpeed;
        jItem["reach"] = item.reach;
        jItem["staminaCost"] = item.staminaCost;

        // Write attachment slots
        json jSlots = json::array();
        for (const auto& slot : item.attachmentSlots)
        {
            json jSlot;
            jSlot["slotType"] = slot.slotType;
            jSlot["slotIndex"] = slot.slotIndex;
            jSlot["isRequired"] = slot.isRequired;
            jSlots.push_back(jSlot);
        }
        jItem["attachmentSlots"] = jSlots;

        jArray.push_back(jItem);
    }

    // Ensure directory exists before writing
    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " weapon items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::WriteWeaponComponentToFile(const std::vector<ItemWeaponComponentData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;
        jItem["description"] = item.description;

        jItem["componentType"] = item.componentType;
        
        // Magazine-specific fields (only for Magazine type)
        if (item.componentType == "Magazine" || item.componentType == "magazine")
        {
            jItem["magazineCapacity"] = item.magazineCapacity;
            jItem["caliber"] = item.caliber;
            jItem["magazineType"] = item.magazineType;
            json jRounds = json::array();
            for (const auto& segment : item.loadedRounds)
            {
                json jSegment;
                jSegment["orderIndex"] = segment.orderIndex;
                jSegment["roundCount"] = segment.roundCount;
                jSegment["ammoId"] = segment.ammoId;
                jSegment["ammoDisplayName"] = segment.ammoDisplayName;
                jSegment["ammoNotes"] = segment.ammoNotes;
                jRounds.push_back(jSegment);
            }
            jItem["loadedRounds"] = jRounds;
        }
        
        jItem["damageModifier"] = item.damageModifier;
        jItem["recoilModifier"] = item.recoilModifier;
        jItem["ergonomicsModifier"] = item.ergonomicsModifier;
        jItem["accuracyModifier"] = item.accuracyModifier;
        jItem["weightModifier"] = item.weightModifier;
        jItem["muzzleVelocityModifier"] = item.muzzleVelocityModifier;
        jItem["effectiveRangeModifier"] = item.effectiveRangeModifier;
        jItem["penetrationModifier"] = item.penetrationModifier;
        jItem["hasBuiltInRail"] = item.hasBuiltInRail;
        jItem["railType"] = item.railType;

        // Write compatible slots
        jItem["compatibleSlots"] = item.compatibleSlots;

        // Write sub slots
        json jSubSlots = json::array();
        for (const auto& subSlot : item.subSlots)
        {
            json jSubSlot;
            jSubSlot["slotType"] = subSlot.slotType;
            jSubSlot["slotIndex"] = subSlot.slotIndex;
            jSubSlot["hasBuiltInRail"] = subSlot.hasBuiltInRail;
            jSubSlots.push_back(jSubSlot);
        }
        jItem["subSlots"] = jSubSlots;

        jArray.push_back(jItem);
    }

    // Ensure directory exists before writing
    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " weapon component items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::MergeWeaponToFile(const std::vector<ItemWeaponData>& newItems, const std::string& path)
{
    std::vector<ItemWeaponData> existingItems;
    std::set<std::string> existingIds;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseWeaponFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    std::vector<ItemWeaponData> mergedItems = existingItems;
    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
        }
    }

    return WriteWeaponToFile(mergedItems, path);
}

bool ItemJsonWriter::MergeWeaponComponentToFile(const std::vector<ItemWeaponComponentData>& newItems, const std::string& path)
{
    std::vector<ItemWeaponComponentData> existingItems;
    std::set<std::string> existingIds;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseWeaponComponentFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    std::vector<ItemWeaponComponentData> mergedItems = existingItems;
    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
        }
    }

    return WriteWeaponComponentToFile(mergedItems, path);
}

std::set<std::string> ItemJsonWriter::GetExistingWeaponIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemWeaponData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseWeaponFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

std::set<std::string> ItemJsonWriter::GetExistingWeaponComponentIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemWeaponComponentData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseWeaponComponentFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

bool ItemJsonWriter::WriteAmmoToFile(const std::vector<ItemAmmoData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;
        jItem["description"] = item.description;

        jItem["caliber"] = item.caliber;
        jItem["damageBonus"] = item.damageBonus;
        jItem["penetration"] = item.penetration;
        jItem["accuracyBonus"] = item.accuracyBonus;
        jItem["recoilModifier"] = item.recoilModifier;
        jItem["armorPiercing"] = item.armorPiercing;
        jItem["hollowPoint"] = item.hollowPoint;
        jItem["tracer"] = item.tracer;
        jItem["incendiary"] = item.incendiary;
        jItem["value"] = item.value;

        jArray.push_back(jItem);
    }

    // Ensure directory exists before writing
    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " ammo items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::MergeAmmoToFile(const std::vector<ItemAmmoData>& newItems, const std::string& path)
{
    std::vector<ItemAmmoData> existingItems;
    std::set<std::string> existingIds;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseAmmoFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    std::vector<ItemAmmoData> mergedItems = existingItems;
    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
        }
    }

    return WriteAmmoToFile(mergedItems, path);
}

std::set<std::string> ItemJsonWriter::GetExistingAmmoIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemAmmoData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseAmmoFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

bool ItemJsonWriter::WriteArmorToFile(const std::vector<ItemArmorData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;
        jItem["description"] = item.description;

        jItem["armorType"] = item.armorType;
        jItem["armorClass"] = item.armorClass;
        jItem["durability"] = item.durability;
        jItem["material"] = item.material;
        jItem["protectionZones"] = item.protectionZones;
        jItem["movementSpeedPenalty"] = item.movementSpeedPenalty;
        jItem["ergonomicsPenalty"] = item.ergonomicsPenalty;
        jItem["turnSpeedPenalty"] = item.turnSpeedPenalty;
        jItem["weight"] = item.weight;
        jItem["capacity"] = item.capacity;
        jItem["blocksHeadset"] = item.blocksHeadset;
        jItem["blocksFaceCover"] = item.blocksFaceCover;

        jArray.push_back(jItem);
    }

    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " armor items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::WriteClothingToFile(const std::vector<ItemClothingData>& items, const std::string& path)
{
    json jArray = json::array();

    for (const auto& item : items)
    {
        json jItem;

        jItem["id"] = item.id;
        jItem["displayName"] = item.displayName;
        jItem["category"] = item.category;
        jItem["rarity"] = item.rarity;
        jItem["maxStack"] = item.maxStack;
        jItem["description"] = item.description;

        jItem["clothingType"] = item.clothingType;
        jItem["coldResistance"] = item.coldResistance;
        jItem["heatResistance"] = item.heatResistance;
        jItem["waterResistance"] = item.waterResistance;
        jItem["windResistance"] = item.windResistance;
        jItem["comfort"] = item.comfort;
        jItem["mobilityBonus"] = item.mobilityBonus;
        jItem["staminaBonus"] = item.staminaBonus;
        jItem["durability"] = item.durability;
        jItem["material"] = item.material;
        jItem["weight"] = item.weight;
        jItem["isInsulated"] = item.isInsulated;
        jItem["isWaterproof"] = item.isWaterproof;
        jItem["isWindproof"] = item.isWindproof;

        jArray.push_back(jItem);
    }

    if (!EnsureDirectoryExists(path))
    {
        std::cerr << "[ItemJsonWriter] Failed to create directory for: " << path << "\n";
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "[ItemJsonWriter] Failed to open file: " << path << "\n";
        return false;
    }

    ofs << jArray.dump(2);
    std::cout << "[ItemJsonWriter] Wrote " << items.size()
        << " clothing items to JSON file: " << path << "\n";
    return true;
}

bool ItemJsonWriter::MergeArmorToFile(const std::vector<ItemArmorData>& newItems, const std::string& path)
{
    std::vector<ItemArmorData> existingItems;
    std::set<std::string> existingIds;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseArmorFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    std::vector<ItemArmorData> mergedItems = existingItems;
    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
        }
    }

    return WriteArmorToFile(mergedItems, path);
}

bool ItemJsonWriter::MergeClothingToFile(const std::vector<ItemClothingData>& newItems, const std::string& path)
{
    std::vector<ItemClothingData> existingItems;
    std::set<std::string> existingIds;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseClothingFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    std::vector<ItemClothingData> mergedItems = existingItems;
    for (const auto& newItem : newItems)
    {
        if (existingIds.find(newItem.id) == existingIds.end())
        {
            mergedItems.push_back(newItem);
            existingIds.insert(newItem.id);
        }
    }

    return WriteClothingToFile(mergedItems, path);
}

std::set<std::string> ItemJsonWriter::GetExistingArmorIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemArmorData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseArmorFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}

std::set<std::string> ItemJsonWriter::GetExistingClothingIds(const std::string& path)
{
    std::set<std::string> existingIds;
    std::vector<ItemClothingData> existingItems;

    std::ifstream ifs(path);
    if (ifs.is_open())
    {
        std::string existingContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (!existingContent.empty())
        {
            if (ItemJsonParser::ParseClothingFromJsonText(existingContent, existingItems))
            {
                for (const auto& item : existingItems)
                {
                    existingIds.insert(item.id);
                }
            }
        }
    }

    return existingIds;
}


