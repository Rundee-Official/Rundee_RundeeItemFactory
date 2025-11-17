# RundeeItemFactory

LLM-powered item generator for Unity & Unreal Engine games.

Automatically generates game item JSON data (Food, Drink, Material, Weapon, WeaponComponent, Ammo) using local LLM (Ollama) and imports them into Unity ScriptableObjects.

## Features

- **Local LLM Integration**: Uses Ollama for local LLM interaction
- **Multiple Item Types**: Food, Drink, Material, Weapon, WeaponComponent, and Ammo items
- **Preset System**: Forest, Desert, Coast, City, and Default presets
- **Unity Integration**: Automatic ScriptableObject generation with EditorWindow GUI
- **Balance Reporting**: Statistical analysis of generated items
- **Retry Logic**: Automatic retry on LLM failures with exponential backoff
- **ID Prefixing**: Automatic type prefixes (Food_, Drink_, Material_, Weapon_, WeaponComponent_, Ammo_)
- **JSON Merging**: Automatically merges new items with existing files, skipping duplicates

## Requirements

- **Ollama**: Install from [ollama.ai](https://ollama.ai)
- **C++ Compiler**: Visual Studio 2019 or later (Windows)
- **Unity**: 2020.3 or later (for Unity integration)

## Installation

1. Clone this repository
2. Open `RundeeItemFactory.sln` in Visual Studio
3. Build the project (Release or Debug)
4. Ensure Ollama is installed and running

## Usage

### Command Line

#### Generate Food Items
```bash
RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 10 --preset city --out items_food.json
```

#### Generate Drink Items
```bash
RundeeItemFactory.exe --mode llm --itemType drink --model llama3 --count 10 --preset coast --out items_drink.json
```

#### Generate Material Items
```bash
RundeeItemFactory.exe --mode llm --itemType material --model llama3 --count 10 --preset forest --out items_material.json
```

#### Generate Weapon Items
```bash
RundeeItemFactory.exe --mode llm --itemType weapon --model llama3 --count 10 --preset city --out items_weapon.json
```

#### Generate Weapon Component Items
```bash
RundeeItemFactory.exe --mode llm --itemType weaponcomponent --model llama3 --count 10 --preset city --out items_weapon_component.json
```

#### Generate Ammo Items
```bash
RundeeItemFactory.exe --mode llm --itemType ammo --model llama3 --count 10 --preset city --out items_ammo.json
```

#### Generate Balance Report
```bash
RundeeItemFactory.exe --report items_food.json --itemType food
```

#### Dummy Mode (for testing without LLM)
```bash
RundeeItemFactory.exe --mode dummy --itemType food --count 5 --out items_dummy.json
```

### Command Line Arguments

| Argument | Description | Default |
|----------|-------------|---------|
| `--mode` | Generation mode: `llm` or `dummy` | `dummy` |
| `--itemType` | Item type: `food`, `drink`, `material`, `weapon`, `weaponcomponent`, or `ammo` | `food` |
| `--model` | Ollama model name | `llama3` |
| `--count` | Number of items to generate | `5` |
| `--preset` | World preset: `default`, `forest`, `desert`, `coast`, `city` | `default` |
| `--maxHunger` | Maximum hunger value | `100` |
| `--maxThirst` | Maximum thirst value | `100` |
| `--out` | Output JSON file path | `items_food.json` |
| `--report` | Generate balance report for existing JSON file | - |

### Unity Integration

#### Using the Item Factory Window (Recommended)

1. Open Unity Editor
2. Go to **Tools > Rundee > Item Factory Window**
3. Configure the generation parameters:
   - **Executable Path**: Path to `RundeeItemFactory.exe` (auto-detected if in standard location)
   - **Model**: LLM model name (e.g., `llama3`)
   - **Preset**: World preset (Default, Forest, Desert, Coast, City)
   - **Item Type**: Food, Drink, Material, Weapon, Weapon Component, or Ammo
   - **Count**: Number of items to generate
   - **Max Hunger/Thirst**: Maximum values for balancing
   - **Output File**: Path where JSON will be saved
   - **Auto Import**: Automatically import items after generation
4. Click **Generate Items**
5. Monitor progress in the log area
6. Items will be automatically imported if "Auto Import" is enabled

#### Manual Import

1. Generate JSON files using the command line tool
2. In Unity Editor, go to **Tools > Rundee > Import [Type] Items From JSON**
3. Select the generated JSON file
4. ScriptableObjects will be created automatically in `Assets/Resources/RundeeItemFactory/[Type]Items/`

#### Available Unity Menu Items

- `Tools/Rundee/Item Factory Window` - GUI for generating and importing items
- `Tools/Rundee/Import Food Items From JSON` - Manual import for Food items
- `Tools/Rundee/Import Drink Items From JSON` - Manual import for Drink items
- `Tools/Rundee/Import Material Items From JSON` - Manual import for Material items
- `Tools/Rundee/Import Weapon Items From JSON` - Manual import for Weapon items
- `Tools/Rundee/Import Weapon Component Items From JSON` - Manual import for Weapon Component items
- `Tools/Rundee/Import Ammo Items From JSON` - Manual import for Ammo items

#### Using ItemDatabase

Add the `ItemDatabase` component to a GameObject in your scene. It will automatically load all items from Resources on `Awake()`.

```csharp
ItemDatabase db = GetComponent<ItemDatabase>();

// Find items by ID
FoodItemDataSO food = db.FindFoodItem("Food_apple");
DrinkItemDataSO drink = db.FindDrinkItem("Drink_water");
MaterialItemDataSO material = db.FindMaterialItem("Material_wood_plank");
WeaponItemDataSO weapon = db.FindWeaponItem("Weapon_ak47");
WeaponComponentItemDataSO component = db.FindWeaponComponentItem("WeaponComponent_suppressor");
AmmoItemDataSO ammo = db.FindAmmoItem("Ammo_9mm");
```

## Project Structure

```
RundeeItemFactory/
├── include/
│   ├── Data/              # Item data structures
│   ├── Validators/        # Item validation logic
│   ├── Parsers/           # JSON parsing
│   ├── Writers/           # JSON writing
│   ├── Generators/        # Item generation
│   ├── Clients/           # LLM client (Ollama)
│   ├── Prompts/           # Prompt building
│   ├── Helpers/           # Command line parsing, item generation
│   └── Utils/             # Utility functions
└── src/                   # Implementation files

UnityRundeeItemFactoryTest/
└── Assets/RundeeItemFactory/
    ├── Editor/            # Unity Editor scripts
    └── Runtime/           # Runtime scripts
```

## Item Data Structure

### Food/Drink Items
```json
{
  "id": "Food_apple",
  "displayName": "Red Apple",
  "category": "Food",
  "rarity": "Common",
  "maxStack": 10,
  "hungerRestore": 15,
  "thirstRestore": 5,
  "healthRestore": 0,
  "spoils": true,
  "spoilTimeMinutes": 1440,
  "description": "A fresh red apple."
}
```

### Material Items
```json
{
  "id": "Material_wood_plank",
  "displayName": "Wood Plank",
  "category": "Material",
  "rarity": "Common",
  "maxStack": 50,
  "materialType": "Wood",
  "hardness": 30,
  "flammability": 60,
  "value": 10,
  "description": "A sturdy wooden plank."
}
```

### Weapon Items
```json
{
  "id": "Weapon_ak47",
  "displayName": "AK-47 Assault Rifle",
  "category": "Weapon",
  "rarity": "Rare",
  "maxStack": 1,
  "weaponType": "AssaultRifle",
  "caliber": "7.62mm",
  "minDamage": 45,
  "maxDamage": 55,
  "fireRate": 600,
  "accuracy": 65,
  "recoil": 45,
  "ergonomics": 50,
  "weight": 3500,
  "durability": 100,
  "magazineCapacity": 30,
  "magazineType": "Standard",
  "attachmentSlots": [
    {"slotType": "Muzzle", "slotIndex": 0, "isRequired": false},
    {"slotType": "Grip", "slotIndex": 0, "isRequired": false},
    {"slotType": "Sight", "slotIndex": 0, "isRequired": false}
  ],
  "description": "A reliable assault rifle with high damage."
}
```

### Weapon Component Items
Magazines can now describe their exact load order. Each entry in `loadedRounds` represents a contiguous block of rounds (top-to-bottom) with a specific ammo ID.

```json
{
  "id": "WeaponComponent_mk3_drum",
  "displayName": "MK3 7.62 Drum",
  "category": "WeaponComponent",
  "rarity": "Rare",
  "maxStack": 1,
  "componentType": "Magazine",
  "compatibleSlots": ["Magazine"],
  "magazineCapacity": 75,
  "caliber": "7.62mm",
  "magazineType": "Drum",
  "loadedRounds": [
    {
      "orderIndex": 0,
      "roundCount": 15,
      "ammoId": "Ammo_762_AP",
      "ammoDisplayName": "7.62 AP",
      "ammoNotes": "Breaching stack"
    },
    {
      "orderIndex": 1,
      "roundCount": 45,
      "ammoId": "Ammo_762_Ball",
      "ammoDisplayName": "7.62 FMJ",
      "ammoNotes": "General purpose"
    },
    {
      "orderIndex": 2,
      "roundCount": 15,
      "ammoId": "Ammo_762_Tracer",
      "ammoDisplayName": "7.62 Tracer",
      "ammoNotes": "Low ammo indicator"
    }
  ],
  "damageModifier": 0,
  "recoilModifier": -5,
  "ergonomicsModifier": -10,
  "accuracyModifier": 0,
  "weightModifier": 500,
  "hasBuiltInRail": false,
  "railType": "",
  "description": "Heavy drum mag with staged AP/FMJ/Tracer load."
}
```

### Ammo Items
```json
{
  "id": "Ammo_9mm",
  "displayName": "9mm Parabellum",
  "category": "Ammo",
  "rarity": "Common",
  "maxStack": 60,
  "caliber": "9mm",
  "damageBonus": 0,
  "penetration": 25,
  "accuracyBonus": 0,
  "recoilModifier": 0,
  "armorPiercing": false,
  "hollowPoint": false,
  "tracer": false,
  "incendiary": false,
  "value": 15,
  "description": "Standard 9mm ammunition."
}
```

## Presets

### Default
Generic early-game survival environment with moderate resources.

### Forest
Temperate forest with berries, nuts, roots, and simple cooked meals.

### Desert
Harsh desert with scarce vegetation. Food items are low in quantity but efficient.

### Coast
Coastal island with coconuts, fish, shellfish, and tropical fruits.

### City
Abandoned modern city with processed food, canned goods, and energy drinks.

## Retry Logic

The tool automatically retries LLM calls up to 3 times if:
- The response is empty
- The response doesn't look like valid JSON
- The command fails

Retry delays use exponential backoff (2, 4, 6 seconds).

## JSON Merging

When generating items to an existing file, the tool automatically:
- Reads existing items from the file
- Filters out duplicate IDs from new items
- Merges unique new items with existing ones
- Ensures the requested count is met by generating additional items if needed

This allows you to incrementally build your item database without losing existing data.

## Balance Report

Generate statistical reports for generated items:

```bash
RundeeItemFactory.exe --report items_food.json --itemType food
RundeeItemFactory.exe --report items_weapon.json --itemType weapon
RundeeItemFactory.exe --report items_ammo.json --itemType ammo
```

The report includes:
- Total item count
- Rarity distribution
- Average/Min/Max stats
- Balance warnings

## Troubleshooting

### Ollama not found
- Ensure Ollama is installed and in your PATH
- Test with: `ollama run llama3 "test"`

### Empty JSON response
- Check if Ollama is running
- Try a different model
- Check network connection (if using remote Ollama)

### Unity import fails
- Ensure JSON file is valid
- Check file path
- Verify Unity Editor console for errors

## License

Copyright (c) 2025 Haneul Lee. All rights reserved.

## Contributing

This is a personal project. For questions or suggestions, please contact the author.
