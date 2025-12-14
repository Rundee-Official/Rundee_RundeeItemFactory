# Metrics Plan

## What to Capture
- Generation time per item type (LLM duration)
- LLM failures/retries per type
- JSON parse failures
- QA warnings per type (from QualityChecker)
- Import success/fail counts (Unity)
- Batch summary: per-job exit codes and durations

## How to Capture (current tooling)
- C++: existing stdout logs + add counters per type (LLM calls, retries, parse failures, QA warnings)
- Script: `scripts/run_smoke_tests.ps1` runs all types + balance report + batch; logs NDJSON to `.cursor/debug.log` (runId: run-smoke-2)
- Unity: Importer logs duplicate skips and parse failures in Console

## Targets / Alerts (initial)
- LLM success rate: > 95% (retries included)
- Parse failures: 0
- QA warnings: < 5% of generated items per type
- Import failures: 0
- Generation time (2 items): food/drink/material < 10s, weapon/component/ammo < 20s (per attempt)

## Reporting
- After a run, parse `.cursor/debug.log` entries with `runId=run-smoke-2`:
  - `run_llm:exit` per type
  - `run_report:exit`
  - `run_batch:exit`
  - `ollama_ping`
- Summarize counts: success/fail, duration if available, warnings (from stdout)



