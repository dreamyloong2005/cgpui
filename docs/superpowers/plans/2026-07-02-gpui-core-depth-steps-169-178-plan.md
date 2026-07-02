# GPUI Core Depth Steps 169-178 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Advance the Windows/Linux GPUI-core track through ten depth slices after Step 168, focusing on real Vulkan text data flow, Wayland protocol payloads, native accessibility adapter boundaries, and native multi-window creation.

**Architecture:** Keep public authoring APIs platform-neutral and deepen the existing renderer/platform seams. Text rendering moves from glyph metadata toward bitmap raster data, atlas allocation/upload records, and textured glyph quads; Wayland moves skeleton data-device/text-input state toward concrete MIME and state-machine behavior; accessibility and multi-window work consume existing snapshots/records instead of forking UI semantics.

**Tech Stack:** C++23, Xmake, Win32 + Vulkan on Windows, Wayland + Vulkan on Linux, WSL Arch Linux verification.

---

## Execution Rules

- Work one step at a time in a feature worktree under `.worktrees/`.
- Start each step with RED coverage that fails for the expected missing API or behavior.
- Implement the smallest GREEN slice that makes that step true without claiming full upstream GPUI parity.
- Run targeted tests on Windows and WSL for platform/shared changes, then full debug suites before merge.
- Fast-forward merge each step to `master`, verify post-merge, update `task_plan.md`, `findings.md`, and `progress.md`, then remove the feature worktree and branch.

## Step 169: Glyph Bitmap And Fallback Rasterizer

**Branch:** `codex/glyph-raster-data-model`  
**Worktree:** `.worktrees/glyph-raster-data-model`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`
- Modify: `tests/ui/text_test.cpp` or create this test target if text coverage is split elsewhere
- Modify: `xmake.lua` only if a new test target is required

- [x] Add RED coverage for `GlyphBitmap`, `RasterizedGlyph`, `GlyphRasterizerOptions`, and `rasterize_fallback_glyph(...)`.
- [x] Expected RED: build fails on missing raster data model APIs.
- [x] GREEN: add deterministic alpha bitmap data for each fallback glyph using glyph key, byte length, device font size, advance, width, height, baseline offset, and left/top bearing metadata.
- [x] Targeted command: `xmake test -P . text_model_test/default ui_header_cleanliness/default core_header_cleanliness/default` passed 3/3 on Windows after merge.
- [x] WSL targeted command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . text_model_test/default ui_header_cleanliness/default core_header_cleanliness/default'` passed 3/3 after merge.
- [x] Full post-merge verification passed: WSL Arch Linux full debug 27/27; Windows `clipboard_test/default` failed once during a full-suite batch, then targeted `clipboard_test/default` passed 1/1 and Windows full debug rerun passed 30/30.

## Step 170: Glyph Atlas Slot Allocation And Upload Records

**Branch:** `codex/glyph-atlas-upload-records`  
**Worktree:** `.worktrees/glyph-atlas-upload-records`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`

- [x] Add RED coverage for `GlyphAtlasPage`, `GlyphAtlasAllocation`, `GlyphUploadRecord`, and cache allocation from `RasterizedGlyph`.
- [x] Expected RED: missing atlas page/allocation/upload record APIs.
- [x] GREEN: pack glyph bitmaps into deterministic rows inside fixed-size atlas pages, store atlas UV/device bounds, and record upload byte ranges without creating Vulkan images yet.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default text_model_test/default core_header_cleanliness/default ui_header_cleanliness/default` passed 4/4 on Windows after merge.
- [x] WSL targeted command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . text_model_test/default core_header_cleanliness/default ui_header_cleanliness/default'` passed 3/3 after merge.
- [x] Full post-merge verification passed: `git diff --check` produced no output, WSL Arch Linux full debug passed 27/27, and Windows full debug passed 30/30.

## Step 171: Vulkan Textured Glyph Quad Commands

