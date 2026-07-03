# GPUI Core Depth Steps 179-218 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Continue the Windows/Linux GPUI-core track through forty production-depth slices after Step 178.

**Architecture:** Keep public authoring APIs platform-neutral while replacing the remaining metadata-only renderer/platform skeletons with testable native behavior. The first half deepens Vulkan text and text editing; the second half promotes platform adapters, accessibility, multi-window ownership, theming, assets, animation, and async execution without starting the macOS/Cocoa + Metal parity track.

**Tech Stack:** C++23, Xmake, Win32 + Vulkan on Windows, Wayland + Vulkan on Linux, WSL Arch Linux verification.

---

## Execution Rules

- Work one step at a time in a feature worktree under `.worktrees/`.
- Start each step with RED coverage that fails for the expected missing API or behavior.
- Implement the smallest GREEN slice that makes that step true without claiming full upstream GPUI parity.
- Run targeted tests on Windows and WSL for platform/shared changes, then full debug suites before merge.
- Fast-forward merge each step to `master`, verify post-merge, update `task_plan.md`, `findings.md`, and `progress.md`, then remove the feature worktree and branch.
- Keep macOS/Cocoa + Metal deferred unless a later user request explicitly pivots to the Mac parity track.

## Band I: Vulkan Text And Renderer Depth

## Step 179: Vulkan Glyph Atlas Image Plan

**Branch:** `codex/vulkan-glyph-atlas-image-plan`
**Worktree:** `.worktrees/vulkan-glyph-atlas-image-plan`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`

- [x] Add RED coverage for renderer-facing glyph atlas image descriptors and upload batches derived from `GlyphUploadRecord`.
- [x] Expected RED: missing `GlyphAtlasImageDescriptor`, `GlyphAtlasUploadBatch`, and Vulkan atlas image planning APIs.
- [x] GREEN: add deterministic image descriptors and upload batches without creating real Vulkan image objects yet.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default core_header_cleanliness/default`.
- [x] WSL targeted command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/vulkan-glyph-atlas-image-plan -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . core_header_cleanliness/default'`.

**Merged:** `ec1c6b5 feat: add glyph atlas image upload plan`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 1/1,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 180: Vulkan Glyph Atlas Texture Resource Skeleton

**Branch:** `codex/vulkan-glyph-atlas-texture-resources`
**Worktree:** `.worktrees/vulkan-glyph-atlas-texture-resources`

**Files:**
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/renderer/vulkan_frame_lifetime_test.cpp`

- [x] Add RED coverage for persistent atlas texture resource records keyed by atlas page.
- [x] Expected RED: Vulkan renderer reports upload plans but no texture resource lifetime state.
- [x] GREEN: add internal resource records for atlas pages with stable create/reuse/drop diagnostics; keep real GPU allocation guarded behind the existing Vulkan surface path.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default vulkan_frame_lifetime_test/default`.
- [x] WSL targeted command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/vulkan-glyph-atlas-texture-resources -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . core_header_cleanliness/default'`.

**Merged:** `8446fb7 feat: add glyph atlas texture resource state`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 1/1,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 181: Vulkan Glyph Upload Dirty-Range Tracking

**Branch:** `codex/vulkan-glyph-upload-dirty-ranges`
**Worktree:** `.worktrees/vulkan-glyph-upload-dirty-ranges`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [x] Add RED coverage proving repeated text draws only upload newly allocated glyph regions.
- [x] Expected RED: upload diagnostics count every glyph each report instead of dirty ranges.
- [x] GREEN: add dirty-range coalescing per atlas page and expose deterministic upload-range diagnostics.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default core_header_cleanliness/default`.
- [x] WSL targeted command: available Linux subset `core_header_cleanliness/default`.

**Merged:** `3844a46 feat: add glyph atlas dirty upload ranges`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 1/1,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 182: Vulkan Text Sampler Pipeline Descriptor

**Branch:** `codex/vulkan-text-sampler-pipeline`
**Worktree:** `.worktrees/vulkan-text-sampler-pipeline`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [x] Add RED coverage for a renderer report that distinguishes glyph quad preparation from sampler-pipeline readiness.
- [x] Expected RED: no text sampler pipeline descriptor or readiness counters exist.
- [x] GREEN: add a deterministic text sampler pipeline descriptor and report fields while keeping shader module creation for later.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default core_header_cleanliness/default`.
- [x] WSL targeted command: available Linux subset `core_header_cleanliness/default`.

