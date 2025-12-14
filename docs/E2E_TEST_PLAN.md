# End-to-End Test Scenario

This checklist verifies that the full authoring pipeline still works after template, batch, and UI changes.

## Prerequisites

- `RundeeItemFactory.exe` built (Debug or Release).
- `prompts/` folder copied next to the executable (done automatically by the build).
- Unity project `UnityRundeeItemFactory` open in the Editor.
- Ollama server running locally (`ollama serve`).

## Steps

1. **Clean old assets**
   - In Unity, open `Tools > Rundee > Item Manager`.
   - Select `Food` tab, click `Delete Selected` (or confirm that folder is empty).
   - Repeat for other item types to avoid stale artifacts.

2. **Generate JSON from CLI**
   ```powershell
   cd <repo>/RundeeItemFactory/x64/Debug
   .\RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 10 --preset forest --out e2e_food.json
   ```
   - Confirm log shows template info (model/timestamp/existing count).
   - Ensure exit code = 0 and JSON file created.

3. **Import into Unity**
   - Use menu `Tools > Rundee > Item Factory > JSON Importer`.
   - Item type: `Food`, File: `e2e_food.json`.
   - Verify new ScriptableObjects under `Assets/Resources/RundeeItemFactory/FoodItems`.

4. **Validate via Item Manager**
   - Open `Tools > Rundee > Item Manager`.
   - Filter by Food, confirm 10 new entries with proper rarity/description.
   - Ping a few assets to ensure they open correctly.

5. **Runtime sanity check**
   - In a test scene, add `ItemDatabase` component.
   - Press Play; console should show loaded counts (no missing references).

6. **Cleanup**
   - Remove temporary JSON (`e2e_food.json`) or archive if needed.
   - Optionally delete generated ScriptableObjects via Item Manager to leave project clean.

## Notes

- Repeat the same flow for other item types if prompts/templates were edited.
- For regression tracking, capture screenshots of CLI output, Item Manager view, and in-editor assets.







