using UnityEngine;

public class ItemDatabase : MonoBehaviour
{
    [Header("Food Items")]
    public FoodItemDataSO[] foodItems;

    [Header("Drink Items")]
    public DrinkItemDataSO[] drinkItems;

    [Header("Material Items")]
    public MaterialItemDataSO[] materialItems;

    [Header("Weapon Items")]
    public WeaponItemDataSO[] weaponItems;

    [Header("Weapon Component Items")]
    public WeaponComponentItemDataSO[] weaponComponentItems;

    [Header("Ammo Items")]
    public AmmoItemDataSO[] ammoItems;

    private void Awake()
    {
        LoadAllItems();
    }

    public void LoadAllItems()
    {
        // Load Food items
        foodItems = Resources.LoadAll<FoodItemDataSO>("RundeeItemFactory/FoodItems");
        Debug.Log($"[ItemDatabase] Loaded {foodItems.Length} food items from Resources.");

        // Load Drink items
        drinkItems = Resources.LoadAll<DrinkItemDataSO>("RundeeItemFactory/DrinkItems");
        Debug.Log($"[ItemDatabase] Loaded {drinkItems.Length} drink items from Resources.");

        // Load Material items
        materialItems = Resources.LoadAll<MaterialItemDataSO>("RundeeItemFactory/MaterialItems");
        Debug.Log($"[ItemDatabase] Loaded {materialItems.Length} material items from Resources.");

        // Load Weapon items
        weaponItems = Resources.LoadAll<WeaponItemDataSO>("RundeeItemFactory/WeaponItems");
        Debug.Log($"[ItemDatabase] Loaded {weaponItems.Length} weapon items from Resources.");

        // Load Weapon Component items
        weaponComponentItems = Resources.LoadAll<WeaponComponentItemDataSO>("RundeeItemFactory/WeaponComponentItems");
        Debug.Log($"[ItemDatabase] Loaded {weaponComponentItems.Length} weapon component items from Resources.");

        // Load Ammo items
        ammoItems = Resources.LoadAll<AmmoItemDataSO>("RundeeItemFactory/AmmoItems");
        Debug.Log($"[ItemDatabase] Loaded {ammoItems.Length} ammo items from Resources.");

        // Output logs
        LogAllItems();
    }

    private void LogAllItems()
    {
        foreach (var item in foodItems)
        {
            Debug.Log(
                $"[FoodItem] {item.id} / {item.displayName} " +
                $"(cat: {item.category}, rarity: {item.rarity}, " +
                $"Hunger+{item.hungerRestore}, Thirst+{item.thirstRestore})");
        }

        foreach (var item in drinkItems)
        {
            Debug.Log(
                $"[DrinkItem] {item.id} / {item.displayName} " +
                $"(cat: {item.category}, rarity: {item.rarity}, " +
                $"Hunger+{item.hungerRestore}, Thirst+{item.thirstRestore})");
        }

        foreach (var item in materialItems)
        {
            Debug.Log(
                $"[MaterialItem] {item.id} / {item.displayName} " +
                $"(cat: {item.category}, rarity: {item.rarity}, " +
                $"type: {item.materialType}, value: {item.value}, " +
                $"hardness: {item.hardness}, flammability: {item.flammability})");
        }
    }

    // Convenience methods: Find items by ID
    public FoodItemDataSO FindFoodItem(string id)
    {
        foreach (var item in foodItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public DrinkItemDataSO FindDrinkItem(string id)
    {
        foreach (var item in drinkItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public MaterialItemDataSO FindMaterialItem(string id)
    {
        foreach (var item in materialItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public WeaponItemDataSO FindWeaponItem(string id)
    {
        foreach (var item in weaponItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public WeaponComponentItemDataSO FindWeaponComponentItem(string id)
    {
        foreach (var item in weaponComponentItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public AmmoItemDataSO FindAmmoItem(string id)
    {
        foreach (var item in ammoItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }
}

