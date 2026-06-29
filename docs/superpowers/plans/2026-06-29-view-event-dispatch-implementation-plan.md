# View Event Dispatch Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a minimal `View` event hook and route runtime input/focus events through it.

**Architecture:** `WindowRuntime` remains responsible for lifecycle and rendering events. `View` gains a default no-op event handler that receives non-lifecycle `PlatformEvent`s plus `WindowRuntimeContext`.

**Tech Stack:** C++23, xmake, existing fake platform/window/renderer UI tests.

---

### Task 1: Add Event Dispatch Tests

**Files:**
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] **Step 1: Write failing tests**

Add `RecordingView::handle_event(...)` to record delivered events, then add tests for non-lifecycle input/focus dispatch and event-driven redraw.

- [x] **Step 2: Verify RED**

Run: `xmake f -c -m debug -P .; xmake test -P . window_runtime_test/default`

Expected: compile failure because `View::handle_event` does not exist yet.

### Task 2: Add Runtime Support

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [x] **Step 1: Add API**

Declare `WindowRuntimeContext` before `View`, then add:

```cpp
virtual void handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context);
```

- [x] **Step 2: Implement dispatch**

Add the no-op default implementation and call `view_.handle_event(event, context())` for events that are not close, resize, or redraw once the runtime has both a window and renderer.

### Task 3: Verify And Commit

- [x] Run targeted tests:
  `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- [x] Run full Windows tests:
  `xmake f -c -m debug -P .; xmake test -P .`
- [x] Run full WSL Arch Linux tests:
  `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/view-event-dispatch && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
- [x] Commit with message `feat: dispatch view events`.
