# RundeeItemFactory

LLM-powered item generator for Unity & Unreal Engine games.

Automatically generates game item JSON data (Food, Drink, Material, Weapon, WeaponComponent, Ammo) using local LLM (Ollama) and imports them into Unity ScriptableObjects.

## Features

- **Local LLM Integration**: Uses Ollama for local LLM interaction
- **Dynamic Profile System**: Create custom item structures with flexible field definitions, validation rules, and relationships
- **Multiple Item Types**: Food, Drink, Material, Weapon, WeaponComponent, and Ammo items
- **Preset System**: Forest, Desert, Coast, City, and Default presets with custom preset support
- **Dynamic Prompt Generation**: LLM prompts are dynamically built from profile data, ensuring all field definitions, validation rules, and player settings are included
- **Unity Integration**: Automatic ScriptableObject generation with EditorWindow GUI
- **Improved UI**: Organized sections with helpful tooltips and progress indicators
- **Retry Logic**: Automatic retry on LLM failures with exponential backoff
- **ID Prefixing**: Automatic type prefixes (Food_, Drink_, Material_, Weapon_, WeaponComponent_, Ammo_)
- **ID Registry System**: Prevents duplicate IDs across generations
- **JSON Merging**: Automatically merges new items with existing files, skipping duplicates

## Requirements

- **Ollama**: Install from [ollama.ai](https://ollama.ai)
- **C++ Compiler**: Visual Studio 2019 or later (Windows)
- **Unity**: 2020.3 or later (for Unity integration)

## Configuration

- Optional config file: `config/rundee_config.json`
- Controls Ollama host/port, retries, and HTTP timeouts. The file is copied next to `RundeeItemFactory.exe` on build so runtime picks it up automatically.
- Example:
  ```json
  {
    "ollama": {
      "host": "localhost",
      "port": 11434,
      "maxRetries": 3,
      "requestTimeoutSeconds": 120
    }
  }
  ```

## Installation

1. Clone this repository
2. Open `RundeeItemFactory/RundeeItemFactory.sln` in Visual Studio
3. Build the project (Release or Debug)
4. Ensure Ollama is installed and running

For detailed installation instructions, see [INSTALLATION_GUIDE.md](docs/INSTALLATION_GUIDE.md)

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

### Dynamic Profile System

The system uses a **Dynamic Profile System** where LLM prompts are automatically generated from item profiles. This ensures:

- **Complete Data Transmission**: All profile data (fields, validation rules, relationships, player settings) is included in the prompt
- **No Data Loss**: Every field definition, constraint, and allowed value is passed to the LLM
- **Flexible Structure**: Create custom item structures without code changes
- **Validation Rules**: Field validation rules (min/max, allowed values, relationships) are included in prompts
- **Player Context**: Player settings (max stats) are included for balance context
- **Duplicate Prevention**: Existing IDs from the registry are included to prevent duplicates

Profiles define:
- Field definitions with types and validation rules
- Relationship constraints between fields
- Allowed values and default values
- Player settings for balance context
- Custom context for LLM guidance

See [Dynamic Profile System Documentation](docs/DYNAMIC_PROFILE_SYSTEM.md) for detailed information.

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
| `--report` | Generate balance report (planned, not yet implemented) | - |

### Unity Integration

#### Using the Item Factory Window (Recommended)

1. Open Unity Editor
2. Go to **Tools > Rundee > Item Factory > Item Factory Window**
3. (Optional) Open the **Setup (Ollama)** foldout to verify Ollama is installed. If not detected on Windows, run the bundled installer (.bat) directly from the window.
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

**Note:** Balance Report feature is available via command line (see below). Unity Editor integration is planned for future releases.

#### Manual Import (Unified)

1. Generate JSON files using the command line tool.
2. In Unity Editor, open **Tools > Rundee > Item Factory > JSON Importer**.
3. Choose the target **Item type** (Food, Drink, Material, Weapon, Weapon Component, Ammo).
4. Pick the JSON file and click **Import Selected JSON**.
5. ScriptableObjects will be created automatically in `Assets/Resources/RundeeItemFactory/[Type]Items/`.

#### Available Unity Menu Items

**Main Windows:**
- `Tools/Rundee/Item Factory/Item Factory` - Main window for generating items and managing settings
- `Tools/Rundee/Item Factory/Import JSON` - Import JSON files and convert to ScriptableObjects

**Setup:**
- `Tools/Rundee/Item Factory/Ollama Setup` - Install Ollama on Windows

**Note:** Additional management tools (Item Manager, Registry Manager, etc.) are planned for future releases.

## Documentation

Available documentation in the `docs/` directory:

- **[Dynamic Profile System](docs/DYNAMIC_PROFILE_SYSTEM.md)** — Complete guide to creating and using custom item profiles
- **[Installation Guide](docs/INSTALLATION_GUIDE.md)** — Detailed setup instructions
- **[Unity Import Guide](docs/UNITY_IMPORT_GUIDE.md)** — Unity integration and usage
- **[Deployment Structure](docs/DEPLOYMENT_STRUCTURE.md)** — Deployment package structure and scenarios
- **[Asset Store Setup](docs/ASSET_STORE_SETUP.md)** — Unity Asset Store distribution guide
- **[Deployment Strategy](docs/DEPLOYMENT_STRATEGY.md)** — Deployment strategies and best practices

## Project Structure

```
RundeeItemFactory/
├── RundeeItemFactory/          # C++ Visual Studio project
│   ├── include/                # Header files
│   │   ├── Data/               # Item data structures, profiles
│   │   ├── Validators/         # Item validation logic
│   │   ├── Parsers/            # JSON parsing
│   │   ├── Writers/            # JSON writing
│   │   ├── Generators/         # Item generation
│   │   ├── Clients/             # LLM client (Ollama)
│   │   ├── Prompts/             # Prompt building (DynamicPromptBuilder)
│   │   ├── Helpers/             # Command line parsing, utilities
│   │   └── Utils/               # Utility functions
│   ├── src/                     # Implementation files
│   ├── config/                  # Configuration files
│   └── RundeeItemFactory.sln    # Visual Studio solution
├── docs/                        # Documentation
│   ├── DYNAMIC_PROFILE_SYSTEM.md
│   ├── INSTALLATION_GUIDE.md
│   ├── UNITY_IMPORT_GUIDE.md
│   └── ...
├── LICENCES/                    # Third-party licenses
├── README.md                     # This file
└── LICENSE                       # Project license

Note: The following folders are for testing/development and are not tracked in git:
- scripts/                       # Build and test scripts
- UnityRundeeItemFactory/        # Unity test project
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

## Balance Report (Planned)

Balance report feature is planned for future releases. This will provide statistical analysis of generated items to analyze balance and distribution patterns.

### Command Line

```bash
RundeeItemFactory.exe --report items_food.json --itemType food
RundeeItemFactory.exe --report items_weapon.json --itemType weapon
RundeeItemFactory.exe --report items_weaponcomponent.json --itemType weaponcomponent
RundeeItemFactory.exe --report items_ammo.json --itemType ammo
```

### Unity Editor

Balance Report feature is planned for future releases. Currently, you can analyze generated JSON files manually or use external tools.

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