**Branch:** `codex/vulkan-textured-glyph-quads`  
**Worktree:** `.worktrees/vulkan-textured-glyph-quads`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [x] Add RED coverage for renderer-facing `TexturedGlyphQuad` records emitted from `TextDraw`.
- [x] Expected RED: missing textured glyph quad command API.
- [x] GREEN: translate glyph atlas entries into stable device-space quads with atlas UVs, color, clip, opacity, and transform metadata.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default render_view_test/default core_header_cleanliness/default ui_header_cleanliness/default` passed 4/4 on Windows after merge.
- [x] WSL targeted command: available Linux subset `render_view_test/default core_header_cleanliness/default ui_header_cleanliness/default` passed 3/3 after merge.
- [x] Full post-merge verification passed: `git diff --check` produced no output, WSL Arch Linux full debug passed 27/27, and Windows full debug passed 30/30.

## Step 172: Vulkan Text Render Report Distinguishes Real Glyph Draw

**Branch:** `codex/vulkan-text-render-report`  
**Worktree:** `.worktrees/vulkan-text-render-report`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `docs/gpui-core-api-parity.md`

- [x] Add RED coverage proving text commands are reported as glyph-quad-backed work rather than metadata-only placeholders.
- [x] Expected RED: report lacks text draw depth/quad/upload counters.
- [x] GREEN: add deterministic counters for glyph cache hits, rasterized glyphs, upload records, and emitted glyph quads while keeping actual Vulkan texture objects as a later GPU-integration step if needed.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default desktop_target_readiness_test/default` passed 2/2 on Windows after merge.
- [x] WSL targeted command: available Linux subset `desktop_target_readiness_test/default` passed 1/1 after merge.
- [x] Full post-merge verification passed: `git diff --check` produced no output, WSL Arch Linux full debug passed 27/27, and Windows full debug passed 30/30.

## Step 173: Wayland Clipboard MIME Payload Extraction

**Branch:** `codex/wayland-clipboard-mime-payloads`  
**Worktree:** `.worktrees/wayland-clipboard-mime-payloads`

**Files:**
- Modify: Linux Wayland clipboard source
- Modify: `tests/platform/clipboard_test.cpp`
- Modify: `tests/platform/wayland_test_compositor.cpp`
- Modify: `tests/platform/wayland_test_compositor.hpp`

- [x] Add RED coverage for offered MIME types, `text/plain;charset=utf-8`, `text/plain`, and deterministic payload reads from the test compositor.
- [x] Expected RED: Wayland clipboard still only reports skeleton memory fallback behavior.
- [x] GREEN: track the current selection offer, choose preferred text MIME type, read payload bytes through deterministic compositor pipes, and return UTF-8 text through the public clipboard API.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . clipboard_test/default wayland_window_source_test/default core_header_cleanliness/default'` passed 3/3 after merge.
- [x] Windows available-target command: `xmake test -P . clipboard_test/default core_header_cleanliness/default` passed 2/2 after merge.
- [x] Full post-merge verification passed: `git diff --check` produced no output, WSL Arch Linux full debug passed 27/27, and Windows full debug passed 30/30.

## Step 174: Wayland Drag/Drop MIME Text And URI Payloads

**Branch:** `codex/wayland-dnd-mime-payloads`  
**Worktree:** `.worktrees/wayland-dnd-mime-payloads`

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/wayland_pointer_button_test.cpp`
- Modify: `tests/platform/wayland_test_compositor.cpp`
- Modify: `tests/platform/wayland_test_compositor.hpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for text/plain drag payloads and `text/uri-list` file payload parsing.
- [x] Expected RED: Wayland drag payloads remain `DragDropPayloadKind::none`.
- [x] GREEN: store offer MIME types, choose text or URI-list payloads, parse local file URIs into public file payloads, and keep no-data offers graceful.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_pointer_button_test/default window_runtime_test/default wayland_window_source_test/default core_header_cleanliness/default ui_header_cleanliness/default'` passed 5/5 after merge.
- [x] Windows available-target command: `xmake test -P . window_runtime_test/default core_header_cleanliness/default ui_header_cleanliness/default wayland_window_source_test/default` passed 4/4 after merge.
- [x] Full post-merge verification passed: `git diff --check` produced no output, WSL Arch Linux full debug passed 27/27, and Windows full debug passed 30/30.

## Step 175: Wayland Text-Input State Machine

**Branch:** `codex/wayland-text-input-state-machine`  
**Worktree:** `.worktrees/wayland-text-input-state-machine`

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/wayland_keyboard_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`
- Modify: `tests/platform/wayland_test_compositor.cpp`
- Modify: `tests/platform/wayland_test_compositor.hpp`

- [x] Add RED coverage for text-input support state with enter, leave, surrounding text, preedit, commit, and content-type records.
- [x] Expected RED observed: WSL targeted build failed on missing
  `WaylandTestCompositor::request_text_input_enter/preedit/commit/leave`,
  wait/accessor helpers, and text-input client-state API.
- [x] GREEN: introduced a protocol-independent `WaylandTextInputState`,
  a minimal handwritten `zwp_text_input_v3` client/server binding, protocol
  records for surrounding/cursor/content-type commits, and deterministic
  compositor enter/preedit/commit/leave events that dispatch public
  `ImeComposition` update/commit events.
- [x] Feature-worktree WSL targeted command passed 3/3:
  `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/wayland-text-input-state-machine -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_keyboard_test/default wayland_window_source_test/default core_header_cleanliness/default'`.
- [x] Feature-worktree Windows targeted command passed 3/3:
  `xmake test -P . window_runtime_test/default wayland_window_source_test/default core_header_cleanliness/default`.
