# Usage Examples

## Basic Examples

### Generate 10 Food Items (City Preset)
```bash
RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 10 --preset city --out items_food_city.json
```

### Generate 5 Drink Items (Coast Preset)
```bash
RundeeItemFactory.exe --mode llm --itemType drink --model llama3 --count 5 --preset coast --out items_drink_coast.json
```

### Generate 20 Material Items (Forest Preset)
```bash
RundeeItemFactory.exe --mode llm --itemType material --model llama3 --count 20 --preset forest --out items_material_forest.json
```

### Generate 10 Weapon Items (City Preset)
```bash
RundeeItemFactory.exe --mode llm --itemType weapon --model llama3 --count 10 --preset city --out items_weapon_city.json
```

### Generate 15 Weapon Component Items (City Preset)
```bash
RundeeItemFactory.exe --mode llm --itemType weaponcomponent --model llama3 --count 15 --preset city --out items_weapon_component_city.json
```

### Generate 20 Ammo Items (City Preset)
```bash
RundeeItemFactory.exe --mode llm --itemType ammo --model llama3 --count 20 --preset city --out items_ammo_city.json
```

## Testing Without LLM (Dummy Mode)

### Generate Dummy Food Items
```bash
RundeeItemFactory.exe --mode dummy --itemType food --count 10 --out items_dummy_food.json
```

This generates items without calling LLM, useful for testing the pipeline.

## Balance Reports

### Check Food Item Balance
```bash
RundeeItemFactory.exe --report items_food_city.json --itemType food
```

### Check Drink Item Balance
```bash
RundeeItemFactory.exe --report items_drink_coast.json --itemType drink
```

### Check Material Item Balance
```bash
RundeeItemFactory.exe --report items_material_forest.json --itemType material
```

### Check Weapon Item Balance
```bash
RundeeItemFactory.exe --report items_weapon_city.json --itemType weapon
```

### Check Weapon Component Item Balance
```bash
RundeeItemFactory.exe --report items_weapon_component_city.json --itemType weaponcomponent
```

### Check Ammo Item Balance
```bash
RundeeItemFactory.exe --report items_ammo_city.json --itemType ammo
```

## Complete Workflow

### Step 1: Generate Items
```bash
RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 15 --preset desert --out items_food_desert.json
```

### Step 2: Check Balance
```bash
RundeeItemFactory.exe --report items_food_desert.json --itemType food
```

### Step 3: Import to Unity
1. Open Unity Editor
2. Go to `Tools/Rundee/Import Food Items From JSON`
3. Select `items_food_desert.json`
4. Check `Assets/Resources/RundeeItemFactory/FoodItems/` for generated ScriptableObjects

## Advanced Examples

### Custom Parameters
```bash
RundeeItemFactory.exe --mode llm --itemType food --model llama3.2:3b --count 25 --preset city --maxHunger 150 --maxThirst 120 --out items_custom.json
```

### Different Models
```bash
# Using llama3.2:3b (smaller, faster)
RundeeItemFactory.exe --mode llm --itemType food --model llama3.2:3b --count 10 --out items_food.json

# Using mistral (alternative model)
RundeeItemFactory.exe --mode llm --itemType food --model mistral --count 10 --out items_food.json
```

## Troubleshooting Examples

### Test Ollama Connection
```bash
ollama run llama3 "test"
```

If this works, Ollama is properly installed.

### Test Dummy Generation (No LLM Required)
```bash
RundeeItemFactory.exe --mode dummy --itemType food --count 5 --out test_dummy.json
```

This should always work and helps verify the tool is built correctly.

### Check Generated JSON
```bash
# View the raw LLM response
type items_food.json.raw.json

# View the processed JSON
type items_food.json
```

## Unity Integration Examples

### C# Script Example
```csharp
using UnityEngine;

public class ItemManager : MonoBehaviour
{
    private ItemDatabase itemDatabase;

    void Start()
    {
        itemDatabase = GetComponent<ItemDatabase>();
        
        // Find specific items
        FoodItemDataSO apple = itemDatabase.FindFoodItem("Food_apple");
        if (apple != null)
        {
            Debug.Log($"Found: {apple.displayName}, Hunger: +{apple.hungerRestore}");
        }
        
        WeaponItemDataSO weapon = itemDatabase.FindWeaponItem("Weapon_ak47");
        if (weapon != null)
        {
            Debug.Log($"Found: {weapon.displayName}, Damage: {weapon.minDamage}-{weapon.maxDamage}");
        }
        
        AmmoItemDataSO ammo = itemDatabase.FindAmmoItem("Ammo_9mm");
        if (ammo != null)
        {
            Debug.Log($"Found: {ammo.displayName}, Penetration: {ammo.penetration}");
        }
        
        // Iterate through all items
        foreach (var food in itemDatabase.foodItems)
        {
            Debug.Log($"[Food] {food.id}: {food.displayName}");
        }
        
        foreach (var weapon in itemDatabase.weaponItems)
        {
            Debug.Log($"[Weapon] {weapon.id}: {weapon.displayName} ({weapon.weaponType})");
        }
        
        foreach (var ammo in itemDatabase.ammoItems)
        {
            Debug.Log($"[Ammo] {ammo.id}: {ammo.displayName} ({ammo.caliber})");
        }
    }
}
```

## Batch Generation Script (Windows)

Create `generate_all.bat`:
```batch
@echo off
echo Generating all item types...

RundeeItemFactory.exe --mode llm --itemType food --count 20 --preset city --out items_food.json
RundeeItemFactory.exe --mode llm --itemType drink --count 15 --preset coast --out items_drink.json
RundeeItemFactory.exe --mode llm --itemType material --count 30 --preset forest --out items_material.json
RundeeItemFactory.exe --mode llm --itemType weapon --count 10 --preset city --out items_weapon.json
RundeeItemFactory.exe --mode llm --itemType weaponcomponent --count 15 --preset city --out items_weapon_component.json
RundeeItemFactory.exe --mode llm --itemType ammo --count 20 --preset city --out items_ammo.json

echo Done! Check the JSON files.
pause
```

## Expected Output

### Successful Generation
```
[Main] Mode = LLM, itemType = food, model = llama3, count = 10, out = items_food.json
[Main] Preset = city
[ItemGenerator] Running in LLM mode.
[OllamaClient] Attempt 1 of 3
[OllamaClient] Running command: ollama run llama3 "..."
[OllamaClient] Successfully received response on attempt 1
=== Ollama Food JSON Response ===
[... JSON output ...]
[ItemJsonParser] Parsed 10 food items from JSON.
[ItemGenerator] Wrote LLM-generated items to items_food.json
```

### Balance Report Output
```
========================================
   FOOD ITEMS BALANCE REPORT
========================================
Total Items: 10

--- Rarity Distribution ---
  Common    : 5 (50.0%)
  Uncommon  : 3 (30.0%)
  Rare      : 2 (20.0%)

--- Hunger Restore Stats ---
  Average: 25
  Min:     10
  Max:     45

--- Balance Warnings ---
  [OK] No balance issues detected.
========================================
```

