# QA Pilot Checklist (50–100 items per type)

## Scope
- Item types: Food, Drink, Material, Weapon, WeaponComponent, Ammo
- Presets: default + 1–2 (e.g., forest, city)
- Volume: 50–100 items per type (per preset)

## Preconditions
- Ollama running (HTTP API reachable at http://localhost:11434)
- Prompts present next to `RundeeItemFactory.exe`
- Unity project open; Resources folders clean for import tests

## Generation Steps
1) For each preset/type, run LLM generation (count 50–100) to JSON.
2) Capture CLI logs (stdout/stderr) and generation time.
3) Keep raw `.raw.json` for audit.

## Import Steps (Unity)
1) Use JSON Importer to import each generated JSON.
2) Confirm ScriptableObjects created under `Assets/Resources/RundeeItemFactory/<Type>Items`.
3) Spot-check 5–10 assets per type: fields populated, IDs unique, no missing references.

## QA Checks (per type)
- IDs: unique, non-empty, type-prefixed correctly
- Required fields present (id/displayName/category/rarity/maxStack)
- Value ranges:
  - Hunger/Thirst/Health: non-negative; within design bounds
  - Weapon stats: damage/accuracy/recoil/durability within expected ranges
  - Material hardness/flammability/value reasonable
- Rarity distribution: no extreme skew unless preset-specific
- Spoilage: reasonable times (Food/Drink)
- Ammo: caliber present; penetration/value plausible
- WeaponComponent: compatibleSlots/subSlots non-empty when applicable

## Quality/Guardrails
- Disallow empty/duplicate IDs (import should skip duplicates with log)
- Banlist check: names/description should not include profanity or placeholder text (“dummy”, “lorem”, etc.)
- Category consistency: category matches item type
- Attachment slots (weapon): must align with weapon type semantics
- Balance warnings: capture any warnings from QualityChecker

## Metrics to Record
- Generation time per type
- LLM call failures/retries
- JSON parse failures
- QA warnings count per type
- Import success/fail count

## Exit Criteria (per type)
- 0 parse failures
- 0 hard errors on import
- QA warnings <= agreed threshold (e.g., <5% of items) or all reviewed/accepted
- No duplicate IDs after import


