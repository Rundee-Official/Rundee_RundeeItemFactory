# RundeeItemFactory

LLM-powered item generator for Unity & Unreal Engine games.

Automatically generates game item JSON data (Food, Drink, Material, Weapon, WeaponComponent, Ammo) using local LLM (Ollama) and imports them into Unity ScriptableObjects.

## Features

- **Local LLM Integration**: Uses Ollama for local LLM interaction
- **Multiple Item Types**: Food, Drink, Material, Weapon, WeaponComponent, and Ammo items
- **Preset System**: Forest, Desert, Coast, City, and Default presets
- **Unity Integration**: Automatic ScriptableObject generation with EditorWindow GUI
- **Balance Reporting**: Statistical analysis of generated items with Quick Summary
- **Unity Balance Report Integration**: Generate balance reports directly from Unity Editor
- **Improved UI**: Organized sections with helpful tooltips and progress indicators
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

### Prompt Template System

All LLM prompts are loaded from external text files so you can tweak wording without recompiling.

- Default templates live under `RundeeItemFactory/prompts/` inside the repo.
- When you build the project, the templates are copied next to `RundeeItemFactory.exe` inside an automatically created `prompts/` directory (for example `x64/Debug/prompts/` during debug builds).
- At runtime the generator loads `<exe>/prompts/<itemType>.txt`. If the file is missing the previous built-in prompt is used as a fallback.

#### Available template placeholders

| Placeholder | Description |
|-------------|-------------|
| `{ITEM_TYPE}` | Friendly item type name (e.g., `Food`, `WeaponComponent`) |
| `{PRESET_NAME}` | Active preset name (`Default`, `Forest`, custom preset display name, etc.) |
| `{PRESET_CONTEXT}` | Long-form flavor text describing the preset |
| `{MAX_HUNGER}` | Maximum hunger parameter (Food/Drink only) |
| `{MAX_THIRST}` | Maximum thirst parameter (Food/Drink only) |
| `{COUNT}` | Number of items requested |
| `{EXCLUDE_IDS}` | Formatted list of IDs to avoid (empty when nothing to exclude) |

Feel free to embed additional narrative instructions, balancing notes, or formatting hints in these files. If you need more placeholders, extend `PromptTemplateLoader::LoadTemplate` with the desired variable.

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

### Batch Mode

Run multiple generation jobs sequentially with one command:

```bash
RundeeItemFactory.exe ^
  --mode batch ^
  --batch "food:10:items_food.json,drink:5,material:8:items_material.json" ^
  --model llama3 ^
  --preset city
```

- Use `--batch "itemType:count[:outputPath],..."`.
- Each entry inherits global arguments (model, preset, maxHunger, etc.) but can override the output path.
- Batch logs show per-job progress, elapsed time, and a final summary table. Failures do not stop the rest of the jobs, so you can re-run only the ones that failed.

### Unity Integration

#### Using the Item Factory Window (Recommended)

1. Open Unity Editor
2. Go to **Tools > Rundee > Item Factory Window**
3. The window is organized into collapsible sections:

   **Configuration Section:**
   - **Executable Path**: Path to `RundeeItemFactory.exe` (auto-detected if in standard location)
   - Shows status indicator when executable is found

   **Generation Parameters Section:**
   - **Model**: LLM model name (e.g., `llama3`, `llama3:8b`, or custom)
   - **Preset**: World preset with helpful descriptions:
     - Default: Generic survival environment
     - Forest: Natural resources, wild foods
     - Desert: Scarce resources, preserved foods
     - Coast: Seafood, fresh water sources
     - City: Processed foods, modern weapons
   - **Item Type**: Food, Drink, Material, Weapon, Weapon Component, or Ammo
   - **Count**: Number of items to generate (recommended: 10-20 for testing, 50-100 for production)
   - **Max Hunger/Thirst**: Maximum values for balancing (Food/Drink only)
   - **Output File**: Path where JSON will be saved (shows file size if exists)
   - **Auto Import**: Automatically import items after generation

4. Click **Generate Items**
5. Monitor progress:
   - Progress bar shows generation status
   - Log area displays real-time output
   - Cancel button available during generation
6. Items will be automatically imported if "Auto Import" is enabled

**Balance Report Feature:**
- Expand the "Balance Report" section
- Select a JSON file and item type
- Click "Generate Report" to view statistical analysis
- Reports include Quick Summary, distribution stats, and balance warnings

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
- `Tools/Rundee/Item Manager` - Item overview, selection, and cleanup

#### Item Manager Window

