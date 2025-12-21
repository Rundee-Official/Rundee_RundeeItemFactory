using UnityEngine;

/// <summary>
/// ScriptableObject representing a drink item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemDrinkData structure and is used to import
/// drink items from JSON into Unity as ScriptableObjects. Drink items restore
/// thirst, hunger, and health when consumed.
/// </remarks>
[CreateAssetMenu(
    fileName = "DrinkItemData",
    menuName = "Rundee/Items/Drink Item Data",
    order = 0)]
public class DrinkItemDataSO : ScriptableObject
{
    /// <summary>Unique item identifier (JSON: id)</summary>
    [Header("Identity")]
    public string id;

    /// <summary>User-facing display name (JSON: displayName)</summary>
    public string displayName;

    /// <summary>Item category, typically "Drink" (JSON: category)</summary>
    public string category;

    /// <summary>Item rarity: "Common", "Uncommon", or "Rare" (JSON: rarity)</summary>
    public string rarity;

    /// <summary>Maximum stack size in inventory (JSON: maxStack)</summary>
    [Header("Stacking")]
    public int maxStack;

    /// <summary>Hunger restoration value (0-100) (JSON: hungerRestore)</summary>
    [Header("Effects")]
    public int hungerRestore;

    /// <summary>Thirst restoration value (0-100) (JSON: thirstRestore)</summary>
    public int thirstRestore;

    /// <summary>Health restoration value (0-100) (JSON: healthRestore)</summary>
    public int healthRestore;

    /// <summary>Whether this drink item spoils over time (JSON: spoils)</summary>
    [Header("Spoilage")]
    public bool spoils;

    /// <summary>Time until spoilage in minutes (JSON: spoilTimeMinutes)</summary>
    public int spoilTimeMinutes;

    /// <summary>Item description text (JSON: description)</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}