**Merged:** `2fbc2b1 feat: add text sampler pipeline readiness report`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 1/1,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 183: Vulkan Rounded-Rect Tessellation Records

**Branch:** `codex/vulkan-rounded-rect-tessellation`
**Worktree:** `.worktrees/vulkan-rounded-rect-tessellation`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [x] Add RED coverage for rounded-rect geometry records with radius and clip metadata.
- [x] Expected RED: rounded rectangles are still unsupported diagnostics or metadata-only commands.
- [x] GREEN: translate rounded-rect paint commands into deterministic tessellation records without changing GPU submission.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default render_view_test/default core_header_cleanliness/default ui_header_cleanliness/default`.
- [x] WSL targeted command: available Linux subset `render_view_test/default core_header_cleanliness/default ui_header_cleanliness/default`.

**Merged:** `395af5d feat: add rounded rect tessellation records`.
**Post-merge verification:** Windows targeted 4/4, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 184: Vulkan Text Selection And Caret Geometry Records

**Branch:** `codex/vulkan-text-selection-caret-records`
**Worktree:** `.worktrees/vulkan-text-selection-caret-records`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage proving text selection and caret paint commands become supported renderer records.
- [x] Expected RED: selection/caret commands remain unsupported or skipped in renderer reports.
- [x] GREEN: add deterministic selection/caret rectangle records with color, clip, opacity, and transform metadata.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default render_view_test/default text_model_test/default window_runtime_test/default core_header_cleanliness/default`.
- [x] WSL targeted command: available Linux subset `render_view_test/default text_model_test/default window_runtime_test/default core_header_cleanliness/default`.

**Merged:** `9a1413b feat: add text selection caret geometry records`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 4/4,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 185: Renderer Clip Stack Metadata

**Branch:** `codex/renderer-clip-stack`
**Worktree:** `.worktrees/renderer-clip-stack`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/render_view_test.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [x] Add RED coverage for nested clip stack records emitted by paint traversal and consumed by renderer reports.
- [x] Expected RED: commands only carry a single optional clip rect.
- [x] GREEN: add a bounded clip stack metadata record while preserving the existing per-command clip API.
- [x] Targeted command: `xmake test -P . render_view_test/default vulkan_solid_rect_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set without Windows-only targets.

**Merged:** `e37a6c1 feat: add renderer clip stack metadata`.
**Post-merge verification:** Windows targeted 4/4, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 186: Renderer Opacity And Transform Stack Reports

**Branch:** `codex/renderer-opacity-transform-stack`
**Worktree:** `.worktrees/renderer-opacity-transform-stack`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [x] Add RED coverage for report counters over nested opacity and transform stack depth.
- [x] Expected RED: renderer reports see final metadata but not stack composition depth.
- [x] GREEN: add deterministic composition-stack diagnostics for rect, rounded-rect, text, selection, and caret records.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default render_view_test/default core_header_cleanliness/default`.
- [x] WSL targeted command: available Linux subset `render_view_test/default core_header_cleanliness/default`.

**Merged:** `36cb483 feat: add renderer composition stack reports`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 187: Renderer Batch Submission Plan

**Branch:** `codex/renderer-batch-submission-plan`
**Worktree:** `.worktrees/renderer-batch-submission-plan`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [x] Add RED coverage for GPU-submission-plan records built from renderer command batches.
- [x] Expected RED: batches are diagnostics only and cannot form stable submission groups.
- [x] GREEN: add deterministic submission-plan records grouped by primitive, clip stack, atlas page, and pipeline descriptor.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default core_header_cleanliness/default`.
- [x] WSL targeted command: available Linux subset `core_header_cleanliness/default`.

**Merged:** `6f9a399 feat: add renderer submission plan records`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 1/1,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 188: Renderer Frame Snapshot Report

**Branch:** `codex/renderer-frame-snapshot-report`
**Worktree:** `.worktrees/renderer-frame-snapshot-report`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`
- Modify: `docs/gpui-core-api-parity.md`

