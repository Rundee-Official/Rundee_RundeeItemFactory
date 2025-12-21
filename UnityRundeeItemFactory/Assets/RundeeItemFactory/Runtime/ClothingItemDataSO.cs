using UnityEngine;

/// <summary>
/// ScriptableObject representing a clothing item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemClothingData structure and is used to import
/// clothing items from JSON into Unity as ScriptableObjects. Clothing items provide
/// warmth and style bonuses.
/// </remarks>
[CreateAssetMenu(
    fileName = "ClothingItemData",
    menuName = "Rundee/Items/Clothing Item Data",
    order = 0)]
public class ClothingItemDataSO : ScriptableObject
{
    /// <summary>Unique item identifier (JSON: id)</summary>
    [Header("Identity")]
    public string id;

    /// <summary>User-facing display name (JSON: displayName)</summary>
    public string displayName;

    /// <summary>Item category, typically "Clothing" (JSON: category)</summary>
    public string category;

    /// <summary>Item rarity: "Common", "Uncommon", or "Rare" (JSON: rarity)</summary>
    public string rarity;

    /// <summary>Maximum stack size in inventory (JSON: maxStack)</summary>
    [Header("Stacking")]
    public int maxStack;

    /// <summary>Warmth value (0-100) (JSON: warmth)</summary>
    [Header("Properties")]
    public int warmth;

    /// <summary>Style value (0-100) (JSON: style)</summary>
    public int style;

    /// <summary>Clothing durability (0-100) (JSON: durability)</summary>
    [Header("Durability")]
    public int durability;

    /// <summary>Weight in grams (JSON: weight)</summary>
    [Header("Physical")]
    public int weight;

    /// <summary>Clothing type (e.g., "Shirt", "Pants", "Jacket") (JSON: clothingType)</summary>
    public string clothingType;

    /// <summary>Item value (JSON: value)</summary>
    public int value;

    /// <summary>Item description text (JSON: description)</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}



