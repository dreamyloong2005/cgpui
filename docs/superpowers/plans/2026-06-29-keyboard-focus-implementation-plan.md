# Keyboard Focus Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add runtime-level keyboard focus state and APIs.

**Architecture:** `WindowRuntime` owns a boolean keyboard focus flag inside `ViewInputState`. Views request changes through `WindowRuntimeContext::runtime`, and later event contexts expose the updated state.

**Tech Stack:** C++23, xmake, existing fake platform/window/renderer UI tests.

---

### Task 1: RED Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Extend RecordingView**

Add test controls that call `context.runtime.request_keyboard_focus()` during one event and `context.runtime.release_keyboard_focus()` during a later event.

- [x] **Step 2: Add Focus Assertions**

Add tests proving a later keyboard event sees `context.input.keyboard_focused == true` after focus request, and `false` after release.

- [x] **Step 3: Verify RED**

Run: `xmake test -P . window_runtime_test/default`

Expected: compile failure because `WindowRuntime` has no keyboard focus API and `ViewInputState` has no `keyboard_focused`.

### Task 2: Runtime Support

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add API And State**

Add `bool keyboard_focused` to `ViewInputState` and declare `request_keyboard_focus()` / `release_keyboard_focus()` on `WindowRuntime`.

- [x] **Step 2: Implement Focus**

Set or clear `input_.keyboard_focused` in those methods; preserve existing input state updates and reset all input state at `run()` start.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/keyboard-focus && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: add keyboard focus state`.
