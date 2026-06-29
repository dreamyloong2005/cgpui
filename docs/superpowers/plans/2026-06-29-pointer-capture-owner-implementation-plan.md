# Pointer Capture Owner Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add owner-aware pointer capture state using `ViewId`.

**Architecture:** `WindowRuntime` stores pointer capture ownership as `std::optional<ViewId>` beside the existing keyboard focus owner. Each `WindowRuntimeContext` receives an input snapshot whose `pointer_captured` flag is computed for the current root view from the owner value.

**Tech Stack:** C++23, xmake, existing fake platform/window/renderer UI tests.

---

### Task 1: RED Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Extend `RecordingView`**

Store `context.input.pointer_capture_owner`, add controls for owner capture, wrong-owner release, and owner release.

- [x] **Step 2: Add Pointer Capture Owner Assertions**

Add a runtime test that dispatches pointer move events, captures for `context.view_id`, attempts to release a different `ViewId`, then releases the actual owner.

- [x] **Step 3: Verify RED**

Run: `xmake test -P . window_runtime_test/default`

Expected: compile failure because `ViewInputState::pointer_capture_owner` and owner-aware pointer capture overloads do not exist yet.

### Task 2: Runtime Owner State

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add Public State And API**

Add `pointer_capture_owner`, `capture_pointer(ViewId)`, and `release_pointer(ViewId)` while preserving existing no-argument methods.

- [x] **Step 2: Implement Snapshot Semantics**

Store `pointer_capture_owner_` in `WindowRuntime`, reset it in `run()`, set it on capture, clear it only when the releasing id owns capture, and compute `pointer_captured` inside `context()`.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/pointer-capture-owner && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: add pointer capture owner`.