- [x] Add RED coverage for a single frame report summarizing supported primitives, atlas uploads, submission plans, and unsupported gaps.
- [x] Expected RED: report pieces exist but no frame-level renderer snapshot captures the full text/render state.
- [x] GREEN: add `RendererFrameReport` aggregation and update the parity audit to reflect the new renderer depth.
- [x] Targeted command: `xmake test -P . vulkan_solid_rect_test/default render_view_test/default desktop_target_readiness_test/default`.
- [x] WSL targeted command: available Linux subset `render_view_test/default desktop_target_readiness_test/default`.

**Merged:** `365d695 feat: add renderer frame snapshot report`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Band J: Text, Font, Editing, And IME Depth

## Step 189: Font Fallback Chain Resolution

**Branch:** `codex/font-fallback-chain`
**Worktree:** `.worktrees/font-fallback-chain`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`

- [x] Add RED coverage for ordered font fallback chains and missing-family fallback behavior.
- [x] Expected RED: `FontDatabase::resolve(...)` returns one face and cannot expose fallback candidates.
- [x] GREEN: add fallback-chain resolution over requested family, generic fallback, and first available face.
- [x] Targeted command: `xmake test -P . text_model_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `6d0afc2 feat: add font fallback chain resolution`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 190: Platform Font Discovery Records

**Branch:** `codex/platform-font-discovery-records`
**Worktree:** `.worktrees/platform-font-discovery-records`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`
- Modify: `tests/ui/text_model_test.cpp`

- [x] Add RED coverage for platform font discovery records with source, family, and path/name metadata.
- [x] Expected RED: Win32 and Wayland discovery overrides remain gracefully empty.
- [x] GREEN: add deterministic discovery hooks with test-visible records while keeping system font enumeration conservative.
- [x] Targeted command: `xmake test -P . text_model_test/default win32_window_source_test/default wayland_window_source_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `c865dc9 feat: add platform font discovery records`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 191: Grapheme-Aware Cursor Movement Skeleton

**Branch:** `codex/text-grapheme-cursor`
**Worktree:** `.worktrees/text-grapheme-cursor`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`

- [x] Add RED coverage for cursor movement over combining marks and emoji byte sequences.
- [x] Expected RED: cursor movement only uses UTF-8 codepoint boundaries.
- [x] GREEN: add a deterministic grapheme-boundary helper covering ASCII, combining mark, regional indicator, and emoji ZWJ skeleton cases.
- [x] Targeted command: `xmake test -P . text_model_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `2108199 feat: add grapheme-aware text cursor movement`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 192: Word Movement And Selection Actions

**Branch:** `codex/text-word-navigation`
**Worktree:** `.worktrees/text-word-navigation`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`

- [x] Add RED coverage for previous/next word cursor movement and word selection extension.
- [x] Expected RED: `TextEditAction` has no word navigation variants.
- [x] GREEN: add word-boundary actions using deterministic ASCII/Unicode-space classes.
- [x] Targeted command: `xmake test -P . text_model_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `eaf6907 feat: add text word navigation actions`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 193: Text Undo And Redo Stack

**Branch:** `codex/text-undo-redo-stack`
**Worktree:** `.worktrees/text-undo-redo-stack`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`

- [x] Add RED coverage for undo/redo after insert, delete, selection replace, and composition commit.
- [x] Expected RED: `TextModel` has no undo or redo APIs.
- [x] GREEN: add bounded edit history records with cursor and selection restoration.
- [x] Targeted command: `xmake test -P . text_model_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `5406e9a feat: add text undo redo stack`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 194: IME Delete-Surrounding Text Action

