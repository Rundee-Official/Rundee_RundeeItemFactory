// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemWeaponData.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Data structure for weapon items with attachment slots.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include "Data/ItemDataBase.h"
#include <vector>
#include <string>

// Attachment slot definition
struct WeaponAttachmentSlot
{
    std::string slotType;        // "Muzzle", "Grip", "Sight", "Scope", "Stock", "Barrel", "Handguard", "Rail", etc.
    int slotIndex = 0;           // For multiple slots of same type (e.g., multiple rail slots)
    bool isRequired = false;     // Whether this slot must be filled
};

struct ItemWeaponData : public ItemDataBase
{
    // Weapon type
    std::string weaponType;      // "AssaultRifle", "SMG", "Pistol", "SniperRifle", "Shotgun", etc.
    std::string caliber;         // "9mm", "5.56mm", "7.62mm", "12gauge", etc.

    // Combat stats
    int minDamage = 0;           // Minimum damage per shot
    int maxDamage = 0;           // Maximum damage per shot
    int fireRate = 0;            // Rounds per minute
    int accuracy = 0;            // 0-100, base accuracy
    int recoil = 0;              // 0-100, lower is better (recoil control)
    int ergonomics = 0;         // 0-100, higher is better (handling)
    int weight = 0;              // Weight in grams
    int durability = 100;        // 0-100, weapon condition

    // Magazine
    int magazineCapacity = 0;    // Default magazine capacity
    std::string magazineType;    // "Standard", "Extended", "Drum", etc.

    // Attachment slots (what can be attached to this weapon)
    std::vector<WeaponAttachmentSlot> attachmentSlots;

    // Override base class method
    std::string GetItemTypeName() const override { return "Weapon"; }
};

