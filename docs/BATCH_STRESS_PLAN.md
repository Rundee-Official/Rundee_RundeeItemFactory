# Batch Mode Stress Test Plan

Goal: ensure batch execution stays stable for large workloads (50–200 items) across multiple item types.

## Test Matrix

| Scenario | Description |
|----------|-------------|
| S1 | Single item type, 100 entries (e.g., food) |
| S2 | Mixed types (food/drink/material) totaling 150 entries |
| S3 | Weapon-heavy batch (weapon + weaponComponent + ammo) |
| S4 | Mixed batch with intentional failure (invalid custom preset path) to validate summary reporting |

## Recommended Commands

```powershell
.\RundeeItemFactory.exe ^
  --mode batch ^
  --batch "food:100:stress_food.json,drink:50:stress_drink.json,material:30" ^
  --model llama3 ^
  --preset city
```

- Adjust counts per scenario.
- Use `--maxHunger` / `--maxThirst` overrides when testing extremes.

## Metrics to Capture

1. **Per-job duration** – from new summary table in CLI output.
2. **Total duration** – overall batch time reported.
3. **Success vs failure counts** – verify non-zero failures are surfaced.
4. **LLM retry behavior** – observe if any job triggers automatic retries.
5. **System resources** – monitor CPU/RAM/GPU (optional but helpful for tuning).

## Validation Steps

1. After batch run, inspect summary table: ensure job ordering and file paths are accurate.
2. Spot-check generated JSON files (especially largest ones) for schema compliance.
3. Import a subset into Unity via **Tools > Rundee > Item Factory > JSON Importer** to ensure data integrity.
4. Use Item Manager to bulk-delete artifacts between runs to keep project tidy.

## Failure Handling

- If a job fails, the batch summary now lists `FAIL` with exit code; record the index and re-run only the failed item by specifying a smaller batch string (e.g., `--batch "material:30:stress_material.json"`).
- Keep raw logs for each scenario (`powershell.exe > batch_s1.log`) for later comparison.

## Acceptance Criteria

- All scenarios complete without crashes.
- CLI summary matches expectations (counts, durations).
- Generated assets import cleanly into Unity.
- No leftover temporary files other than intended JSON outputs.







