// ===============================
// Project Name: RundeeItemFactory
// File Name: WeaponComponentItemDataSO.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: ScriptableObject data structure for WeaponComponent items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using System;
using UnityEngine;

/// <summary>
/// ScriptableObject representing a WeaponComponent item with all its properties.
/// </summary>
[CreateAssetMenu(
    fileName = "WeaponComponentItemData",
    menuName = "Rundee/Items/Weapon Component Item Data",
    order = 0)]
public class WeaponComponentItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;
    public string displayName;
    public string category;
    public string rarity;

    [Header("Stacking")]
    public int maxStack;

    [Header("Component Type")]
    public string componentType;
    public string[] compatibleSlots;

    [Header("Magazine Properties (Only for Magazine type)")]
    public int magazineCapacity;
    public string caliber;
    public string magazineType;
    [Serializable]
    public class LoadedRoundSegment
    {
        public int orderIndex;
        public int roundCount;
        public string ammoId;
        public string ammoDisplayName;
        public string ammoNotes;
    }
    [Header("Magazine Load Plan (Top-to-Bottom)")]
    public LoadedRoundSegment[] loadedRounds;

    [Serializable]
    public class ComponentSubSlot
    {
        public string slotType;
        public int slotIndex;
        public bool hasBuiltInRail;
    }
    [Header("Sub Slots")]
    public ComponentSubSlot[] subSlots;

    [Header("Stat Modifiers")]
    public int damageModifier;
    public int recoilModifier;
    public int ergonomicsModifier;
    public int accuracyModifier;
    public int weightModifier;
    public int muzzleVelocityModifier;
    public int effectiveRangeModifier;
    public int penetrationModifier;

    [Header("Rail Properties")]
    public bool hasBuiltInRail;
    public string railType;

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

