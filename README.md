# RundeeItemFactory

LLM-powered command-line tool for generating game item JSON data using local LLM (Ollama).

Automatically generates game item JSON data (Food, Drink, Material, Weapon, WeaponComponent, Ammo) using local LLM (Ollama) with dynamic profile system support.

## Features

- **Local LLM Integration**: Uses Ollama for local LLM interaction
- **Dynamic Profile System**: Create custom item structures with flexible field definitions, validation rules, and relationships
- **Player Profile System**: Define player stat settings and stat sections that influence item generation and balancing
- **Multiple Item Types**: Food, Drink, Material, Weapon, WeaponComponent, and Ammo items
- **World Context System**: Define custom world context in Item Profiles for flexible item generation
- **Dynamic Prompt Generation**: LLM prompts are dynamically built from profile data, ensuring all field definitions, validation rules, player settings, and custom context are included
- **Retry Logic**: Automatic retry on LLM failures with exponential backoff
- **ID Prefixing**: Automatic type prefixes (Food_, Drink_, Material_, Weapon_, WeaponComponent_, Ammo_)
- **ID Registry System**: Prevents duplicate IDs across generations
- **JSON Merging**: Automatically merges new items with existing files, skipping duplicates

## Requirements

- **Windows 10/11** (64-bit)
- **Visual Studio 2019 or later** (with C++ workload)
- **Ollama**: Install from [ollama.ai](https://ollama.ai)

## Installation

### 1. Build from Source

1. Clone this repository
2. Open `RundeeItemFactory/RundeeItemFactory.sln` in Visual Studio
3. Build the project (Release or Debug)
4. The executable will be in `RundeeItemFactory/RundeeItemFactory/x64/Release/` (or Debug)

### 2. Install Ollama

1. Download Ollama from https://ollama.ai/download
2. Run the installer
3. Pull a model (we recommend llama3):
   ```powershell
   ollama pull llama3
   ```

### 3. Prepare Profiles

Create JSON profile files in the following directories (relative to the executable):

- `ItemProfiles/` - Item profile JSON files
- `PlayerProfiles/` - Player profile JSON files

## Configuration

Optional config file: `config/rundee_config.json`

Controls Ollama host/port, retries, and HTTP timeouts. The file is copied next to `RundeeItemFactory.exe` on build so runtime picks it up automatically.

Example:
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

## Usage

### Command Line

#### Basic Usage

Generate items using a custom item profile:
```bash
RundeeItemFactory.exe --mode llm --model llama3 --count 10 --profile your_profile_id --playerProfile your_player_profile_id --out items.json
```

**Note:** Item types and structures are defined in Item Profiles, not hardcoded. Create your own Item Profile to define custom item types and fields.

#### Using Default Profiles

If you have default profiles set up, you can use `--itemType` to select a default profile:
```bash
RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 10 --out items_food.json
```

**Note:** The `--itemType` argument is only used to find default profiles. The actual item structure is defined in the Item Profile's `itemTypeName` field and field definitions.

#### Example with Custom Profile

```bash
RundeeItemFactory.exe --mode llm --model llama3 --count 10 --profile realistic_firearms --playerProfile default_player --out items_weapon.json
```

This example uses:
- `realistic_firearms` - A custom Item Profile that defines weapon structure and fields
- `default_player` - A Player Profile that defines player stat settings
- The Item Profile's `itemTypeName` field determines the item type (e.g., "Weapon", "Food", or any custom type you define)

### Command Line Arguments

| Argument | Description | Default |
|----------|-------------|---------|
| `--mode` | Generation mode: `llm` | `llm` |
| `--model` | Ollama model name | `llama3` |
| `--count` | Number of items to generate | `5` |
| `--profile` | Item profile ID to use (from `ItemProfiles/` directory). **Required for custom item types.** | Uses default for `--itemType` if specified |
| `--playerProfile` | Player profile ID to use (from `PlayerProfiles/` directory) | Uses default player profile |
| `--itemType` | Legacy argument: Used to find default profiles. Item structure is actually defined in the Item Profile. | `food` |
| `--out` | Output JSON filename (saved to `ItemJson/` directory) | `items_food.json` |
| `--maxHunger` | Maximum hunger value for player stats | From player profile |
| `--maxThirst` | Maximum thirst value for player stats | From player profile |
| `--additionalPrompt` | Additional prompt text to append to LLM request | - |
| `--test` or `--testMode` | Enable test mode | `false` |

**Important Notes:**
- **Item types are user-defined**: Create Item Profiles to define your own item types and structures. The `--itemType` argument is only a legacy way to find default profiles.
- The `--out` argument specifies only the filename. All output files are automatically saved to the `ItemJson/` directory relative to the executable.
- World context is defined in Item Profiles using the "World Context / Background" field.
- The `--preset` argument is deprecated. Use Item Profile's custom context instead.

## Dynamic Profile System

The system uses a **Dynamic Profile System** where LLM prompts are automatically generated from item profiles. This ensures:

- **Complete Data Transmission**: All profile data (fields, validation rules, relationships, custom context, metadata) is included in the prompt
- **No Data Loss**: Every field definition, constraint, allowed value, and custom field is passed to the LLM
- **Flexible Structure**: Create custom item structures without code changes
- **Validation Rules**: Field validation rules (min/max, allowed values, relationships) are included in prompts
- **Player Context**: Player profile settings (max stats) and stat sections are included for balance context
- **Duplicate Prevention**: Existing IDs from the registry are included to prevent duplicates
- **Descriptive IDs**: Automatically generates short, descriptive item IDs (e.g., `weapon_FNScar17S`) from display names

### Item Profiles

Item Profiles define:
- Field definitions with types and validation rules
- Relationship constraints between fields
- Allowed values and default values
- Custom context for LLM guidance (World Context / Background)
- Metadata for additional information

Profile files should be placed in `ItemProfiles/` directory and named as `{profile_id}.json`.

### Player Profiles

Player Profiles define:
- Player stat settings (max hunger, thirst, health, stamina, weight, energy)
- Stat sections with nested fields and values
- Display order and organization
- Descriptions and metadata

Profile files should be placed in `PlayerProfiles/` directory and named as `{profile_id}.json`.

## Project Structure

```
RundeeItemFactory/
├── RundeeItemFactory/          # C++ Visual Studio project
│   ├── include/                # Header files
│   │   ├── Data/               # Item/Player data structures, profiles
│   │   ├── Parsers/            # JSON parsing
│   │   ├── Writers/            # JSON writing
│   │   ├── Generators/         # Item generation
│   │   ├── Clients/            # LLM client (Ollama)
│   │   ├── Prompts/            # Prompt building (DynamicPromptBuilder)
│   │   ├── Helpers/            # Command line parsing, utilities
│   │   └── Utils/              # Utility functions
│   ├── src/                    # Implementation files
│   ├── config/                 # Configuration files
│   └── RundeeItemFactory.sln   # Visual Studio solution
├── LICENCES/                   # Third-party licenses
├── README.md                   # This file
└── LICENSE                     # Project license
```

### Runtime Structure

When running the executable, the following directory structure is expected:

```
RundeeItemFactory.exe location/
├── RundeeItemFactory.exe       # Main executable
├── config/
│   └── rundee_config.json      # Optional configuration
├── ItemProfiles/               # Item profile JSON files
│   └── {profile_id}.json
├── PlayerProfiles/             # Player profile JSON files
│   └── {profile_id}.json
├── ItemJson/                   # Generated item JSON files (created by .exe)
└── Registry/                   # ID registry for duplicate prevention (created by .exe)
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

### Example: Material Items
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

### Example: Weapon Items
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

### Example: Weapon Component Items
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

### Example: Ammo Items
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

## World Context

World context defines the environment and setting for item generation, influencing the types and characteristics of generated items. This is configured in Item Profiles using the "World Context / Background" field, allowing for complete customization without code changes.

### Example Contexts

- **Survival**: Generic early-game survival environment with moderate resources. Items should feel simple and grounded, not magical or high-tech.
- **Forest**: Temperate forest environment with natural resources, wild foods, and organic materials.
- **Desert**: Harsh desert environment with scarce resources, preserved foods, and heat-resistant materials.
- **Coastal**: Coastal environment with seafood, fresh water sources, and marine materials.
- **Urban**: Urban environment with processed foods, manufactured items, and modern weapons.

You can define any world context in your Item Profiles to match your game's setting and requirements.

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

## Troubleshooting

### Ollama not found
- Ensure Ollama is installed and in your PATH
- Test with: `ollama run llama3 "test"`
- Verify Ollama service is running: `ollama list`

### Empty JSON response
- Check if Ollama is running
- Try a different model
- Check network connection (if using remote Ollama)
- Verify the model is installed: `ollama list`

### Profile not found
- Ensure profile JSON files are in the correct directories (`ItemProfiles/` or `PlayerProfiles/`)
- Check that the profile ID matches the filename (without .json extension)
- Verify the JSON file is valid

### Build errors
- Ensure Visual Studio 2019 or later is installed
- Install "Desktop development with C++" workload
- Check that all dependencies are properly configured

## License

Copyright (c) 2025 Haneul Lee. All rights reserved.

## Contributing

This is a personal project. For questions or suggestions, please contact the author.