**Branch:** `codex/ime-delete-surrounding-text`
**Worktree:** `.worktrees/ime-delete-surrounding-text`

**Files:**
- Modify: `include/cgpui/core/events.hpp`
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/platform/wayland_keyboard_test.cpp`

- [x] Add RED coverage for IME delete-surrounding events mutating focused text models.
- [x] Expected RED: IME commit/preedit exists but delete-surrounding has no public event or routing.
- [x] GREEN: add event shape and runtime text-model application with deterministic byte-boundary clamping.
- [x] Targeted command: `xmake test -P . window_runtime_test/default text_model_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: include `wayland_keyboard_test/default`.

**Merged:** `98c2902 feat: route ime delete surrounding text`.
**Post-merge verification:** Windows targeted 4/4, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30 after
isolating one transient `clipboard_test/default` batch failure with a passing
targeted rerun and full-suite rerun.

## Step 195: Multiline Text Model And Line Navigation

**Branch:** `codex/multiline-text-model`
**Worktree:** `.worktrees/multiline-text-model`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`

- [x] Add RED coverage for line start/end, previous/next line, and multiline selection ranges.
- [x] Expected RED: text model treats content as one logical line.
- [x] GREEN: add deterministic line-index helpers and edit actions without adding full paragraph layout.
- [x] Targeted command: `xmake test -P . text_model_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `715f7bb feat: add multiline text navigation`.
**Post-merge verification:** Windows targeted 2/2, WSL targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 196: Text Measurement Cache

**Branch:** `codex/text-measurement-cache`
**Worktree:** `.worktrees/text-measurement-cache`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `tests/ui/text_model_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [x] Add RED coverage for cache hits when measuring the same text/font/scale tuple repeatedly.
- [x] Expected RED: shaping and measurement are recomputed with no observable cache record.
- [x] GREEN: add a small deterministic measurement cache API usable by text elements and renderer tests.
- [x] Targeted command: `xmake test -P . text_model_test/default render_view_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

## Step 197: Text Pointer Selection Geometry

**Branch:** `codex/text-pointer-selection-geometry`
**Worktree:** `.worktrees/text-pointer-selection-geometry`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/ui/text_model_test.cpp`

- [x] Add RED coverage for hit-testing text glyph positions into cursor offsets and selection ranges.
- [x] Expected RED: pointer events focus text inputs but cannot map pointer x/y to text offsets.
- [x] GREEN: add deterministic single-line text hit geometry and route pointer drag into selection state.
- [x] Targeted command: `xmake test -P . window_runtime_test/default text_model_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `7d148c5 feat: add text pointer selection geometry`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 198: Soft Wrap Layout Records

**Branch:** `codex/text-soft-wrap-records`
**Worktree:** `.worktrees/text-soft-wrap-records`

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/text_model_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [x] Add RED coverage for deterministic soft-wrap line records under a maximum width.
- [x] Expected RED: text layout emits one run with no wrap records.
- [x] GREEN: add wrap-record generation and use it for text paint bounds while preserving existing single-line behavior.
- [x] Targeted command: `xmake test -P . text_model_test/default render_view_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `02b534c feat: add text soft wrap records`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Band K: Windows And Linux Native Platform Depth

## Step 199: Wayland Clipboard Ownership And Send Offers

**Branch:** `codex/wayland-clipboard-ownership`
**Worktree:** `.worktrees/wayland-clipboard-ownership`

**Files:**
- Modify: `src/platform/clipboard.cpp`
- Modify: `tests/platform/clipboard_test.cpp`
- Modify: `tests/platform/wayland_test_compositor.cpp`
- Modify: `tests/platform/wayland_test_compositor.hpp`

- [x] Add RED coverage for Wayland clipboard write ownership and compositor-side payload receive.
- [x] Expected RED: Wayland clipboard can read offers but writes fall back to memory-only behavior.
- [x] GREEN: add data-source ownership, offered text MIME types, and deterministic send path in the test compositor.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/wayland-clipboard-ownership -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . clipboard_test/default wayland_window_source_test/default core_header_cleanliness/default'`.
- [x] Windows available-target command: `xmake test -P . clipboard_test/default wayland_window_source_test/default core_header_cleanliness/default`.

