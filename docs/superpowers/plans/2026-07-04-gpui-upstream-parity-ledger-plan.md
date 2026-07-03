# GPUI Upstream Parity Ledger Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete Steps 219-258 by pinning upstream GPUI and establishing a durable parity ledger, extractor, status export, and first example/API parity gate.

**Architecture:** Keep Phase A as documentation, tooling, and tests. The ledger is the source of truth; later implementation phases close ledger rows instead of inventing new scope from memory.

**Tech Stack:** C++23, Xmake, PowerShell, Python 3 standard library, official Zed GPUI sources pinned at `5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`.

---

### Task 1: Steps 219-222 Upstream Pin

**Files:**
- Create: `docs/gpui-upstream-pinned-revision.md`

- [x] Record upstream repository, commit hash, GPUI source tree URL, raw README,
  crate root, context docs, key dispatch docs, and the first example URL.
- [x] Record crate versions: `gpui = 0.2.2` and `gpui_platform = 0.1.0`.
- [x] Record active platform interpretation: Windows Win32 + Vulkan, Linux
  Wayland + Vulkan, macOS Cocoa + Metal later, X11 deferred.

### Task 2: Steps 223-226 Extractor

**Files:**
- Create: `tools/gpui_parity/README.md`
- Create: `tools/gpui_parity/extract_upstream_symbols.py`

- [x] Add a deterministic Phase A extractor with `GPUI_UPSTREAM_REVISION`.
- [x] Implement `extract_public_reexports(...)`.
- [x] Implement `extract_examples(...)`.
- [x] Add a CLI that writes JSON to `--output`.

### Task 3: Steps 227-238 Ledger And Categories

**Files:**
- Create: `docs/gpui-complete-parity-ledger.md`
- Create: `docs/gpui-complete-parity-ledger.json`

- [x] Map the first upstream concepts to CGPUI headers, examples, tests, and
  next phases.
- [x] Split rows into `Required`, `Adapted`, `Deferred`, and `Non-goal`.
- [x] Keep X11 explicitly deferred unless strict upstream Linux backend parity
  is later accepted by the user.
- [x] Add machine-readable status counts, examples, platform targets, and rows.

### Task 4: Steps 231-234 And 247-250 API Parity Gate

**Files:**
- Create: `tests/api_parity/gpui_parity_ledger_test.cpp`
- Modify: `xmake.lua`

- [x] Run RED:

```powershell
xmake test -y -P . gpui_parity_ledger_test/default
```

Expected: FAIL before Phase A files exist.

- [x] Run GREEN after docs/tooling/example are added:

```powershell
xmake test -y -P . gpui_parity_ledger_test/default
```

Expected: PASS.

### Task 5: Steps 239-246 Parity Doc And Hello World Example

**Files:**
- Modify: `docs/gpui-core-api-parity.md`
- Create: `examples/api_parity/hello_world/main.cpp`
- Modify: `xmake.lua`

- [x] Update the core parity audit so it points to the Step 258 ledger instead
  of the old Step 168-only wording.
- [x] Add a compileable C++ hello-world parity example mapping upstream
  `application().run`, `App::open_window`, `Render`, `Context<T>`, `div`,
  `px`, `rgb`, bounds, and child composition to current CGPUI public APIs.
- [x] Register the `api_parity_hello_world` build target.

### Task 6: Steps 251-258 Verification And Closeout

**Files:**
- Modify: `task_plan.md`
- Modify: `findings.md`
- Modify: `progress.md`

- [x] Build the hello-world parity example.
- [x] Run the extractor and validate JSON output.
- [x] Run focused Windows tests.
- [x] Run focused WSL tests.
- [x] Run Windows full debug.
- [x] Run WSL full debug.
- [x] Run `git diff --check`.
- [x] Update durable planning files and mark Phase A complete.

Verification evidence captured on 2026-07-04:

- Windows extractor:
  `python tools\gpui_parity\extract_upstream_symbols.py --output build\gpui_parity_snapshot.json`
  exited 0.
- Windows JSON validation:
  `python -m json.tool build\gpui_parity_snapshot.json` exited 0 and showed
  55 public re-exports plus 20 examples from the pinned upstream snapshot.
- Windows focused tests:
  `xmake test -y -P . gpui_parity_ledger_test/default` passed 1/1.
- Windows parity example:
  `xmake build -y -P . api_parity_hello_world` exited 0.
- WSL extractor:
  `python tools/gpui_parity/extract_upstream_symbols.py --output build/gpui_parity_snapshot_wsl.json`
  exited 0.
- WSL JSON validation:
  `python -m json.tool build/gpui_parity_snapshot_wsl.json` exited 0.
- WSL focused tests:
  `XMAKE_ROOT=y xmake test -y -P . gpui_parity_ledger_test/default` passed
  1/1.
- WSL parity example:
  `XMAKE_ROOT=y xmake build -y -P . api_parity_hello_world` exited 0.
- Windows full debug:
  `xmake f -c -m debug -P .` exited 0 and `xmake test -P .` passed 42/42.
- WSL full debug:
  `XMAKE_ROOT=y xmake f -y -c -m debug -P .` exited 0 and
  `XMAKE_ROOT=y xmake test -y -P .` passed 39/39.
- `git diff --check` exited 0 with only expected LF-to-CRLF normalization
  warnings for touched text files.

## Phase A complete

Phase A is complete only when `gpui_parity_ledger_test/default`, the
`api_parity_hello_world` build, extractor JSON generation, Windows full debug,
WSL full debug, and `git diff --check` all pass.
