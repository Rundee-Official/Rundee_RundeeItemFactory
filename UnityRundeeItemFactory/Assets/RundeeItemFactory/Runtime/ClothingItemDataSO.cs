// ===============================
// Project Name: RundeeItemFactory
// File Name: ClothingItemDataSO.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: ScriptableObject data structure for Clothing items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using UnityEngine;

/// <summary>
/// ScriptableObject representing a Clothing item with all its properties.
/// </summary>
[CreateAssetMenu(
    fileName = "ClothingItemData",
    menuName = "Rundee/Items/Clothing Item Data",
    order = 0)]
public class ClothingItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;
    public string displayName;
    public string category;
    public string rarity;

    [Header("Stacking")]
    public int maxStack;

    [Header("Clothing Type")]
    public string clothingType;

    [Header("Environmental Protection")]
    public int coldResistance;
    public int heatResistance;
    public int waterResistance;
    public int windResistance;

    [Header("Comfort and Mobility")]
    public int comfort;
    public int mobilityBonus;
    public int staminaBonus;

    [Header("Durability")]
    public int durability;
    public int material;

    [Header("Weight")]
    public int weight;

    [Header("Special Properties")]
    public bool isInsulated;
    public bool isWaterproof;
    public bool isWindproof;

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

