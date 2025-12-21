using UnityEngine;

/// <summary>
/// ScriptableObject representing a medicine item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemMedicineData structure and is used to import
/// medicine items from JSON into Unity as ScriptableObjects. Medicine items restore
/// health when consumed.
/// </remarks>
[CreateAssetMenu(
    fileName = "MedicineItemData",
    menuName = "Rundee/Items/Medicine Item Data",
    order = 0)]
public class MedicineItemDataSO : ScriptableObject
{
    /// <summary>Unique item identifier (JSON: id)</summary>
    [Header("Identity")]
    public string id;

    /// <summary>User-facing display name (JSON: displayName)</summary>
    public string displayName;

    /// <summary>Item category, typically "Medicine" (JSON: category)</summary>
    public string category;

    /// <summary>Item rarity: "Common", "Uncommon", or "Rare" (JSON: rarity)</summary>
    public string rarity;

    /// <summary>Maximum stack size in inventory (JSON: maxStack)</summary>
    [Header("Stacking")]
    public int maxStack;

    /// <summary>Health restoration value (0-100) (JSON: healthRestore)</summary>
    [Header("Effects")]
    public int healthRestore;

    /// <summary>Whether this medicine item spoils over time (JSON: spoils)</summary>
    [Header("Spoilage")]
    public bool spoils;

    /// <summary>Time until spoilage in minutes (JSON: spoilTimeMinutes)</summary>
    public int spoilTimeMinutes;

    /// <summary>Item description text (JSON: description)</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

