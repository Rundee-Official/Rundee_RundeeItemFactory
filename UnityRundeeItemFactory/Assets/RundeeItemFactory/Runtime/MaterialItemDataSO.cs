// ===============================
// Project Name: RundeeItemFactory
// File Name: MaterialItemDataSO.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: ScriptableObject data structure for Material items.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using UnityEngine;

/// <summary>
/// ScriptableObject representing a Material item with all its properties.
/// </summary>
[CreateAssetMenu(
    fileName = "MaterialItemData",
    menuName = "Rundee/Items/Material Item Data",
    order = 1)]
public class MaterialItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;              // JSON: id
    public string displayName;     // JSON: displayName

    [Header("Classification")]
    public string category;        // JSON: category (Material, Junk, Component)
    public string rarity;          // JSON: rarity

    [Header("Stacking")]
    public int maxStack;           // JSON: maxStack

    [Header("Material Properties")]
    public string materialType;   // JSON: materialType (Wood, Metal, Plastic, etc.)
    public int hardness;          // JSON: hardness (0-100)
    public int flammability;      // JSON: flammability (0-100)
    public int value;             // JSON: value (0-100)

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;     // JSON: description
}




