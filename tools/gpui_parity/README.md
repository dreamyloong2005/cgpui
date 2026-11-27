# GPUI Parity Tooling

`extract_upstream_symbols.py` records a deterministic snapshot from the pinned
upstream GPUI revision and writes a machine-readable summary that can be
compared with `docs/gpui-complete-parity-ledger.json`.

The tool is intentionally lightweight for Phase A. It does not attempt a full
Rust parser yet. It fetches the pinned raw upstream files when the network is
available and falls back to the pinned example inventory when it is not. It
extracts:

- public re-export lines from `crates/gpui/src/gpui.rs`;
- public module lines from the same crate root;
- example names from `crates/gpui/Cargo.toml`;
- the pinned revision and source URLs used by the ledger.

Run from the repository root:

```powershell
python tools\gpui_parity\extract_upstream_symbols.py --output build\gpui_parity_snapshot.json
```

The authoritative status export remains:

```text
docs/gpui-complete-parity-ledger.json
```

Phase J strict audit:

```powershell
python tools\gpui_parity\extract_upstream_symbols.py --require-live --output docs\gpui-upstream-snapshot.json
python tools\gpui_parity\audit_upstream_snapshot.py --snapshot docs\gpui-upstream-snapshot.json --ledger docs\gpui-complete-parity-ledger.json --output docs\gpui-phase-j-upstream-audit.json
```

Strict mode fails unless both pinned upstream source files were fetched live.
The snapshot records content hashes, counts, and fetch mode; the auditor checks
the exact pinned inventory and rejects unresolved required ledger rows.

## Performance workload sensitivity

`check_performance_workload_sensitivity.py` verifies **actual compiled runner
binaries** through their existing CLI. It neither builds nor edits sources and
is **not automatically run by the ordinary mcpp test suite**. Run it explicitly
when changing the performance runner, its workload observations, or publication
gate. This checks failure sensitivity, not performance budgets or native GPU
coverage; retain the normal platform baseline validator for budget checks.

Prepare three binaries serially, using the same toolchain/profile:

1. Build the unmodified `tests/performance` runner with
   `scripts/dev/test.ps1 -p tests/performance` (or `scripts/dev/test.sh`), then
   preserve the resulting executable as the normal binary.
2. Save the exact bytes of `tests/performance/baseline_scheduling_workloads.cpp`.
   Change only its task-completion loop from
   `for (const auto& task : view.tasks)` to
   `for (const auto& task : std::span{view.tasks}.first(0))`, rebuild, and preserve
   the resulting executable as the zero-work binary. Keep all 64 task spawns,
   the expected count of 64, observations, and the other seven workloads intact.
3. Repeat with `.first(32)` for the partial-work binary. The two mutated test
   runs must exit nonzero **after a successful build**; a compiler error or
   crash is not evidence of workload rejection.
4. Restore the original source bytes even if a build fails, rebuild the normal
   runner, and verify its normal run again. Do not run mutations concurrently
   with another build or source edit. Keep any required runtime libraries and
   dependency environment available to the preserved binaries.

From the repository root, substitute the three preserved executable paths:

```powershell
python tools/gpui_parity/check_performance_workload_sensitivity.py `
  --normal-runner <normal.exe> --zero-runner <zero.exe> `
  --partial-runner <partial.exe> --platform windows --iterations 4
```

The check requires the normal report's eight metrics and actual counts
(`4` for the first six, `64` tasks, `10` animation ticks). Each mutation is run
against both a fresh output path and an existing sentinel file: it must exit
`4`, diagnose `async_wakeup` as `0/64` or `32/64`, create no new report, and leave
the existing file unchanged. The script prints executable hashes and all five
CLI outcomes for the caller's evidence log; it creates no production fault flag
and never fabricates sample observations to test the validator in isolation.
