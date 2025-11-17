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
    // Weapon category
    std::string weaponCategory;  // "Ranged" or "Melee" - determines if weapon uses ammo
    
    // Weapon type
    std::string weaponType;      // For Ranged: "AssaultRifle", "SMG", "Pistol", "SniperRifle", "Shotgun", "LMG", "DMR"
                                  // For Melee: "Sword", "Axe", "Knife", "Mace", "Spear", "Club", etc.
    
    // Caliber (only for Ranged weapons)
    std::string caliber;         // "9mm", "5.56mm", "7.62mm", "12gauge", etc. (empty for Melee)

    // Combat stats
    // For Ranged: Base damage (actual damage = base + ammo.damageBonus)
    // For Melee: Actual damage (no ammo modifier)
    int minDamage = 0;           // Minimum damage per hit
    int maxDamage = 0;           // Maximum damage per hit
    // Ranged weapon stats
    int fireRate = 0;            // Rounds per minute (Ranged) or Attacks per minute (Melee)
    int accuracy = 0;            // 0-100, base accuracy (Ranged) or hit chance (Melee)
    int recoil = 0;              // 0-100, lower is better (recoil control, Ranged only)
    int ergonomics = 0;         // 0-100, higher is better (handling)
    int weight = 0;              // Weight in grams
    int durability = 100;        // 0-100, weapon condition
    
    // Advanced stats (Ranged weapons only)
    int muzzleVelocity = 0;      // Muzzle velocity in m/s (Ranged only, 0 for Melee)
    int effectiveRange = 0;      // Effective range in meters (Ranged only, 0 for Melee)
    int penetrationPower = 0;   // 0-100, armor penetration capability (Ranged only)
    int moddingSlots = 0;        // Number of available modding slots
    
    // Melee weapon stats (only for Melee weapons)
    int attackSpeed = 0;         // Attacks per second (Melee only, alternative to fireRate)
    int reach = 0;               // Reach in meters (Melee only)
    int staminaCost = 0;         // Stamina cost per attack (Melee only, 0-100)

    // Attachment slots (what can be attached to this weapon)
    // Note: Magazine is now a separate WeaponComponent, not included here
    std::vector<WeaponAttachmentSlot> attachmentSlots;

    // Override base class method
    std::string GetItemTypeName() const override { return "Weapon"; }
};

