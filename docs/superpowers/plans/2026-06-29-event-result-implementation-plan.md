# Event Result Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add result-returning event handling to the UI runtime.

**Architecture:** `View::handle_event` returns `EventResult`. `WindowRuntime` stores the most recent view result and exposes it through `WindowRuntimeContext`, while preserving default behavior for existing views.

**Tech Stack:** C++23, xmake, existing fake platform/window/renderer UI tests.

---

### Task 1: RED Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Extend `RecordingView`**

Return `EventResult::consumed()` for a configured pointer event and `EventResult::cancelled()` for a configured keyboard event.

- [x] **Step 2: Add Event Result Assertions**

Add a test proving later contexts expose the result of the previous view-dispatched event.

- [x] **Step 3: Verify RED**

Run: `xmake test -P . window_runtime_test/default`

Expected: compile failure because `EventResult` and `WindowRuntimeContext::last_event_result` do not exist yet.

### Task 2: Runtime Support

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add Public Type And Signature**

Add `EventResult`, change `View::handle_event` to return it, and add `WindowRuntimeContext::last_event_result`.

- [x] **Step 2: Store Results**

Have `WindowRuntime::handle_event` assign the result from `view_.handle_event(event, context())` to runtime state.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/core-event-result && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: add event result`.