**Merged:** `96a5afa feat: add wayland clipboard ownership`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30 after a
transient `clipboard_test/default` failure was isolated with a passing targeted
rerun and passing full-suite rerun.

## Step 200: Wayland Drag Action Negotiation

**Branch:** `codex/wayland-dnd-actions`
**Worktree:** `.worktrees/wayland-dnd-actions`

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/wayland_pointer_button_test.cpp`
- Modify: `tests/platform/wayland_test_compositor.cpp`
- Modify: `tests/platform/wayland_test_compositor.hpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for copy/move/none drag action negotiation and finish records.
- [x] Expected RED: drag payload extraction exists but accept/finish/action state is missing.
- [x] GREEN: track offered actions, selected action, accept calls, drop finish, and public drag action metadata.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/wayland-dnd-actions -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_pointer_button_test/default window_runtime_test/default wayland_window_source_test/default core_header_cleanliness/default ui_header_cleanliness/default'`.
- [x] Windows available-target command: `xmake test -P . window_runtime_test/default wayland_window_source_test/default core_header_cleanliness/default ui_header_cleanliness/default`.

**Merged:** `d45061c feat: add wayland drag action negotiation`.
**Post-merge verification:** WSL targeted 5/5, Windows targeted 4/4,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 201: Wayland Cursor Theme Image State

**Branch:** `codex/wayland-cursor-theme-state`
**Worktree:** `.worktrees/wayland-cursor-theme-state`

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`
- Modify: `tests/platform/wayland_pointer_button_test.cpp`

- [x] Add RED coverage for cursor theme/image state records when runtime applies cursor shapes.
- [x] Expected RED: Wayland cursor path only has `wl_pointer.set_cursor` skeleton markers.
- [x] GREEN: add deterministic cursor-name mapping, theme-load state, and graceful unavailable behavior.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/wayland-cursor-theme-state -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_pointer_button_test/default wayland_window_source_test/default core_header_cleanliness/default'`.
- [x] Windows available-target command: `xmake test -P . wayland_window_source_test/default core_header_cleanliness/default`.

**Merged:** `2b5dd4a feat: add wayland cursor theme state`.
**Post-merge verification:** WSL targeted 3/3, Windows targeted 2/2,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 202: Wayland XDG Configure Lifecycle State

**Branch:** `codex/wayland-xdg-configure-lifecycle`
**Worktree:** `.worktrees/wayland-xdg-configure-lifecycle`

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/wayland_compositor_resize_test.cpp`
- Modify: `tests/platform/wayland_test_compositor.cpp`
- Modify: `tests/platform/wayland_test_compositor.hpp`

- [x] Add RED coverage for configure serial ack, pending size, activated, maximized, and fullscreen states.
- [x] Expected RED: Wayland resize tests see framebuffer size but not full configure lifecycle state.
- [x] GREEN: add deterministic xdg configure state records and route lifecycle events into public window events.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/wayland-xdg-configure-lifecycle -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_compositor_resize_test/default window_runtime_test/default wayland_window_source_test/default core_header_cleanliness/default'`.
- [x] Windows available-target command: `xmake test -P . window_runtime_test/default wayland_window_source_test/default core_header_cleanliness/default`.

**Merged:** `6c9b867 feat: add wayland configure lifecycle state`.
**Post-merge verification:** WSL targeted 4/4, Windows targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 203: Win32 OLE Drop Target Skeleton

**Branch:** `codex/win32-ole-drop-target`
**Worktree:** `.worktrees/win32-ole-drop-target`

