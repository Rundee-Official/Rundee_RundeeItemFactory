using UnityEngine;

/// <summary>
/// ScriptableObject representing a material item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemMaterialData structure and is used to import
/// material items from JSON into Unity as ScriptableObjects. Material items are
/// used for crafting and trading.
/// </remarks>
[CreateAssetMenu(
    fileName = "MaterialItemData",
    menuName = "Rundee/Items/Material Item Data",
    order = 1)]
public class MaterialItemDataSO : ScriptableObject
{
    /// <summary>Unique item identifier (JSON: id)</summary>
    [Header("Identity")]
    public string id;

    /// <summary>User-facing display name (JSON: displayName)</summary>
    public string displayName;

    /// <summary>Item category: "Material", "Junk", or "Component" (JSON: category)</summary>
    [Header("Classification")]
    public string category;

    /// <summary>Item rarity: "Common", "Uncommon", or "Rare" (JSON: rarity)</summary>
    public string rarity;

    /// <summary>Maximum stack size in inventory (JSON: maxStack)</summary>
    [Header("Stacking")]
    public int maxStack;

    /// <summary>Material type: "Wood", "Metal", "Plastic", etc. (JSON: materialType)</summary>
    [Header("Material Properties")]
    public string materialType;

    /// <summary>Structural strength (0-100) (JSON: hardness)</summary>
    public int hardness;

    /// <summary>Flammability rating (0-100) (JSON: flammability)</summary>
    public int flammability;

    /// <summary>Trade/scrap value (0-100) (JSON: value)</summary>
    public int value;

    /// <summary>Item description text (JSON: description)</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}




