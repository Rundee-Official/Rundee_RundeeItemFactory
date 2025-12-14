// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemJsonParser.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of unified JSON parser.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Parsers/ItemJsonParser.h"
#include "Utils/StringUtils.h"
#include "Utils/JsonUtils.h"
#include "Validators/FoodItemValidator.h"
#include "Validators/DrinkItemValidator.h"
#include "Validators/MaterialItemValidator.h"
#include "Validators/WeaponItemValidator.h"
#include "Validators/WeaponComponentItemValidator.h"
#include "Validators/AmmoItemValidator.h"
#include <fstream>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cctype>

using nlohmann::json;

// Debug-mode name shape logging (limited)
namespace
{
    static int gNameLogCount = 0;
    static const int kNameLogMax = 25;

    std::string SanitizeForLog(const std::string& s)
    {
        std::string out = s;
        for (char& c : out)
        {
            if (c == '"') c = '\'';
        }
        return out;
    }

    void LogNameShape(const char* typeName, const std::string& id, const std::string& displayName)
    {
        if (gNameLogCount >= kNameLogMax) return;
        ++gNameLogCount;
        auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
        if (!dbg.is_open()) return;
        dbg << R"({"sessionId":"debug-session","runId":"display-debug","hypothesisId":"H3","location":"ItemJsonParser.cpp","message":"name shape","data":{"type":")"
            << typeName << R"(","id":")" << SanitizeForLog(id) << R"(","display":")" << SanitizeForLog(displayName) << R"("},"timestamp":)"
            << ts << "})" << "\n";
    }
}

