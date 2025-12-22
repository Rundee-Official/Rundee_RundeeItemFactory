# Dynamic Profile System

## Overview

The Dynamic Profile System allows users to create custom item structures for item generation. Instead of being limited to hardcoded item types, you can define your own item fields, types, and validation rules.

## Key Concepts

### Profiles

A **Profile** defines the structure of items:
- Field definitions (name, type, validation rules)
- Default values
- Field categories and display order
- Metadata

### Fields

Each **Field** in a profile defines:
- **Name**: JSON key name (e.g., "hungerRestore")
- **Type**: Data type (String, Integer, Float, Boolean, Array, Object)
- **Display Name**: Human-readable name for UI
- **Category**: Grouping for organization
- **Validation**: Rules (min/max values, required flag, allowed values, etc.)
- **Default Value**: Default value if not provided

## Creating Profiles

### Using Unity Editor (Recommended)

1. Open **Tools > Rundee > Item Factory > Tools > Item Profile Manager**

2. Select item type (Food, Weapon, etc.)

3. Click **"New Profile"**

4. Edit profile details:
   - Display Name
   - Description
   - ID (must be unique)

5. Add fields:
   - Click **"Add Field"**
   - Configure field properties:
     - Name (JSON key)
     - Type (String, Integer, etc.)
     - Validation rules
     - Default value

6. Click **"Save Profile"**

### Profile File Format

Profiles are stored as JSON files in the `profiles/` directory:

```json
{
  "id": "my_custom_food",
  "displayName": "My Custom Food Profile",
  "description": "Custom food items with special fields",
  "itemTypeName": "Food",
  "version": 1,
  "isDefault": false,
  "fields": [
    {
      "name": "id",
      "type": "string",
      "displayName": "ID",
      "description": "Unique item identifier",
      "category": "Identity",
      "displayOrder": 0,
      "isRequired": true,
      "defaultValue": "",
      "minLength": 1,
      "maxLength": 0
    },
    {
      "name": "hungerRestore",
      "type": "integer",
      "displayName": "Hunger Restore",
      "description": "Amount of hunger restored",
      "category": "Effects",
      "displayOrder": 1,
      "isRequired": true,
      "defaultValue": 0,
      "minValue": 0,
      "maxValue": 100
    }
  ],
  "metadata": {}
}
```

## Field Types

### String
Text data (e.g., "id", "displayName", "description")
- Validation: minLength, maxLength, allowedValues

### Integer
Whole numbers (e.g., 0, 10, 100)
- Validation: minValue, maxValue

### Float
Decimal numbers (e.g., 1.5, 3.14)
- Validation: minValue, maxValue

### Boolean
True/false values
- No additional validation

### Array
List of values
- Validation: minLength (minimum array size), maxLength (maximum array size)

### Object
Nested JSON object
- Use for complex nested structures

## Using Profiles for Generation

### Command Line

```bash
RundeeItemFactory.exe --mode llm --itemType food --profile my_custom_food --model llama3 --count 10
```

### Unity Editor

1. Open **Item Factory Window**
2. Select item type
3. Choose profile from dropdown (or use default)
4. Generate items normally

The LLM will generate items matching the profile structure.

## Default Profiles

Built-in item types (Food, Weapon, etc.) have default profiles that match the original hardcoded structures. These are automatically created on first run and stored as:
- `profiles/default_food.json`
- `profiles/default_weapon.json`
- etc.

You can:
- Use default profiles as-is
- Copy and modify them
- Create entirely new profiles

## Validation

Profiles include validation rules that are enforced during:
1. **Profile creation/editing**: Unity UI validates profiles
2. **Item parsing**: Generated items are validated against profile
3. **Generation**: LLM receives validation rules in prompts

### Validation Types

- **Required fields**: Must be present
- **Type checking**: Values must match field type
- **Range validation**: Min/max values for numeric types
- **Length validation**: String/array length limits
- **Allowed values**: Enum-like restrictions (e.g., rarity: Common/Uncommon/Rare)

## Best Practices

1. **Start with defaults**: Copy default profiles and modify rather than creating from scratch

2. **Use categories**: Group related fields (Identity, Stats, Effects, etc.)

3. **Set display order**: Control field order in UI and prompts

4. **Provide descriptions**: Help LLM understand field purpose

5. **Set validation rules**: Prevent invalid values at generation time

6. **Test incrementally**: Start with simple profiles, add complexity gradually

## Migration from Hardcoded Types

Existing hardcoded item types are automatically converted to default profiles:
- No code changes needed
- Existing JSON files remain compatible
- Can gradually migrate to custom profiles

## Examples

### Example 1: Simple Custom Food

Add a "calories" field to food items:

```json
{
  "name": "calories",
  "type": "integer",
  "displayName": "Calories",
  "description": "Calorie content",
  "category": "Nutrition",
  "minValue": 0,
  "maxValue": 10000
}
```

### Example 2: Weapon with Custom Stats

Add "durability" and "repairCost" fields:

```json
{
  "fields": [
    {
      "name": "durability",
      "type": "integer",
      "displayName": "Durability",
      "minValue": 0,
      "maxValue": 100,
      "defaultValue": 100
    },
    {
      "name": "repairCost",
      "type": "integer",
      "displayName": "Repair Cost",
      "minValue": 0,
      "defaultValue": 0
    }
  ]
}
```

### Example 3: Items with Tags

Add a "tags" array field:

```json
{
  "name": "tags",
  "type": "array",
  "displayName": "Tags",
  "description": "Item tags for filtering",
  "category": "Metadata",
  "minLength": 0,
  "maxLength": 10
}
```

## Limitations

1. **Nested objects**: Complex nested structures may require manual JSON editing
2. **Array item types**: Arrays are currently homogeneous (all elements same type)
3. **Dynamic Unity SO**: Unity ScriptableObject generation requires profile-aware importer

## Future Enhancements

- Nested object field definitions
- Typed array elements
- Field dependencies (conditional fields)
- Profile versioning and migration
- Profile templates and presets



