# GPUI Parity Tooling

`extract_upstream_symbols.py` records a small, deterministic snapshot from the
pinned upstream GPUI revision and writes a machine-readable summary that can be
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

Future phases can replace the line-oriented extractor with a proper Rust AST
or rustdoc JSON pass, but the output contract should remain stable: upstream
revision, public re-exports, modules, example names, and source URLs.
