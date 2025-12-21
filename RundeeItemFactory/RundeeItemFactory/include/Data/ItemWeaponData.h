/**
 * @file ItemWeaponData.h
 * @brief Data structure for weapon items with attachment slots
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for weapon items, including ranged and melee weapons.
 * Supports attachment slots for weapon customization.
 */

#pragma once

#include "Data/ItemDataBase.h"
#include <vector>
#include <string>

/**
 * @struct WeaponAttachmentSlot
 * @brief Defines an attachment slot on a weapon
 * 
 * Represents a single slot where a weapon component can be attached.
 * Supports multiple slots of the same type (e.g., multiple rail slots).
 */
struct WeaponAttachmentSlot
{
    /**
     * @brief Slot type identifier
     * 
     * Examples: "Muzzle", "Grip", "Sight", "Scope", "Stock", "Barrel",
     * "Handguard", "Rail", etc. Must match componentType of compatible components.
     */
    std::string slotType;

    /**
     * @brief Slot index for multiple slots of same type
     * 
     * Used when a weapon has multiple slots of the same type (e.g., multiple rail slots).
     * Index 0 is the first slot, 1 is the second, etc.
     */
    int slotIndex = 0;

    /**
     * @brief Whether this slot must be filled
     * 
     * If true, the weapon cannot function without a component in this slot.
     */
    bool isRequired = false;
};

/**
 * @struct ItemWeaponData
 * @brief Data structure representing a weapon item
 * 
 * Contains all properties for both ranged and melee weapons, including
 * combat stats, attachment slots, and weapon-specific properties.
 */
struct ItemWeaponData : public ItemDataBase
{
    /**
     * @brief Weapon category
     * 
     * Either "Ranged" or "Melee". Determines if the weapon uses ammunition
     * and which stats are applicable.
     */
    std::string weaponCategory;

    /**
     * @brief Weapon type identifier
     * 
     * For Ranged: "AssaultRifle", "SMG", "Pistol", "SniperRifle", "Shotgun", "LMG", "DMR"
     * For Melee: "Sword", "Axe", "Knife", "Mace", "Spear", "Club", etc.
     */
    std::string weaponType;

    /**
     * @brief Caliber for ranged weapons
     * 
     * Examples: "9mm", "5.56mm", "7.62mm", "12gauge", etc.
     * Empty string for melee weapons. Must match compatible ammo caliber.
     */
    std::string caliber;

    /**
     * @brief Minimum damage per hit
     * 
     * For Ranged: Base damage (actual damage = base + ammo.damageBonus)
     * For Melee: Actual damage (no ammo modifier)
     */
    int minDamage = 0;

    /**
     * @brief Maximum damage per hit
     * 
     * Damage varies between minDamage and maxDamage.
     */
    int maxDamage = 0;

    /**
     * @brief Fire rate
     * 
     * Rounds per minute for Ranged weapons, or Attacks per minute for Melee weapons.
     */
    int fireRate = 0;

    /**
     * @brief Base accuracy (0-100)
     * 
     * For Ranged: Base accuracy (higher is better)
     * For Melee: Hit chance (higher is better)
     */
    int accuracy = 0;

    /**
     * @brief Recoil control (0-100, lower is better)
     * 
     * Only for Ranged weapons. Lower values indicate better recoil control.
     */
    int recoil = 0;

    /**
     * @brief Ergonomics rating (0-100, higher is better)
     * 
     * Affects handling and weapon manipulation speed.
     */
    int ergonomics = 0;

    /**
     * @brief Weight in grams
     * 
     * Affects movement speed and stamina consumption.
     */
    int weight = 0;

    /**
     * @brief Weapon condition (0-100)
     * 
     * Current durability. Lower values may affect weapon performance.
     */
    int durability = 100;

    /**
     * @brief Muzzle velocity in m/s (Ranged only)
     * 
     * Bullet velocity at the muzzle. 0 for melee weapons.
     */
    int muzzleVelocity = 0;

    /**
     * @brief Effective range in meters (Ranged only)
     * 
     * Maximum effective engagement range. 0 for melee weapons.
     */
    int effectiveRange = 0;

    /**
     * @brief Armor penetration capability (0-100, Ranged only)
     * 
     * Higher values indicate better armor penetration.
     */
    int penetrationPower = 0;

    /**
     * @brief Number of available modding slots
     * 
     * Total number of attachment slots available on this weapon.
     */
    int moddingSlots = 0;

    /**
     * @brief Attack speed in attacks per second (Melee only)
     * 
     * Alternative to fireRate for melee weapons.
     */
    int attackSpeed = 0;

    /**
     * @brief Reach in meters (Melee only)
     * 
     * Maximum melee attack range.
     */
    int reach = 0;

    /**
     * @brief Stamina cost per attack (0-100, Melee only)
     * 
     * Higher values indicate more stamina consumption per attack.
     */
    int staminaCost = 0;

    /**
     * @brief Attachment slots available on this weapon
     * 
     * List of slots where weapon components can be attached.
     * Note: Magazine is now a separate WeaponComponent, not included here.
     */
    std::vector<WeaponAttachmentSlot> attachmentSlots;

    /**
     * @brief Get the item type name
     * @return Always returns "Weapon"
     */
    std::string GetItemTypeName() const override { return "Weapon"; }
};

