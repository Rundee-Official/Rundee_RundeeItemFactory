using System;
using UnityEngine;

[CreateAssetMenu(
    fileName = "WeaponItemData",
    menuName = "Rundee/Items/Weapon Item Data",
    order = 0)]
public class WeaponItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;
    public string displayName;
    public string category;
    public string rarity;

    [Header("Stacking")]
    public int maxStack;

    [Header("Weapon Type")]
    public string weaponType;
    public string caliber;

    [Header("Combat Stats")]
    public int minDamage;
    public int maxDamage;
    public int fireRate;
    public int accuracy;
    public int recoil;
    public int ergonomics;
    public int weight;
    public int durability;

    [Header("Advanced Stats (Tarkov-style)")]
    public int muzzleVelocity;
    public int effectiveRange;
    public int penetrationPower;
    public int moddingSlots;

    [Serializable]
    public class AttachmentSlot
    {
        public string slotType;
        public int slotIndex;
        public bool isRequired;
    }

    [Header("Attachment Slots")]
    public AttachmentSlot[] attachmentSlots;

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;
}