**Files:**
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `xmake.lua`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/platform/win32_input_event_test.cpp`
- Existing coverage: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for Win32 OLE drop target registration markers and public drag action metadata.
- [x] Expected RED: Win32 drag/drop remains deterministic event hooks without OLE registration state.
- [x] GREEN: add an internal OLE drop target skeleton with registration diagnostics and text/file payload conversion boundaries.
- [x] Targeted command: `xmake test -P . win32_input_event_test/default window_runtime_test/default win32_window_source_test/default core_header_cleanliness/default ui_header_cleanliness/default`.
- [x] WSL available-target command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/win32-ole-drop-target -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . window_runtime_test/default win32_window_source_test/default core_header_cleanliness/default ui_header_cleanliness/default'`.

**Merged:** `3f26a33 feat: add win32 ole drop target skeleton`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 4/4,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 204: Native Menu And Accelerator API Skeleton

**Branch:** `codex/native-menu-accelerator-api`
**Worktree:** `.worktrees/native-menu-accelerator-api`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`

- [x] Add RED coverage for platform-neutral menu models, accelerator descriptors, and runtime installation.
- [x] Expected RED: no menu/accelerator public API or platform hook exists.
- [x] GREEN: add inert menu model storage and platform install hooks with Win32/Wayland skeleton diagnostics.
- [x] Targeted command: `xmake test -P . app_runner_test/default win32_window_source_test/default wayland_window_source_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `25c5e5f feat: add native menu accelerator skeleton`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30 after
isolating one transient `clipboard_test/default` full-suite failure with a
passing targeted rerun and passing full-suite rerun.

## Step 205: Native File Dialog API Skeleton

**Branch:** `codex/native-file-dialog-api`
**Worktree:** `.worktrees/native-file-dialog-api`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`

- [x] Add RED coverage for open/save file dialog request descriptors and graceful unsupported results.
- [x] Expected RED: no platform-neutral file dialog request/result API exists.
- [x] GREEN: add descriptor/result types, app-context forwarding, and Win32/Wayland skeleton platform hooks.
- [x] Targeted command: `xmake test -P . app_runner_test/default win32_window_source_test/default wayland_window_source_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `a7a2ac5 feat: add native file dialog skeleton`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 206: Window Chrome Customization Skeleton

**Branch:** `codex/window-chrome-customization`
**Worktree:** `.worktrees/window-chrome-customization`

**Files:**
- Modify: `include/cgpui/core/window.hpp`
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`

- [x] Add RED coverage for titlebar visibility, decorations, resizable, and transparent window options.
- [x] Expected RED: `WindowOptions` cannot express window chrome preferences.
- [x] GREEN: add option metadata and platform state application skeletons with graceful unsupported behavior.
- [x] Targeted command: `xmake test -P . app_runner_test/default win32_window_source_test/default wayland_window_source_test/default core_header_cleanliness/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `6387371 feat: add window chrome customization skeleton`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 207: App Command Palette Registry

**Branch:** `codex/app-command-palette-registry`
**Worktree:** `.worktrees/app-command-palette-registry`

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`

- [x] Add RED coverage for command palette entries backed by the existing scoped action registry.
- [x] Expected RED: actions exist but have no searchable command metadata registry.
- [x] GREEN: add command metadata, grouping, enablement, and dispatch through existing action scopes.
- [x] Targeted command: `xmake test -P . window_runtime_test/default app_runner_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `92e03ea feat: add command palette registry`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30 after
isolating a transient `clipboard_test/default` batch failure with a passing
targeted rerun and full-suite rerun.

## Step 208: Platform Diagnostics Event Stream

**Branch:** `codex/platform-diagnostics-stream`
**Worktree:** `.worktrees/platform-diagnostics-stream`

