// ===============================
// Project Name: RundeeItemFactory
// File Name: ArmorItemDataSO.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: ScriptableObject data structure for Armor items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using UnityEngine;

/// <summary>
/// ScriptableObject representing an Armor item with all its properties.
/// </summary>
[CreateAssetMenu(
    fileName = "ArmorItemData",
    menuName = "Rundee/Items/Armor Item Data",
    order = 0)]
public class ArmorItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;
    public string displayName;
    public string category;
    public string rarity;

    [Header("Stacking")]
    public int maxStack;

    [Header("Armor Type")]
    public string armorType;

    [Header("Protection Stats")]
    public int armorClass;
    public int durability;
    public int material;
    public string protectionZones;

    [Header("Mobility Impact")]
    public int movementSpeedPenalty;
    public int ergonomicsPenalty;
    public int turnSpeedPenalty;

    [Header("Weight and Capacity")]
    public int weight;
    public int capacity;

    [Header("Special Properties")]
    public bool blocksHeadset;
    public bool blocksFaceCover;

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

