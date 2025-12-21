using System;
using UnityEngine;

/// <summary>
/// ScriptableObject representing a weapon item in Unity
/// </summary>
/// <remarks>
/// This class mirrors the C++ ItemWeaponData structure and is used to import
/// weapon items from JSON into Unity as ScriptableObjects. Supports both ranged
/// and melee weapons with attachment slots.
/// </remarks>
[CreateAssetMenu(
    fileName = "WeaponItemData",
    menuName = "Rundee/Items/Weapon Item Data",
    order = 0)]
public class WeaponItemDataSO : ScriptableObject
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

    /// <summary>Weapon type (e.g., "AssaultRifle", "Sword")</summary>
    [Header("Weapon Type")]
    public string weaponType;

    /// <summary>Caliber for ranged weapons (e.g., "9mm", "5.56mm")</summary>
    public string caliber;

    /// <summary>Minimum damage per hit</summary>
    [Header("Combat Stats")]
    public int minDamage;

    /// <summary>Maximum damage per hit</summary>
    public int maxDamage;

    /// <summary>Fire rate (rounds per minute for ranged, attacks per minute for melee)</summary>
    public int fireRate;

    /// <summary>Base accuracy (0-100)</summary>
    public int accuracy;

    /// <summary>Recoil control (0-100, lower is better)</summary>
    public int recoil;

    /// <summary>Ergonomics rating (0-100, higher is better)</summary>
    public int ergonomics;

    /// <summary>Weight in grams</summary>
    public int weight;

    /// <summary>Weapon condition (0-100)</summary>
    public int durability;

    /// <summary>Muzzle velocity in m/s (ranged only)</summary>
    [Header("Advanced Stats (Tarkov-style)")]
    public int muzzleVelocity;

    /// <summary>Effective range in meters (ranged only)</summary>
    public int effectiveRange;

    /// <summary>Armor penetration capability (0-100)</summary>
    public int penetrationPower;

    /// <summary>Number of available modding slots</summary>
    public int moddingSlots;

    /// <summary>
    /// Represents an attachment slot on a weapon
    /// </summary>
    [Serializable]
    public class AttachmentSlot
    {
        /// <summary>Slot type (e.g., "Muzzle", "Grip", "Sight")</summary>
        public string slotType;

        /// <summary>Slot index for multiple slots of same type</summary>
        public int slotIndex;

        /// <summary>Whether this slot must be filled</summary>
        public bool isRequired;
    }

    /// <summary>Attachment slots available on this weapon</summary>
    [Header("Attachment Slots")]
    public AttachmentSlot[] attachmentSlots;

    /// <summary>Item description text</summary>
    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