**Files:**
- Modify: `include/cgpui/core/events.hpp`
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for collecting platform diagnostics from clipboard, DnD, IME, accessibility, and window lifecycle hooks.
- [x] Expected RED: diagnostics snapshots cover runtime/frame data but not platform adapter state.
- [x] GREEN: add a bounded platform diagnostics stream surfaced through runtime diagnostics snapshots.
- [x] Targeted command: `xmake test -P . window_runtime_test/default win32_window_source_test/default wayland_window_source_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `6ff6b0f feat: add platform diagnostics stream`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Band L: Accessibility, Multi-Window, Theme, Assets, Animation, Async

## Step 209: UIA Provider Tree Facade

**Branch:** `codex/win32-uia-provider-facade`
**Worktree:** `.worktrees/win32-uia-provider-facade`

**Files:**
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for a UIA provider facade that exposes stable node ids, role/name/text/value, bounds, and focus state.
- [x] Expected RED: Win32 UIA adapter only counts nodes.
- [x] GREEN: add internal facade records over `PlatformAccessibilityTreeUpdate` without implementing COM provider objects yet.
- [x] Targeted command: `xmake test -P . win32_window_source_test/default window_runtime_test/default core_header_cleanliness/default ui_header_cleanliness/default`.
- [x] WSL available-target command: same target set.

**Merged:** `2eb0749 feat: add win32 uia provider facade`.
**Post-merge verification:** Windows targeted 4/4, WSL targeted 4/4,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 210: AT-SPI Object Model Facade

**Branch:** `codex/linux-atspi-object-facade`
**Worktree:** `.worktrees/linux-atspi-object-facade`

**Files:**
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`

- [x] Add RED coverage for an AT-SPI object facade with stable object paths, roles, names, text, and focus state.
- [x] Expected RED: Linux AT-SPI adapter only counts nodes.
- [x] GREEN: add internal object-model records over `PlatformAccessibilityTreeUpdate` without exposing D-Bus provider types.
- [x] Targeted WSL command: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/linux-atspi-object-facade -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . wayland_window_source_test/default window_runtime_test/default core_header_cleanliness/default ui_header_cleanliness/default'`.
- [x] Windows available-target command: `xmake test -P . wayland_window_source_test/default window_runtime_test/default core_header_cleanliness/default ui_header_cleanliness/default`.

**Merged:** `069a997 feat: add linux atspi object facade`.
**Post-merge verification:** Windows targeted 4/4, WSL targeted 4/4,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30 after
isolating a transient `clipboard_test/default` batch failure with a passing
targeted rerun.

## Step 211: Accessibility Value And Live Update Events

**Branch:** `codex/accessibility-value-live-events`
**Worktree:** `.worktrees/accessibility-value-live-events`

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/wayland_window_source_test.cpp`

- [x] Add RED coverage for value/text-change/focus-change accessibility update events.
- [x] Expected RED: platform accessibility updates are full snapshots only.
- [x] GREEN: add incremental update records generated from focused text and element snapshot changes.
- [x] Targeted command: `xmake test -P . window_runtime_test/default win32_window_source_test/default wayland_window_source_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `1e51aa8 feat: add accessibility live update records`.
**Post-merge verification:** Windows targeted 5/5, WSL targeted 5/5,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 212: Additional Window Renderer Ownership

**Branch:** `codex/additional-window-renderer-ownership`
**Worktree:** `.worktrees/additional-window-renderer-ownership`

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`

- [x] Add RED coverage proving app-opened child windows can own renderers created from their native surfaces.
- [x] Expected RED: child windows own native windows but `renderer` remains null.
- [x] GREEN: create child renderers through the existing renderer factory boundary and preserve graceful per-record creation errors.
- [x] Targeted command: `xmake test -P . app_runner_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `701a2f4 feat: add additional window renderer ownership`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 213: Additional Window Event Routing

**Branch:** `codex/additional-window-event-routing`
**Worktree:** `.worktrees/additional-window-event-routing`

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED coverage for routing pointer, keyboard, focus, redraw, resize, and close events by `WindowRuntimeId`.
- [x] Expected RED: child platform callback only updates resize and close-active state.
- [x] GREEN: route child events through the existing runtime event path with per-window root view lookup.
- [x] Targeted command: `xmake test -P . app_runner_test/default window_runtime_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `d8b86fc feat: route additional window events`.
**Post-merge verification:** Windows targeted 4/4, WSL targeted 4/4,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 214: Additional Window Lifecycle Cleanup

**Branch:** `codex/additional-window-lifecycle-cleanup`
**Worktree:** `.worktrees/additional-window-lifecycle-cleanup`

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`

