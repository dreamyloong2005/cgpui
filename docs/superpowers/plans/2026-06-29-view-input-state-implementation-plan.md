# View Input State Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a lightweight input snapshot to `WindowRuntimeContext`.

**Architecture:** `WindowRuntime` owns mutable `ViewInputState`. It updates that state when platform input/focus events arrive, then dispatches the event to the active `View` with a context containing the latest snapshot.

**Tech Stack:** C++23, xmake, existing `window_runtime_test` fake platform.

---

### Task 1: RED Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Extend RecordingView**

Record `context.input.focused` and `context.input.pointer_position` in `RecordingView::handle_event`.

- [x] **Step 2: Add Assertions**

Assert focus is true after a `WindowFocused{true}` event and pointer position matches the latest pointer event delivered to the view.

- [x] **Step 3: Verify RED**

Run: `xmake test -P . window_runtime_test/default`

Expected: compile failure because `WindowRuntimeContext` has no `input` member yet.

### Task 2: Runtime Input State

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add Types And Storage**

Add `ViewInputState` with `bool focused` and `Point pointer_position`; add it to `WindowRuntimeContext` and `WindowRuntime`.

- [x] **Step 2: Update State Before Dispatch**

Reset input state at the start of `WindowRuntime::run`, update focus/pointer state for input events, then dispatch to the view with the updated context.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/view-input-state && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: expose view input state`.
