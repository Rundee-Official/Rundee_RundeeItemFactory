using UnityEngine;

/// <summary>
/// ScriptableObject representing an armor item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemArmorData structure and is used to import
/// armor items from JSON into Unity as ScriptableObjects. Armor items provide
/// protection and damage reduction.
/// </remarks>
[CreateAssetMenu(
    fileName = "ArmorItemData",
    menuName = "Rundee/Items/Armor Item Data",
    order = 0)]
public class ArmorItemDataSO : ScriptableObject
{
    /// <summary>Unique item identifier (JSON: id)</summary>
    [Header("Identity")]
    public string id;

    /// <summary>User-facing display name (JSON: displayName)</summary>
    public string displayName;

    /// <summary>Item category, typically "Armor" (JSON: category)</summary>
    public string category;

    /// <summary>Item rarity: "Common", "Uncommon", or "Rare" (JSON: rarity)</summary>
    public string rarity;

    /// <summary>Maximum stack size in inventory (JSON: maxStack)</summary>
    [Header("Stacking")]
    public int maxStack;

    /// <summary>Armor protection value (0-100) (JSON: armorValue)</summary>
    [Header("Protection")]
    public int armorValue;

    /// <summary>Armor durability (0-100) (JSON: durability)</summary>
    [Header("Durability")]
    public int durability;

    /// <summary>Weight in grams (JSON: weight)</summary>
    [Header("Properties")]
    public int weight;

    /// <summary>Armor type (e.g., "Helmet", "Vest", "Plate") (JSON: armorType)</summary>
    public string armorType;

    /// <summary>Item value (JSON: value)</summary>
    public int value;

    /// <summary>Item description text (JSON: description)</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}



