// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonWriter.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of unified JSON writer.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Writers/ItemJsonWriter.h"
#include "Parsers/ItemJsonParser.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <iterator>

using nlohmann::json;

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
        jItem["magazineCapacity"] = item.magazineCapacity;
        jItem["magazineType"] = item.magazineType;

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
        jItem["damageModifier"] = item.damageModifier;
        jItem["recoilModifier"] = item.recoilModifier;
        jItem["ergonomicsModifier"] = item.ergonomicsModifier;
        jItem["accuracyModifier"] = item.accuracyModifier;
        jItem["weightModifier"] = item.weightModifier;
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


