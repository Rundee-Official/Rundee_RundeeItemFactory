# QA Guardrails

## Duplicate / ID
- Enforce type prefixes (Food_/Drink_/Material_/Weapon_/WeaponComponent_/Ammo_)
- Skip duplicates on merge/import; log skipped IDs
- Exclusion list passed to LLM; log any filtered duplicates

## Value ranges (baseline)
- Hunger/Thirst/Health: >= 0
- SpoilTime (Food/Drink): > 0 and reasonable (e.g., 15–2880 minutes)
- Weapon: damage/accuracy/recoil/durability within design bounds (adjust as needed)
- Material: hardness/value non-negative; flammability 0–100
- Ammo: penetration/value non-negative; caliber non-empty

## Text / Tone
- Reject/flag profanity or placeholder terms (“dummy”, “lorem”, “test item”)
- Category consistency: category must match item type; weaponType/componentType present where needed

## Rarity / Distribution
- Warn if >70% of items in one rarity (unless preset-specific)
- Flag extreme stats inconsistent with rarity

## Logging & Warnings
- QualityChecker prints warnings; count warnings per type in logs
- Fail fast on JSON parse errors; keep `.raw.json` for audit

## Unity Import
- On import, skip duplicates, log missing fields, and stop on parse errors
- Spot-check assets after import; ensure required fields are serialized


