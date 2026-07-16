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
