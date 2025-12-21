/**
 * @file ItemDataBase.h
 * @brief Base class for all item data types
 * @author Haneul Lee (Rundee)
 * @date 2025-11-15
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Defines the base structure that all item types inherit from.
 * Contains common properties shared by all items: identity, classification, inventory, and description.
 */

#pragma once

#include <string>

/**
 * @struct ItemDataBase
 * @brief Base structure for all item data types
 * 
 * Provides common properties and virtual methods for all item types.
 * All specific item types (Food, Drink, Material, Weapon, etc.) inherit from this base class.
 */
struct ItemDataBase
{
    /**
     * @brief Unique item identifier
     * 
     * Format: lowercase, underscore-separated (e.g., "food_canned_beans").
     * Must be unique across all items. Used for duplicate detection and references.
     */
    std::string id;

    /**
     * @brief User-facing display name
     * 
     * Human-readable name shown in the game UI (e.g., "Canned Beans").
     */
    std::string displayName;

    /**
     * @brief Item category
     * 
     * Examples: "Food", "Drink", "Material", "Junk", "Component", etc.
     * Used for classification and filtering.
     */
    std::string category;

    /**
     * @brief Item rarity
     * 
     * Valid values: "Common", "Uncommon", "Rare".
     * Affects spawn rates and game balance.
     */
    std::string rarity;

    /**
     * @brief Maximum stack size
     * 
     * How many items of this type can stack in a single inventory slot.
     * Default is 1 (no stacking).
     */
    int maxStack = 1;

    /**
     * @brief Item description
     * 
     * Flavor text describing the item. Shown in item tooltips and UI.
     */
    std::string description;

    /**
     * @brief Virtual destructor
     * 
     * Required for polymorphism. Allows proper cleanup of derived classes.
     */
    virtual ~ItemDataBase() = default;

    /**
     * @brief Get the item type name
     * 
     * Pure virtual method that must be implemented by derived classes.
     * Returns the item type name (e.g., "Food", "Weapon") for logging and debugging.
     * 
     * @return Item type name as a string
     */
    virtual std::string GetItemTypeName() const = 0;
};












