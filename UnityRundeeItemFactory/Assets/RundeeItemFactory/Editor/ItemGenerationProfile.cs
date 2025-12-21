using UnityEngine;
using System;

/// <summary>
/// ScriptableObject representing a character/item generation profile
/// </summary>
/// <remarks>
/// This profile stores example character values and constraints that can be used
/// to generate balanced items across all item types. It serves as a reference
/// for the AI to understand the game's balance and generate appropriate items.
/// </remarks>
[CreateAssetMenu(
    fileName = "ItemGenerationProfile",
    menuName = "Rundee/Item Factory/Generation Profile",
    order = 0)]
public class ItemGenerationProfile : ScriptableObject
{
    [Header("Profile Information")]
    [Tooltip("Name of this profile (e.g., 'Survival Character', 'Combat Focused')")]
    public string profileName = "Default Profile";
    
    [TextArea(3, 5)]
    [Tooltip("Description of this profile and its intended use case")]
    public string description = "Default generation profile";

    [Header("Character Stats (Reference Values)")]
    [Tooltip("Maximum hunger value (0-1000). Used for Food/Drink generation.")]
    [Range(1, 1000)]
    public int maxHunger = 100;
    
    [Tooltip("Maximum thirst value (0-1000). Used for Food/Drink generation.")]
    [Range(1, 1000)]
    public int maxThirst = 100;
    
    [Tooltip("Maximum health value (0-1000). Used for Food/Drink generation.")]
    [Range(1, 1000)]
    public int maxHealth = 100;

    [Header("Weapon Constraints")]
    [Tooltip("Maximum damage value for weapons")]
    [Range(1, 1000)]
    public int maxWeaponDamage = 100;
    
    [Tooltip("Maximum effective range for weapons (meters)")]
    [Range(1, 10000)]
    public int maxWeaponRange = 1000;
    
    [Tooltip("Maximum accuracy bonus for weapons")]
    [Range(-100, 100)]
    public int maxWeaponAccuracy = 100;

    [Header("Armor Constraints")]
    [Tooltip("Maximum armor protection value")]
    [Range(1, 100)]
    public int maxArmorValue = 100;
    
    [Tooltip("Maximum armor durability")]
    [Range(1, 100)]
    public int maxArmorDurability = 100;

    [Header("Clothing Constraints")]
    [Tooltip("Maximum warmth value for clothing")]
    [Range(1, 100)]
    public int maxClothingWarmth = 100;
    
    [Tooltip("Maximum style value for clothing")]
    [Range(1, 100)]
    public int maxClothingStyle = 100;

    [Header("Material Constraints")]
    [Tooltip("Maximum hardness value for materials")]
    [Range(1, 100)]
    public int maxMaterialHardness = 100;
    
    [Tooltip("Maximum flammability value for materials")]
    [Range(1, 100)]
    public int maxMaterialFlammability = 100;

    [Header("Ammo Constraints")]
    [Tooltip("Maximum damage bonus for ammo")]
    [Range(1, 200)]
    public int maxAmmoDamage = 100;
    
    [Tooltip("Maximum penetration value for ammo")]
    [Range(1, 100)]
    public int maxAmmoPenetration = 100;

    [Header("Weapon Component Constraints")]
    [Tooltip("Maximum modifier values for weapon components (damage, recoil, etc.)")]
    [Range(-100, 100)]
    public int maxComponentModifier = 50;

    [Header("General Constraints")]
    [Tooltip("Maximum stack size for items")]
    [Range(1, 1000)]
    public int maxStackSize = 100;
    
    [Tooltip("Maximum item value (economy)")]
    [Range(1, 100000)]
    public int maxItemValue = 10000;

    /// <summary>
    /// Generate a context string from this profile for use in item generation prompts
    /// </summary>
    public string GenerateContextString(ItemType itemType)
    {
        System.Text.StringBuilder sb = new System.Text.StringBuilder();
        sb.AppendLine($"Character Profile: {profileName}");
        sb.AppendLine($"Description: {description}");
        sb.AppendLine();
        sb.AppendLine("Character Stats:");
        sb.AppendLine($"- Max Hunger: {maxHunger}");
        sb.AppendLine($"- Max Thirst: {maxThirst}");
        sb.AppendLine($"- Max Health: {maxHealth}");
        sb.AppendLine();

        switch (itemType)
        {
            case ItemType.Food:
            case ItemType.Drink:
                sb.AppendLine("Food/Drink Constraints:");
                sb.AppendLine($"- Max Hunger Restore: {maxHunger}");
                sb.AppendLine($"- Max Thirst Restore: {maxThirst}");
                sb.AppendLine($"- Max Health Restore: {maxHealth}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.Medicine:
                sb.AppendLine("Medicine Constraints:");
                sb.AppendLine($"- Max Health Restore: {maxHealth}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.Weapon:
                sb.AppendLine("Weapon Constraints:");
                sb.AppendLine($"- Max Damage: {maxWeaponDamage}");
                sb.AppendLine($"- Max Range: {maxWeaponRange}m");
                sb.AppendLine($"- Max Accuracy Bonus: {maxWeaponAccuracy}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.Armor:
                sb.AppendLine("Armor Constraints:");
                sb.AppendLine($"- Max Armor Value: {maxArmorValue}");
                sb.AppendLine($"- Max Durability: {maxArmorDurability}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.Clothing:
                sb.AppendLine("Clothing Constraints:");
                sb.AppendLine($"- Max Warmth: {maxClothingWarmth}");
                sb.AppendLine($"- Max Style: {maxClothingStyle}");
                sb.AppendLine($"- Max Durability: {maxArmorDurability}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.Material:
                sb.AppendLine("Material Constraints:");
                sb.AppendLine($"- Max Hardness: {maxMaterialHardness}");
                sb.AppendLine($"- Max Flammability: {maxMaterialFlammability}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.Ammo:
                sb.AppendLine("Ammo Constraints:");
                sb.AppendLine($"- Max Damage Bonus: {maxAmmoDamage}");
                sb.AppendLine($"- Max Penetration: {maxAmmoPenetration}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;

            case ItemType.WeaponComponent:
                sb.AppendLine("Weapon Component Constraints:");
                sb.AppendLine($"- Max Modifier Values: ±{maxComponentModifier}");
                sb.AppendLine($"- Max Stack Size: {maxStackSize}");
                sb.AppendLine($"- Max Item Value: {maxItemValue}");
                break;
        }

        return sb.ToString();
    }

    /// <summary>
    /// Get a short summary string for display in UI
    /// </summary>
    public string GetSummary()
    {
        return $"{profileName} (Hunger:{maxHunger}, Thirst:{maxThirst}, Health:{maxHealth}, Weapon:{maxWeaponDamage}, Armor:{maxArmorValue})";
    }
}



