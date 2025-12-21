/**
 * @file ItemWeaponComponentData.h
 * @brief Data structure for weapon attachment components
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for weapon components that can be attached to weapons.
 * Includes magazines, scopes, grips, muzzles, and other attachments.
 */

#pragma once

#include "Data/ItemDataBase.h"
#include <vector>
#include <string>

/**
 * @struct ComponentAttachmentSlot
 * @brief Defines a sub-slot on a component
 * 
 * Some components can have other components attached to them (e.g., handguard with rail slots).
 * This structure represents such sub-slots.
 */
struct ComponentAttachmentSlot
{
    /**
     * @brief Slot type identifier
     * 
     * Examples: "Muzzle", "Grip", "Sight", "Rail", etc.
     */
    std::string slotType;

    /**
     * @brief Slot index for multiple slots of same type
     * 
     * Used when a component has multiple slots of the same type.
     */
    int slotIndex = 0;

    /**
     * @brief Whether this slot already has a built-in rail
     * 
     * If true, the slot already has an integrated rail (e.g., handguard with integrated rail).
     */
    bool hasBuiltInRail = false;
};

/**
 * @struct LoadedRoundSegment
 * @brief Represents a segment of loaded rounds in a magazine
 * 
 * Used for magazines to represent the actual load order of different ammo types.
 * Allows mixed ammo loads (e.g., AP rounds at top, FMJ at bottom).
 */
struct LoadedRoundSegment
{
    /**
     * @brief Order index (0 = first rounds to fire)
     * 
     * Lower values indicate rounds that will be fired first (top of magazine).
     */
    int orderIndex = 0;

    /**
     * @brief Number of consecutive rounds using this ammo type
     * 
     * How many rounds in sequence use the specified ammo type.
     */
    int roundCount = 0;

    /**
     * @brief Reference to Ammo item ID
     * 
     * Examples: "Ammo_762x39_AP", "Ammo_9mm_FMJ", etc.
     */
    std::string ammoId;

    /**
     * @brief Optional friendly display name
     * 
     * Examples: "7.62x39 AP", "9mm FMJ", etc.
     */
    std::string ammoDisplayName;

    /**
     * @brief Notes about the ammo
     * 
     * Examples: "AP", "FMJ", "Tracer", etc.
     */
    std::string ammoNotes;
};

/**
 * @struct ItemWeaponComponentData
 * @brief Data structure representing a weapon component/attachment
 * 
 * Contains all properties for weapon components that can be attached to weapons.
 * Includes stat modifiers, compatibility information, and special properties.
 */
struct ItemWeaponComponentData : public ItemDataBase
{
    /**
     * @brief Component type identifier
     * 
     * Examples: "Muzzle", "Grip", "Sight", "Scope", "Stock", "Barrel", "Handguard", "Rail",
     * "Flashlight", "Laser", "Magazine", "GasBlock", "ChargingHandle", "PistolGrip",
     * "Foregrip", "Bipod", "TacticalDevice", "Mount", "Adapter", etc.
     */
    std::string componentType;

    /**
     * @brief Magazine capacity in rounds (Magazine type only)
     * 
     * Only used when componentType is "Magazine".
     */
    int magazineCapacity = 0;

    /**
     * @brief Compatible caliber (Magazine type only)
     * 
     * Only used when componentType is "Magazine". Must match weapon caliber.
     */
    std::string caliber;

    /**
     * @brief Magazine type (Magazine type only)
     * 
     * Examples: "Standard", "Extended", "Drum", "QuadStack", etc.
     * Only used when componentType is "Magazine".
     */
    std::string magazineType;

    /**
     * @brief Actual loaded round order preview (Magazine type only)
     * 
     * Represents the actual load order of different ammo types in the magazine.
     * Allows mixed ammo loads.
     */
    std::vector<LoadedRoundSegment> loadedRounds;

    /**
     * @brief Compatible weapon slots
     * 
     * List of weapon slot types this component can attach to.
     * Examples: ["Muzzle", "Barrel"] for a muzzle device.
     */
    std::vector<std::string> compatibleSlots;

    /**
     * @brief Sub-slots on this component
     * 
     * Components that can be attached to this component.
     * Examples: Handguard can have Rail slots, Barrel can have Muzzle slot.
     */
    std::vector<ComponentAttachmentSlot> subSlots;

    /**
     * @brief Damage modifier
     * 
     * Damage bonus/penalty applied when attached. Can be negative.
     */
    int damageModifier = 0;

    /**
     * @brief Recoil modifier
     * 
     * Recoil reduction when attached. Positive values reduce recoil.
     */
    int recoilModifier = 0;

    /**
     * @brief Ergonomics modifier
     * 
     * Ergonomics bonus/penalty when attached. Can be negative.
     */
    int ergonomicsModifier = 0;

    /**
     * @brief Accuracy modifier
     * 
     * Accuracy bonus/penalty when attached. Can be negative.
     */
    int accuracyModifier = 0;

    /**
     * @brief Weight modifier in grams
     * 
     * Weight change when attached. Can be negative (reduces weight).
     */
    int weightModifier = 0;

    /**
     * @brief Muzzle velocity modifier in m/s
     * 
     * Muzzle velocity change when attached. Can be negative.
     */
    int muzzleVelocityModifier = 0;

    /**
     * @brief Effective range modifier in meters
     * 
     * Effective range change when attached. Can be negative.
     */
    int effectiveRangeModifier = 0;

    /**
     * @brief Penetration power modifier
     * 
     * Penetration change when attached. Can be negative.
     */
    int penetrationModifier = 0;

    /**
     * @brief Whether this component has an integrated rail
     * 
     * If true, the component has a built-in rail (e.g., handguard with integrated rail).
     */
    bool hasBuiltInRail = false;

    /**
     * @brief Rail type (if hasBuiltInRail is true)
     * 
     * Examples: "Picatinny", "M-LOK", "KeyMod", etc.
     */
    std::string railType;

    /**
     * @brief Get the item type name
     * @return Always returns "WeaponComponent"
     */
    std::string GetItemTypeName() const override { return "WeaponComponent"; }
};

