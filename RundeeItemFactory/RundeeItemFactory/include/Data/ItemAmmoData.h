/**
 * @file ItemAmmoData.h
 * @brief Data structure for ammunition items
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the data structure for ammunition items used by ranged weapons.
 * Includes combat modifiers and special properties.
 */

#pragma once

#include "Data/ItemDataBase.h"
#include <string>

/**
 * @struct ItemAmmoData
 * @brief Data structure representing an ammunition item
 * 
 * Contains properties for ammunition, including caliber, combat modifiers,
 * and special properties like armor piercing or tracer rounds.
 */
struct ItemAmmoData : public ItemDataBase
{
    /**
     * @brief Caliber identifier
     * 
     * Examples: "9mm", "5.56mm", "7.62mm", "12gauge", ".45ACP", etc.
     * Must match the caliber of compatible weapons.
     */
    std::string caliber;

    /**
     * @brief Damage bonus/penalty
     * 
     * Modifies weapon base damage. Can be negative for weaker ammo types.
     * Actual damage = weapon.baseDamage + damageBonus.
     */
    int damageBonus = 0;

    /**
     * @brief Armor penetration (0-100, higher is better)
     * 
     * Effectiveness against armored targets. Higher values penetrate better.
     */
    int penetration = 0;

    /**
     * @brief Accuracy bonus/penalty
     * 
     * Modifies weapon accuracy. Can be negative for less accurate ammo.
     */
    int accuracyBonus = 0;

    /**
     * @brief Recoil modifier
     * 
     * Positive values reduce recoil, negative values increase recoil.
     */
    int recoilModifier = 0;

    /**
     * @brief Armor piercing property
     * 
     * If true, the ammo has high penetration capability (AP rounds).
     */
    bool armorPiercing = false;

    /**
     * @brief Hollow point property
     * 
     * If true, the ammo deals higher damage but has lower penetration (HP rounds).
     */
    bool hollowPoint = false;

    /**
     * @brief Tracer property
     * 
     * If true, the bullet trajectory is visible (tracer rounds).
     */
    bool tracer = false;

    /**
     * @brief Incendiary property
     * 
     * If true, the ammo can cause fire damage (incendiary rounds).
     */
    bool incendiary = false;

    /**
     * @brief Trade/scrap value (0-100)
     * 
     * Economic value of the ammunition.
     */
    int value = 0;

    /**
     * @brief Get the item type name
     * @return Always returns "Ammo"
     */
    std::string GetItemTypeName() const override { return "Ammo"; }
};











