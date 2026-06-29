# Window Runtime Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a reusable `WindowRuntime` that owns the platform window/render loop now embedded in the hello example.

**Architecture:** Implement the runtime in the UI layer because it bridges platform events, renderer lifecycle, and view painting. Keep platform backends unchanged and verify behavior with pure fake objects.

**Tech Stack:** C++23, xmake, existing `cgpui::Result`, `PlatformApplication`, `Renderer`, and `View` interfaces.

---

### Task 1: Runtime Tests

**Files:**
- Create: `tests/ui/window_runtime_test.cpp`
- Modify: `xmake.lua`

- [ ] Write a failing test that constructs fake platform/rendering objects and calls `cgpui::WindowRuntime`.
- [ ] Run `xmake build -P . window_runtime_test`; expected failure is missing `WindowRuntime`.

### Task 2: Runtime Implementation

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`

- [ ] Add `WindowRuntimeOptions`, `WindowRuntime`, and `WindowRuntime::run`.
- [ ] Route `WindowResized`, `WindowRedrawRequested`, and `WindowCloseRequested`.
- [ ] Return `1` when renderer creation or rendering fails; otherwise return the platform application result.
- [ ] Run `xmake test -P . window_runtime_test`; expected pass.

### Task 3: Example Refactor

**Files:**
- Modify: `examples/hello_window/main.cpp`
- Modify: `tests/architecture/hello_window_lifetime_test.cpp`

- [ ] Replace duplicated loop state in `hello_window` with `WindowRuntime`.
- [ ] Preserve existing smoke env vars: `CGPUI_EXIT_AFTER_FIRST_FRAME`, `CGPUI_RESIZE_AFTER_FIRST_FRAME`, and `CGPUI_CLOSE_AFTER_FIRST_FRAME`.
- [ ] Run hello smoke tests on Windows and WSL.

### Task 4: Verification

- [ ] Run targeted Windows tests: `xmake test -P . window_runtime_test hello_window/*`.
- [ ] Run full Windows tests.
- [ ] Run full WSL Arch Linux tests with `XMAKE_ROOT=y`.
- [ ] Commit the feature branch.
