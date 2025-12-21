using UnityEngine;

/// <summary>
/// ScriptableObject representing an ammunition item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemAmmoData structure and is used to import
/// ammunition items from JSON into Unity as ScriptableObjects. Ammo items
/// modify weapon performance when loaded.
/// </remarks>
[CreateAssetMenu(
    fileName = "AmmoItemData",
    menuName = "Rundee/Items/Ammo Item Data",
    order = 0)]
public class AmmoItemDataSO : ScriptableObject
{
    /// <summary>Unique item identifier</summary>
    [Header("Identity")]
    public string id;

    /// <summary>User-facing display name</summary>
    public string displayName;

    /// <summary>Item category</summary>
    public string category;

    /// <summary>Item rarity: "Common", "Uncommon", or "Rare"</summary>
    public string rarity;

    /// <summary>Maximum stack size in inventory</summary>
    [Header("Stacking")]
    public int maxStack;

    /// <summary>Caliber identifier (e.g., "9mm", "5.56mm")</summary>
    [Header("Ammo Type")]
    public string caliber;

    /// <summary>Damage bonus/penalty (can be negative)</summary>
    [Header("Combat Stats")]
    public int damageBonus;

    /// <summary>Armor penetration (0-100, higher is better)</summary>
    public int penetration;

    /// <summary>Accuracy bonus/penalty (can be negative)</summary>
    public int accuracyBonus;

    /// <summary>Recoil modifier (positive = less recoil)</summary>
    public int recoilModifier;

    /// <summary>Armor piercing property (AP rounds)</summary>
    [Header("Special Properties")]
    public bool armorPiercing;

    /// <summary>Hollow point property (HP rounds)</summary>
    public bool hollowPoint;

    /// <summary>Tracer property (visible trajectory)</summary>
    public bool tracer;

    /// <summary>Incendiary property (fire damage)</summary>
    public bool incendiary;

    /// <summary>Trade/scrap value (0-100)</summary>
    [Header("Value")]
    public int value;

    /// <summary>Item description text</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}