- [x] Add RED coverage for child window close destroying child root views, subscriptions, renderer ownership, and native window ownership.
- [x] Expected RED: shutdown clears pointers but child-close lifecycle does not release owned runtime state.
- [x] GREEN: add deterministic child-window close cleanup with root view and subscription teardown.
- [x] Targeted command: `xmake test -P . app_runner_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `437ef5e feat: clean up additional window lifecycle`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 215: Runtime Theme Inheritance And Switching

**Branch:** `codex/runtime-theme-switching`
**Worktree:** `.worktrees/runtime-theme-switching`

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [x] Add RED coverage for app/window theme storage, inherited token lookup, and dynamic theme switching invalidation.
- [x] Expected RED: theme token storage exists but no runtime theme hierarchy or switch invalidation exists.
- [x] GREEN: add runtime theme slots, inherited resolution, and redraw invalidation on theme changes.
- [x] Targeted command: `xmake test -P . style_test/default render_view_test/default ui_header_cleanliness/default`.
- [x] WSL targeted command: same target set.

**Merged:** `46f96e0 feat: add runtime theme switching`.
**Post-merge verification:** Windows targeted 3/3, WSL targeted 3/3,
`git diff --check`, WSL full debug 27/27, and Windows full debug 30/30.

## Step 216: Animation Clock And Tween Primitives

**Branch:** `codex/animation-clock-tween`
**Worktree:** `.worktrees/animation-clock-tween`

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/ui/style_test.cpp`

- [ ] Add RED coverage for animation handles, easing, tween progress, and timer-driven redraw.
- [ ] Expected RED: no animation clock, easing, or style tween API exists.
- [ ] GREEN: add deterministic animation records driven by the existing timer/wakeup system.
- [ ] Targeted command: `xmake test -P . window_runtime_test/default style_test/default ui_header_cleanliness/default`.
- [ ] WSL targeted command: same target set.

## Step 217: Asset And Image Pipeline Skeleton

**Branch:** `codex/asset-image-pipeline`
**Worktree:** `.worktrees/asset-image-pipeline`

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `tests/ui/render_view_test.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [ ] Add RED coverage for image asset descriptors, decoded bitmap records, image paint commands, and Vulkan upload planning.
- [ ] Expected RED: no image/asset paint or renderer upload path exists.
- [ ] GREEN: add deterministic in-memory bitmap assets and image command/upload-plan records without external decoders.
- [ ] Targeted command: `xmake test -P . render_view_test/default vulkan_solid_rect_test/default ui_header_cleanliness/default core_header_cleanliness/default`.
- [ ] WSL targeted command: available Linux subset `render_view_test/default core_header_cleanliness/default ui_header_cleanliness/default`.

## Step 218: Threaded Async Executor And Cancellation

**Branch:** `codex/threaded-async-executor`
**Worktree:** `.worktrees/threaded-async-executor`

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `docs/gpui-core-api-parity.md`

- [ ] Add RED coverage for background task execution, cancellation, completion dispatch, and diagnostics.
- [ ] Expected RED: async task handles are skeletons completed manually on the main runtime.
- [ ] GREEN: add a small threaded executor abstraction with cancellation tokens and main-thread completion dispatch through the existing wakeup path.
- [ ] Targeted command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default core_header_cleanliness/default desktop_target_readiness_test/default`.
- [ ] WSL targeted command: same target set.

## Post-Step-218 Checkpoint

- [ ] Refresh `docs/gpui-core-api-parity.md` so renderer, text, platform, accessibility, multi-window, theme, asset, animation, and async state are accurately categorized.
- [ ] Run Windows full debug: `xmake f -c -m debug -P .; xmake test -P .`.
- [ ] Run WSL full debug: `wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui -- bash -lc 'XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`.
- [ ] Confirm `git status --short --branch` on `master` has no tracked changes and only the expected untracked `.vscode/`.