`Tools > Rundee > Item Manager` provides a searchable overview of every generated ScriptableObject:

- Filter by item type, rarity, or keyword (matches ID/display name).
- Ping the underlying asset in the Project window or bulk-delete obsolete items.
- Helpful when pruning prototypes before re-importing fresh data.

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
Weapons are categorized as either "Ranged" or "Melee". Ranged weapons use ammo and have caliber, while Melee weapons have inherent damage and melee-specific stats.

```json
{
  "id": "Weapon_ak47",
  "displayName": "AK-47 Assault Rifle",
  "category": "Weapon",
  "rarity": "Rare",
  "maxStack": 1,
  "weaponCategory": "Ranged",
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
  "muzzleVelocity": 715,
  "effectiveRange": 400,
  "penetrationPower": 60,
  "moddingSlots": 4,
  "attachmentSlots": [
    {"slotType": "Muzzle", "slotIndex": 0, "isRequired": false},
    {"slotType": "Grip", "slotIndex": 0, "isRequired": false},
    {"slotType": "Sight", "slotIndex": 0, "isRequired": false},
    {"slotType": "Magazine", "slotIndex": 0, "isRequired": true}
  ],
  "description": "A reliable assault rifle with high damage."
}
```

**Melee Weapon Example:**
```json
{
  "id": "Weapon_combat_knife",
  "displayName": "Combat Knife",
  "category": "Weapon",
  "rarity": "Common",
  "maxStack": 1,
  "weaponCategory": "Melee",
  "weaponType": "Knife",
  "caliber": "",
  "minDamage": 20,
  "maxDamage": 30,
  "fireRate": 120,
  "accuracy": 80,
  "recoil": 0,
  "ergonomics": 70,
  "weight": 200,
  "durability": 100,
  "muzzleVelocity": 0,
  "effectiveRange": 0,
  "penetrationPower": 0,
  "attackSpeed": 3,
  "reach": 50,
  "staminaCost": 10,
  "moddingSlots": 0,
  "attachmentSlots": [],
  "description": "A sharp combat knife for close-quarters combat."
}
```

### Weapon Component Items
Weapon components modify weapon stats when attached. Magazines can describe their exact load order with mixed ammo types. All magazines of the same caliber support mixed ammo by default.

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

Presets define the world context for item generation, influencing the types and characteristics of generated items.

### Default
Generic early-game survival environment with moderate resources. Items should feel simple and grounded, not magical or high-tech.

### Forest
Temperate forest environment with natural resources, wild foods, and organic materials. Items include berries, nuts, roots, and simple cooked meals.

### Desert
Harsh desert environment with scarce resources, preserved foods, and heat-resistant materials. Food items are low in quantity but efficient.

### Coast
Coastal environment with seafood, fresh water sources, and marine materials. Items include coconuts, fish, shellfish, and tropical fruits.

### City
Urban environment with processed foods, manufactured items, and modern weapons. Items include canned goods, energy drinks, and advanced weaponry.

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

Generate statistical reports for generated items to analyze balance and distribution patterns.

### Command Line

```bash
RundeeItemFactory.exe --report items_food.json --itemType food
RundeeItemFactory.exe --report items_weapon.json --itemType weapon
RundeeItemFactory.exe --report items_weaponcomponent.json --itemType weaponcomponent
RundeeItemFactory.exe --report items_ammo.json --itemType ammo
```

### Unity Editor

1. Open **Tools > Rundee > Item Factory Window**
2. Expand the **Balance Report** section
3. Select a JSON file and item type
4. Click **Generate Report**
5. View the report in the scrollable text area

### Report Contents

Each report includes:

**Quick Summary:**
- Key statistics at a glance
- Average values for important stats
- Distribution overview

**Detailed Statistics:**
- Total item count
- Rarity distribution (with percentages)
- Average/Min/Max for all stats
- Type-specific distributions (weapon types, calibers, component types, etc.)

**Balance Warnings:**
- Detects unrealistic values
- Highlights potential balance issues
- Provides recommendations

**Example Output:**
```
========================================
   WEAPON ITEMS BALANCE REPORT
========================================
Total Items: 25

--- Quick Summary ---
  Average Damage: 42.5
  Average Fire Rate: 580 RPM
  Average Accuracy: 68/100
  Average Weight: 3200g (3.2 kg)
  Weapon Categories: 18 Ranged, 7 Melee

--- Rarity Distribution ---
  Common    :  8 (32.0%)
  Uncommon  : 10 (40.0%)
  Rare      :  7 (28.0%)

--- Balance Warnings ---
  [OK] No balance issues detected.
========================================
```

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