- [x] `git diff --check` reported only expected CRLF warnings in the feature
  worktree and no whitespace errors.
- [x] Feature-worktree full debug verification passed: WSL Arch Linux 27/27;
  Windows 30/30.
- [x] Merged to `master` as
  `3484f62 feat: add wayland text input state machine`.
- [x] Post-merge targeted verification passed: WSL Arch Linux 4/4; Windows 3/3.
- [x] Post-merge `git diff --check` produced no output.
- [x] Full post-merge verification passed: WSL Arch Linux full debug 27/27;
  Windows `clipboard_test/default` failed once during a full-suite batch, then
  targeted `clipboard_test/default` passed 1/1 and Windows full debug rerun
  passed 30/30.

## Step 176: Windows UIA Accessibility Adapter Skeleton

**Branch:** `codex/win32-uia-accessibility-adapter`  
**Worktree:** `.worktrees/win32-uia-accessibility-adapter`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for a Win32 accessibility adapter update record consuming `AccessibilityTreeSnapshot`.
- [x] Expected RED observed: Windows targeted build failed on missing
  `PlatformAccessibilityNodeUpdate`, `PlatformAccessibilityRole`,
  `PlatformAccessibilityTreeUpdate::nodes`, and
  `PlatformAccessibilityTreeUpdate::focused_node_count`.
- [x] GREEN: added platform-neutral accessibility node update records, role
  mapping from `AccessibilityTreeSnapshot`, runtime forwarding to
  `PlatformWindow::update_accessibility_tree(...)`, and a Win32
  `Win32UiaAccessibilityAdapter` skeleton that consumes the update while
  keeping production UIA COM/provider details out of public headers.
- [x] Feature-worktree Windows targeted command passed 4/4:
  `xmake test -P . win32_window_source_test/default window_runtime_test/default core_header_cleanliness/default ui_header_cleanliness/default`.
- [x] Feature-worktree WSL targeted command passed 4/4:
  `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/win32-uia-accessibility-adapter -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . win32_window_source_test/default window_runtime_test/default core_header_cleanliness/default ui_header_cleanliness/default'`.
- [x] `git diff --check` reported only expected CRLF warnings in the feature
  worktree and no whitespace errors.
- [x] Feature-worktree full debug verification passed: WSL Arch Linux 27/27;
  Windows 30/30.
- [x] Merged to `master` as
  `edd3513 feat: add win32 uia accessibility adapter skeleton`.
- [x] Post-merge targeted verification passed: WSL Arch Linux 4/4; Windows 4/4.
- [x] Post-merge `git diff --check` produced no output.
- [x] Full post-merge verification passed: WSL Arch Linux full debug 27/27;
  Windows full debug 30/30.

## Step 177: Linux AT-SPI Accessibility Adapter Skeleton

**Branch:** `codex/linux-atspi-accessibility-adapter`  
**Worktree:** `.worktrees/linux-atspi-accessibility-adapter`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED coverage for a Linux AT-SPI adapter update record consuming `AccessibilityTreeSnapshot`.
- [ ] Expected RED: Wayland platform does not record AT-SPI-facing tree updates.
- [ ] GREEN: add a Wayland/Linux AT-SPI adapter skeleton with stable node id/name/role/focus counts and source isolation from the public UI surface.
- [ ] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/linux-atspi-accessibility-adapter -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_window_source_test/default window_runtime_test/default ui_header_cleanliness/default core_header_cleanliness/default'`.
- [ ] Windows available-target command: platform-neutral/header coverage only.

## Step 178: Native Additional Window Creation Slice

**Branch:** `codex/native-additional-window-creation`  
**Worktree:** `.worktrees/native-additional-window-creation`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: current app runner header/source
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`

- [ ] Add RED coverage proving `AppContext::open_window(...)` can create an owned native platform window record beyond the root instead of only registering metadata.
- [ ] Expected RED: multi-window registry has records but no native child-window creation bridge.
- [ ] GREEN: add a platform-neutral additional-window creation request, Win32/Wayland skeleton native window creation path, per-window runtime record activation, and graceful unsupported behavior where a backend cannot create the window during tests.
- [ ] Targeted command: `xmake test -P . app_runner_test/default win32_window_source_test/default wayland_window_source_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [ ] WSL targeted command: same target set under the Step 178 worktree through WSL.

## Post-Step-178 Checkpoint

- [ ] Refresh `docs/gpui-core-api-parity.md` so text rendering, Wayland protocol depth, accessibility adapters, and multi-window state are accurately categorized.
- [ ] Run Windows full debug: `xmake f -c -m debug -P .; xmake test -P .`.
- [ ] Run WSL full debug: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui -- bash -lc 'XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`.
- [ ] Confirm `git status --short --branch` on `master` has no tracked changes and only the expected untracked `.vscode/`.
