using UnityEngine;

[CreateAssetMenu(
    fileName = "DrinkItemData",
    menuName = "Rundee/Items/Drink Item Data",
    order = 0)]
public class DrinkItemDataSO : ScriptableObject
{
    [Header("Identity")]
    public string id;              // JSON: id
    public string displayName;     // JSON: displayName
    public string category;        // JSON: category (Drink)
    public string rarity;          // JSON: rarity

    [Header("Stacking")]
    public int maxStack;           // JSON: maxStack

    [Header("Effects")]
    public int hungerRestore;      // JSON: hungerRestore
    public int thirstRestore;      // JSON: thirstRestore
    public int healthRestore;      // JSON: healthRestore

    [Header("Spoilage")]
    public bool spoils;            // JSON: spoils
    public int spoilTimeMinutes;   // JSON: spoilTimeMinutes

    [Header("Description")]
    [TextArea(2, 4)]
    public string description;     // JSON: description
}