bool ItemJsonParser::ParseFoodFromJsonText(const std::string& jsonText, std::vector<ItemFoodData>& outItems)
{
    outItems.clear();

    // Check for empty input
    if (jsonText.empty() || jsonText.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        std::cerr << "[ItemJsonParser] Error: Input JSON text is empty or contains only whitespace.\n";
        return false;
    }

    // Clean LLM response string
    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);
    
    if (cleaned.empty() || cleaned.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        std::cerr << "[ItemJsonParser] Error: JSON text became empty after cleaning.\n";
        return false;
    }

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const json::parse_error& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error (position " << e.byte << "): "
            << e.what() << "\n";
        std::cerr << "[ItemJsonParser] First 200 chars of cleaned text: "
            << cleaned.substr(0, 200) << "\n";
        return false;
    }
    catch (const std::exception& e)
    {
        // #region agent log
        static int dbgCountWeapon = 0;
        if (dbgCountWeapon < 10)
        {
            ++dbgCountWeapon;
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            auto sanitize = [](std::string v)
            {
                for (char& ch : v)
                {
                    if (ch == '"') ch = '\'';
                    else if (ch == '\\') ch = '/';
                }
                return v;
            };
            std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
            if (dbg.is_open())
            {
                dbg << R"({"sessionId":"debug-session","runId":"parse-error","hypothesisId":"H1","location":"ItemJsonParser.cpp","message":"weapon parse fail","data":{"lenOriginal":)"
                    << jsonText.size() << R"(,"lenCleaned":)" << cleaned.size() << R"(,"error":")" << sanitize(e.what() ? std::string(e.what()) : std::string("n/a"))
                    << R"(","firstClean":")" << sanitize(cleaned.substr(0, std::min<size_t>(200, cleaned.size())))
                    << R"("},"timestamp":)" << ts << "})" << "\n";
            }
        }
        // #endregion
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (root.is_null())
    {
        std::cerr << "[ItemJsonParser] Error: Parsed JSON is null.\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Error: Root JSON is not an array (type: " 
            << (root.is_object() ? "object" : "other") << ").\n";
        return false;
    }

    if (root.empty())
    {
        std::cerr << "[ItemJsonParser] Warning: JSON array is empty. No items to parse.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemFoodData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");

        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.hungerRestore = JsonUtils::GetIntSafe(jItem, "hungerRestore", 0);
        item.thirstRestore = JsonUtils::GetIntSafe(jItem, "thirstRestore", 0);
        item.healthRestore = JsonUtils::GetIntSafe(jItem, "healthRestore", 0);
        item.spoils = JsonUtils::GetBoolSafe(jItem, "spoils", false);
        item.spoilTimeMinutes = JsonUtils::GetIntSafe(jItem, "spoilTimeMinutes", 0);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        // Minimum validation: id/displayName must exist
        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        // Category validation: only accept "Food"
        if (item.category != "Food" && item.category != "food" && item.category != "FOOD")
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (category is \"" << item.category << "\", expected \"Food\")\n";
            continue;
        }

        LogNameShape("Food", item.id, item.displayName);

        // Validation/balancing
        FoodItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " food items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseDrinkFromJsonText(const std::string& jsonText, std::vector<ItemDrinkData>& outItems)
{
    outItems.clear();

    // Clean LLM response string
    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        // #region agent log
        static int dbgCountWc = 0;
        if (dbgCountWc < 10)
        {
            ++dbgCountWc;
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            auto sanitize = [](std::string v)
            {
                for (char& ch : v)
                {
                    if (ch == '"') ch = '\'';
                    else if (ch == '\\') ch = '/';
                }
                return v;
            };
            std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
            if (dbg.is_open())
            {
                dbg << R"({"sessionId":"debug-session","runId":"parse-error","hypothesisId":"H1","location":"ItemJsonParser.cpp","message":"weaponcomponent parse fail","data":{"lenOriginal":)"
                    << jsonText.size() << R"(,"lenCleaned":)" << cleaned.size() << R"(,"error":")" << sanitize(e.what() ? std::string(e.what()) : std::string("n/a"))
                    << R"(","firstClean":")" << sanitize(cleaned.substr(0, std::min<size_t>(200, cleaned.size())))
                    << R"("},"timestamp":)" << ts << "})" << "\n";
            }
        }
        // #endregion
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemDrinkData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");

        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.hungerRestore = JsonUtils::GetIntSafe(jItem, "hungerRestore", 0);
        item.thirstRestore = JsonUtils::GetIntSafe(jItem, "thirstRestore", 0);
        item.healthRestore = JsonUtils::GetIntSafe(jItem, "healthRestore", 0);
        item.spoils = JsonUtils::GetBoolSafe(jItem, "spoils", false);
        item.spoilTimeMinutes = JsonUtils::GetIntSafe(jItem, "spoilTimeMinutes", 0);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        // Minimum validation: id/displayName must exist
        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        // Category validation: only accept "Drink"
        if (item.category != "Drink" && item.category != "drink" && item.category != "DRINK")
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (category is \"" << item.category << "\", expected \"Drink\")\n";
            continue;
        }

        LogNameShape("Drink", item.id, item.displayName);

        // Validation/balancing
        DrinkItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " drink items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseMaterialFromJsonText(const std::string& jsonText, std::vector<ItemMaterialData>& outItems)
{
    outItems.clear();

    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error: " << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemMaterialData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");
        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.materialType = JsonUtils::GetStringSafe(jItem, "materialType");
        item.hardness = JsonUtils::GetIntSafe(jItem, "hardness", 0);
        item.flammability = JsonUtils::GetIntSafe(jItem, "flammability", 0);
        item.value = JsonUtils::GetIntSafe(jItem, "value", 0);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping item at index "
                << i << " (missing id/displayName)\n";
            continue;
        }

        LogNameShape("Material", item.id, item.displayName);

        MaterialItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " material items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseWeaponFromJsonText(const std::string& jsonText, std::vector<ItemWeaponData>& outItems)
{
    outItems.clear();

    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        // #region agent log
        static int dbgCountAmmo = 0;
        if (dbgCountAmmo < 10)
        {
            ++dbgCountAmmo;
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            auto sanitize = [](std::string v)
            {
                for (char& ch : v)
                {
                    if (ch == '"') ch = '\'';
                    else if (ch == '\\') ch = '/';
                }
                return v;
            };
            std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
            if (dbg.is_open())
            {
                dbg << R"({"sessionId":"debug-session","runId":"parse-error","hypothesisId":"H1","location":"ItemJsonParser.cpp","message":"ammo parse fail","data":{"lenOriginal":)"
                    << jsonText.size() << R"(,"lenCleaned":)" << cleaned.size() << R"(,"error":")" << sanitize(e.what() ? std::string(e.what()) : std::string("n/a"))
                    << R"(","firstClean":")" << sanitize(cleaned.substr(0, std::min<size_t>(200, cleaned.size())))
                    << R"("},"timestamp":)" << ts << "})" << "\n";
            }
        }
        // #endregion
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemWeaponData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");
        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        item.weaponCategory = JsonUtils::GetStringSafe(jItem, "weaponCategory");
        item.weaponType = JsonUtils::GetStringSafe(jItem, "weaponType");
        item.caliber = JsonUtils::GetStringSafe(jItem, "caliber");
        item.minDamage = JsonUtils::GetIntSafe(jItem, "minDamage", 0);
        item.maxDamage = JsonUtils::GetIntSafe(jItem, "maxDamage", 0);
        item.fireRate = JsonUtils::GetIntSafe(jItem, "fireRate", 0);
        item.accuracy = JsonUtils::GetIntSafe(jItem, "accuracy", 0);
        item.recoil = JsonUtils::GetIntSafe(jItem, "recoil", 0);
        item.ergonomics = JsonUtils::GetIntSafe(jItem, "ergonomics", 0);
        item.weight = JsonUtils::GetIntSafe(jItem, "weight", 0);
        item.durability = JsonUtils::GetIntSafe(jItem, "durability", 100);
        item.muzzleVelocity = JsonUtils::GetIntSafe(jItem, "muzzleVelocity", 0);
        item.effectiveRange = JsonUtils::GetIntSafe(jItem, "effectiveRange", 0);
        item.penetrationPower = JsonUtils::GetIntSafe(jItem, "penetrationPower", 0);
        item.moddingSlots = JsonUtils::GetIntSafe(jItem, "moddingSlots", 0);
        item.attackSpeed = JsonUtils::GetIntSafe(jItem, "attackSpeed", 0);
        item.reach = JsonUtils::GetIntSafe(jItem, "reach", 0);
        item.staminaCost = JsonUtils::GetIntSafe(jItem, "staminaCost", 0);

        // Parse attachment slots
        if (jItem.contains("attachmentSlots") && jItem["attachmentSlots"].is_array())
        {
            for (const auto& jSlot : jItem["attachmentSlots"])
            {
                if (jSlot.is_object())
                {
                    WeaponAttachmentSlot slot;
                    slot.slotType = JsonUtils::GetStringSafe(jSlot, "slotType");
                    slot.slotIndex = JsonUtils::GetIntSafe(jSlot, "slotIndex", 0);
                    slot.isRequired = JsonUtils::GetBoolSafe(jSlot, "isRequired", false);
                    item.attachmentSlots.push_back(slot);
                }
            }
        }

        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping weapon at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        LogNameShape("Weapon", item.id, item.displayName);

        // Validate weaponCategory
        if (item.weaponCategory != "Ranged" && item.weaponCategory != "Melee" && item.weaponCategory != "ranged" && item.weaponCategory != "melee")
        {
            // Try to infer from weaponType if not set
            if (item.weaponCategory.empty())
            {
                std::string lowerType = item.weaponType;
                std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
                if (lowerType.find("sword") != std::string::npos || 
                    lowerType.find("axe") != std::string::npos ||
                    lowerType.find("knife") != std::string::npos ||
                    lowerType.find("mace") != std::string::npos ||
                    lowerType.find("spear") != std::string::npos ||
                    lowerType.find("club") != std::string::npos ||
                    lowerType.find("hammer") != std::string::npos ||
                    lowerType.find("blade") != std::string::npos ||
                    lowerType.find("melee") != std::string::npos)
                {
                    item.weaponCategory = "Melee";
                }
                else
                {
                    item.weaponCategory = "Ranged";
                }
            }
            else
            {
                std::cerr << "[ItemJsonParser] Warning: Invalid weaponCategory \"" << item.weaponCategory 
                    << "\" for weapon " << item.id << ", defaulting to Ranged\n";
                item.weaponCategory = "Ranged";
            }
        }
        
        // Normalize to proper case
        if (item.weaponCategory == "ranged") item.weaponCategory = "Ranged";
        if (item.weaponCategory == "melee") item.weaponCategory = "Melee";

        if (item.category != "Weapon" && item.category != "weapon" && item.category != "WEAPON")
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (category is \"" << item.category << "\", expected \"Weapon\")\n";
            continue;
        }

        // Validation/balancing
        WeaponItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " weapon items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseWeaponComponentFromJsonText(const std::string& jsonText, std::vector<ItemWeaponComponentData>& outItems)
{
    outItems.clear();

    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemWeaponComponentData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");
        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 1);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        item.componentType = JsonUtils::GetStringSafe(jItem, "componentType");
        
        // Magazine-specific fields (only for Magazine type)
        item.magazineCapacity = JsonUtils::GetIntSafe(jItem, "magazineCapacity", 0);
        item.caliber = JsonUtils::GetStringSafe(jItem, "caliber");
        item.magazineType = JsonUtils::GetStringSafe(jItem, "magazineType");
        
        item.damageModifier = JsonUtils::GetIntSafe(jItem, "damageModifier", 0);
        item.recoilModifier = JsonUtils::GetIntSafe(jItem, "recoilModifier", 0);
        item.ergonomicsModifier = JsonUtils::GetIntSafe(jItem, "ergonomicsModifier", 0);
        item.accuracyModifier = JsonUtils::GetIntSafe(jItem, "accuracyModifier", 0);
        item.weightModifier = JsonUtils::GetIntSafe(jItem, "weightModifier", 0);
        item.muzzleVelocityModifier = JsonUtils::GetIntSafe(jItem, "muzzleVelocityModifier", 0);
        item.effectiveRangeModifier = JsonUtils::GetIntSafe(jItem, "effectiveRangeModifier", 0);
        item.penetrationModifier = JsonUtils::GetIntSafe(jItem, "penetrationModifier", 0);
        item.hasBuiltInRail = JsonUtils::GetBoolSafe(jItem, "hasBuiltInRail", false);
        item.railType = JsonUtils::GetStringSafe(jItem, "railType");

        // Parse compatible slots
        if (jItem.contains("compatibleSlots") && jItem["compatibleSlots"].is_array())
        {
            for (const auto& jSlot : jItem["compatibleSlots"])
            {
                if (jSlot.is_string())
                {
                    item.compatibleSlots.push_back(jSlot.get<std::string>());
                }
            }
        }

        // Parse sub slots
        if (jItem.contains("subSlots") && jItem["subSlots"].is_array())
        {
            for (const auto& jSubSlot : jItem["subSlots"])
            {
                if (jSubSlot.is_object())
                {
                    ComponentAttachmentSlot subSlot;
                    subSlot.slotType = JsonUtils::GetStringSafe(jSubSlot, "slotType");
                    subSlot.slotIndex = JsonUtils::GetIntSafe(jSubSlot, "slotIndex", 0);
                    subSlot.hasBuiltInRail = JsonUtils::GetBoolSafe(jSubSlot, "hasBuiltInRail", false);
                    item.subSlots.push_back(subSlot);
                }
            }
        }

        // Parse loaded round segments
        if (jItem.contains("loadedRounds") && jItem["loadedRounds"].is_array())
        {
            for (const auto& jRound : jItem["loadedRounds"])
            {
                if (jRound.is_object())
                {
                    LoadedRoundSegment segment;
                    segment.orderIndex = JsonUtils::GetIntSafe(jRound, "orderIndex", 0);
                    segment.roundCount = JsonUtils::GetIntSafe(jRound, "roundCount", 0);
                    segment.ammoId = JsonUtils::GetStringSafe(jRound, "ammoId");
                    segment.ammoDisplayName = JsonUtils::GetStringSafe(jRound, "ammoDisplayName");
                    segment.ammoNotes = JsonUtils::GetStringSafe(jRound, "ammoNotes");
                    item.loadedRounds.push_back(segment);
                }
            }
        }

        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping component at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        if (item.category != "WeaponComponent" && item.category != "weaponcomponent" && item.category != "WEAPONCOMPONENT")
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (category is \"" << item.category << "\", expected \"WeaponComponent\")\n";
            continue;
        }

        LogNameShape("WeaponComponent", item.id, item.displayName);

        // Validation/balancing
        WeaponComponentItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " weapon component items from JSON.\n";

    return !outItems.empty();
}

