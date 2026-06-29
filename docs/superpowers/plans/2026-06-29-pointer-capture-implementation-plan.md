# Pointer Capture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add runtime-level pointer capture state and APIs.

**Architecture:** `WindowRuntime` owns a boolean capture flag inside `ViewInputState`. Views request changes through `WindowRuntimeContext::runtime`, and later event contexts expose the updated state.

**Tech Stack:** C++23, xmake, existing fake platform/window/renderer UI tests.

---

### Task 1: RED Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Extend RecordingView**

Add test controls that call `context.runtime.capture_pointer()` on the first pointer event and `context.runtime.release_pointer()` on a later pointer event.

- [x] **Step 2: Add Capture Assertions**

Add tests proving a later event sees `context.input.pointer_captured == true` after capture, and `false` after release.

- [x] **Step 3: Verify RED**

Run: `xmake test -P . window_runtime_test/default`

Expected: compile failure because `WindowRuntime` has no pointer capture API and `ViewInputState` has no `pointer_captured`.

### Task 2: Runtime Support

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add API And State**

Add `bool pointer_captured` to `ViewInputState` and declare `capture_pointer()` / `release_pointer()` on `WindowRuntime`.

- [x] **Step 2: Implement Capture**

Set or clear `input_.pointer_captured` in those methods; preserve existing input state updates and reset all input state at `run()` start.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/pointer-capture && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: add pointer capture state`.
