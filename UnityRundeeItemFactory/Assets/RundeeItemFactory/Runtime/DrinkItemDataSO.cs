// ===============================
// Project Name: RundeeItemFactory
// File Name: DrinkItemDataSO.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: ScriptableObject data structure for Drink items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using UnityEngine;

/// <summary>
/// ScriptableObject representing a Drink item with all its properties.
/// </summary>
[CreateAssetMenu(
    fileName = "DrinkItemData",
    menuName = "Rundee/Items/Drink Item Data",
    order = 0)]
public class DrinkItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;              // JSON: id
    public string displayName;     // JSON: displayName
    public string category;        // JSON: category (Drink)
    public string rarity;          // JSON: rarity

    [Header("Stacking")]
    public int maxStack;           // JSON: maxStack

    [Header("Effects")]
    public int hungerRestore;      // JSON: hungerRestore
    public int thirstRestore;      // JSON: thirstRestore
    public int healthRestore;      // JSON: healthRestore

    [Header("Spoilage")]
    public bool spoils;            // JSON: spoils
    public int spoilTimeMinutes;   // JSON: spoilTimeMinutes

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;     // JSON: description
}