bool ItemJsonParser::ParseAmmoFromJsonText(const std::string& jsonText, std::vector<ItemAmmoData>& outItems)
{
    outItems.clear();

    std::string cleaned = StringUtils::CleanJsonArrayText(jsonText);

    json root;
    try
    {
        root = json::parse(cleaned);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ItemJsonParser] JSON parse error even after cleanup: "
            << e.what() << "\n";
        return false;
    }

    if (!root.is_array())
    {
        std::cerr << "[ItemJsonParser] Root JSON is not an array.\n";
        return false;
    }

    for (size_t i = 0; i < root.size(); ++i)
    {
        const json& jItem = root[i];
        if (!jItem.is_object())
        {
            std::cerr << "[ItemJsonParser] Element " << i << " is not an object.\n";
            continue;
        }

        ItemAmmoData item;
        item.id = JsonUtils::GetStringSafe(jItem, "id");
        item.displayName = JsonUtils::GetStringSafe(jItem, "displayName");
        item.category = JsonUtils::GetStringSafe(jItem, "category");
        item.rarity = JsonUtils::GetStringSafe(jItem, "rarity");
        item.maxStack = JsonUtils::GetIntSafe(jItem, "maxStack", 20);
        item.description = JsonUtils::GetStringSafe(jItem, "description");

        item.caliber = JsonUtils::GetStringSafe(jItem, "caliber");
        item.damageBonus = JsonUtils::GetIntSafe(jItem, "damageBonus", 0);
        item.penetration = JsonUtils::GetIntSafe(jItem, "penetration", 0);
        item.accuracyBonus = JsonUtils::GetIntSafe(jItem, "accuracyBonus", 0);
        item.recoilModifier = JsonUtils::GetIntSafe(jItem, "recoilModifier", 0);
        item.armorPiercing = JsonUtils::GetBoolSafe(jItem, "armorPiercing", false);
        item.hollowPoint = JsonUtils::GetBoolSafe(jItem, "hollowPoint", false);
        item.tracer = JsonUtils::GetBoolSafe(jItem, "tracer", false);
        item.incendiary = JsonUtils::GetBoolSafe(jItem, "incendiary", false);
        item.value = JsonUtils::GetIntSafe(jItem, "value", 0);

        if (item.id.empty() || item.displayName.empty())
        {
            std::cerr << "[ItemJsonParser] Skipping ammo at index " << i
                << " (missing id/displayName)\n";
            continue;
        }

        if (item.category != "Ammo" && item.category != "ammo" && item.category != "AMMO")
        {
            std::cerr << "[ItemJsonParser] Skipping item at index " << i
                << " (category is \"" << item.category << "\", expected \"Ammo\")\n";
            continue;
        }

        LogNameShape("Ammo", item.id, item.displayName);

        // Validation/balancing
        AmmoItemValidator::Validate(item);

        outItems.push_back(item);
    }

    std::cout << "[ItemJsonParser] Parsed " << outItems.size()
        << " ammo items from JSON.\n";

    return !outItems.empty();
}


