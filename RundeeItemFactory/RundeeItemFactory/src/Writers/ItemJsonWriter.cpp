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
#include "json.hpp"
#include <fstream>
#include <iostream>

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


