// ===============================
// Project Name: RundeeItemFactory
// File Name: AmmoItemDataSO.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: ScriptableObject data structure for Ammo items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using UnityEngine;

/// <summary>
/// ScriptableObject representing an Ammo item with all its properties.
/// </summary>
[CreateAssetMenu(
    fileName = "AmmoItemData",
    menuName = "Rundee/Items/Ammo Item Data",
    order = 0)]
public class AmmoItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;
    public string displayName;
    public string category;
    public string rarity;

    [Header("Stacking")]
    public int maxStack;

    [Header("Ammo Type")]
    public string caliber;

    [Header("Combat Stats")]
    public int damageBonus;
    public int penetration;
    public int accuracyBonus;
    public int recoilModifier;

    [Header("Special Properties")]
    public bool armorPiercing;
    public bool hollowPoint;
    public bool tracer;
    public bool incendiary;

    [Header("Value")]
    public int value;

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}



