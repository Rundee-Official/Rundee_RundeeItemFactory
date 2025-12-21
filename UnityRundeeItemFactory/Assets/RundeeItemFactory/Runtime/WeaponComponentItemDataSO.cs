using System;
using UnityEngine;

/// <summary>
/// ScriptableObject representing a weapon component/attachment in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemWeaponComponentData structure and is used to import
/// weapon component items from JSON into Unity as ScriptableObjects. Components can
/// be attached to weapons to modify their stats.
/// </remarks>
[CreateAssetMenu(
    fileName = "WeaponComponentItemData",
    menuName = "Rundee/Items/Weapon Component Item Data",
    order = 0)]
public class WeaponComponentItemDataSO : ScriptableObject
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

    /// <summary>Component type (e.g., "Muzzle", "Grip", "Scope", "Magazine")</summary>
    [Header("Component Type")]
    public string componentType;

    /// <summary>Compatible weapon slots this component can attach to</summary>
    public string[] compatibleSlots;

    /// <summary>Magazine capacity in rounds (Magazine type only)</summary>
    [Header("Magazine Properties (Only for Magazine type)")]
    public int magazineCapacity;

    /// <summary>Compatible caliber (Magazine type only)</summary>
    public string caliber;

    /// <summary>Magazine type (e.g., "Standard", "Extended", "Drum")</summary>
    public string magazineType;

    /// <summary>
    /// Represents a segment of loaded rounds in a magazine
    /// </summary>
    [Serializable]
    public class LoadedRoundSegment
    {
        /// <summary>Order index (0 = first rounds to fire)</summary>
        public int orderIndex;

        /// <summary>Number of consecutive rounds using this ammo type</summary>
        public int roundCount;

        /// <summary>Reference to Ammo item ID</summary>
        public string ammoId;

        /// <summary>Optional friendly display name</summary>
        public string ammoDisplayName;

        /// <summary>Notes about the ammo (e.g., "AP", "FMJ")</summary>
        public string ammoNotes;
    }

    /// <summary>Actual loaded round order preview (Magazine type only)</summary>
    [Header("Magazine Load Plan (Top-to-Bottom)")]
    public LoadedRoundSegment[] loadedRounds;

    /// <summary>
    /// Represents a sub-slot on a component
    /// </summary>
    [Serializable]
    public class ComponentSubSlot
    {
        /// <summary>Slot type (e.g., "Muzzle", "Rail")</summary>
        public string slotType;

        /// <summary>Slot index for multiple slots of same type</summary>
        public int slotIndex;

        /// <summary>Whether this slot has a built-in rail</summary>
        public bool hasBuiltInRail;
    }

    /// <summary>Sub-slots on this component (components that can attach to this component)</summary>
    [Header("Sub Slots")]
    public ComponentSubSlot[] subSlots;

    /// <summary>Damage modifier (can be negative)</summary>
    [Header("Stat Modifiers")]
    public int damageModifier;

    /// <summary>Recoil modifier (positive = less recoil)</summary>
    public int recoilModifier;

    /// <summary>Ergonomics modifier (can be negative)</summary>
    public int ergonomicsModifier;

    /// <summary>Accuracy modifier (can be negative)</summary>
    public int accuracyModifier;

    /// <summary>Weight modifier in grams (can be negative)</summary>
    public int weightModifier;

    /// <summary>Muzzle velocity modifier in m/s (can be negative)</summary>
    public int muzzleVelocityModifier;

    /// <summary>Effective range modifier in meters (can be negative)</summary>
    public int effectiveRangeModifier;

    /// <summary>Penetration power modifier (can be negative)</summary>
    public int penetrationModifier;

    /// <summary>Whether this component has an integrated rail</summary>
    [Header("Rail Properties")]
    public bool hasBuiltInRail;

    /// <summary>Rail type (e.g., "Picatinny", "M-LOK")</summary>
    public string railType;

    /// <summary>Item description text</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

