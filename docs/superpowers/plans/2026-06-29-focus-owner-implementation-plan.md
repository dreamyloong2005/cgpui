# Focus Owner Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a root view identity and keyboard focus owner state.

**Architecture:** `WindowRuntime` owns a stable root `ViewId` and stores the optional keyboard focus owner separately from raw pointer/window input state. Each `WindowRuntimeContext` receives the current root `view_id` plus an input snapshot whose `keyboard_focused` flag is computed for that view.

**Tech Stack:** C++23, xmake, existing fake platform/window/renderer UI tests.

---

### Task 1: RED Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Extend `RecordingView`**

Store `context.view_id`, `context.input.keyboard_focus_owner`, and extra booleans for wrong-owner release and owner release observations.

- [x] **Step 2: Add Focus Owner Assertions**

Add a runtime test that dispatches key/text events, requests keyboard focus for `context.view_id`, attempts to release a different `ViewId`, then releases the actual owner.

- [x] **Step 3: Verify RED**

Run: `xmake test -P . window_runtime_test/default`

Expected: compile failure because `ViewId`, `WindowRuntimeContext::view_id`, and `ViewInputState::keyboard_focus_owner` do not exist yet.

### Task 2: Runtime Identity And Owner State

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add Public Types And API**

Add `ViewId`, `WindowRuntimeContext::view_id`, `ViewInputState::keyboard_focus_owner`, and owner-aware focus overloads while preserving the existing no-argument methods.

- [x] **Step 2: Implement Snapshot Semantics**

Store `keyboard_focus_owner_` in `WindowRuntime`, reset it in `run()`, set it on request, clear it only when the releasing id owns focus, and compute `keyboard_focused` inside `context()`.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/focus-owner && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: add keyboard focus owner`.
