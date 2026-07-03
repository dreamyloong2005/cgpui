# CGPUI Structural Optimization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish an aggressive structure-only optimization of the Windows/Linux CGPUI codebase by shrinking private headers, remaining long implementation files, public API headers, and oversized tests.

**Architecture:** Encode every split with RED architecture tests first, then move declarations and implementations into focused files. Compatibility aggregate headers stay thin while leaf headers and private implementation units are aggressively reduced.

**Tech Stack:** C++23, Xmake, Win32 + Vulkan, Wayland + Vulkan, architecture tests under `tests/architecture`, header-cleanliness tests under `tests/header_cleanliness`.

---

### Task 1: Private Header Surgery

**Files:**
- Modify: `tests/architecture/platform_source_structure_test.cpp`
- Modify: `tests/architecture/renderer_source_structure_test.cpp`
- Create: `src/platform/linux/wayland_application_core_internal.hpp`
- Create: `src/platform/linux/wayland_application_registry_internal.hpp`
- Create: `src/platform/linux/wayland_application_input_internal.hpp`
- Create: `src/platform/linux/wayland_application_cursor_internal.hpp`
- Create: `src/platform/linux/wayland_registered_window_internal.hpp`
- Create: `src/platform/linux/wayland_window_configure_internal.hpp`
- Create: `src/renderer/vulkan/vulkan_platform_internal.hpp`
- Create: `src/renderer/vulkan/vulkan_state_internal.hpp`
- Create: `src/renderer/vulkan/vulkan_swapchain_internal.hpp`
- Create: `src/renderer/vulkan/vulkan_device_internal.hpp`
- Modify: `src/platform/linux/wayland_application_internal.hpp`
- Modify: `src/platform/linux/wayland_window_internal.hpp`
- Modify: `src/renderer/vulkan/vulkan_internal.hpp`

- [x] **Step 1: Add failing private-header structure tests**

Add checks that require the new private headers above to exist and require:

```cpp
line_count(read_source("src/platform/linux/wayland_application_internal.hpp")) <= 120
line_count(read_source("src/platform/linux/wayland_window_internal.hpp")) <= 120
line_count(read_source("src/renderer/vulkan/vulkan_internal.hpp")) <= 120
```

- [x] **Step 2: Run RED tests**

Run:

```powershell
xmake test -y -P . platform_source_structure_test/default renderer_source_structure_test/default wayland_window_source_test/default
```

Expected: FAIL on missing new private headers or line thresholds.

- [x] **Step 3: Move declarations into focused private headers**

Move Wayland app registry/input/cursor callback declarations, registered-window
declarations, and Vulkan state/swapchain/device helper declarations into the new
headers. Keep `wayland_application_internal.hpp`, `wayland_window_internal.hpp`,
and `vulkan_internal.hpp` as small internal aggregates.

- [x] **Step 4: Run GREEN focused tests**

Run:

```powershell
xmake test -y -P . platform_source_structure_test/default renderer_source_structure_test/default wayland_window_source_test/default win32_window_source_test/default wayland_pointer_button_test/default wayland_vulkan_surface_test/default vulkan_surface_validation_test/default vulkan_resize_test/default
```

Expected: PASS on available Windows targets. Run the WSL variant for Wayland
targets before moving to Task 2.

### Task 2: Remaining Long Implementation Bridges

**Files:**
- Modify: `tests/architecture/win32_window_source_test.cpp`
- Modify: `tests/architecture/platform_source_structure_test.cpp`
- Modify: `tests/architecture/renderer_source_structure_test.cpp`
- Create: `src/platform/win32/win32_window_proc_drag.cpp`
- Create: `src/platform/win32/win32_window_proc_lifecycle.cpp`
- Create: `src/platform/win32/win32_window_proc_pointer.cpp`
- Create: `src/platform/win32/win32_window_proc_keyboard.cpp`
- Create: `src/platform/linux/wayland_application_window_registry.cpp`
- Create: `src/platform/linux/wayland_application_window_creation.cpp`
- Create: `src/renderer/vulkan/vulkan_swapchain_query.cpp`
- Create: `src/renderer/vulkan/vulkan_swapchain_create_info.cpp`
- Create: `src/renderer/vulkan/vulkan_report_text_submission.cpp`
- Create: `src/renderer/vulkan/vulkan_report_submission_stats.cpp`
- Modify: `src/platform/win32/win32_window_proc.cpp`
- Modify: `src/platform/linux/wayland_application_windows.cpp`
- Modify: `src/renderer/vulkan/vulkan_swapchain_create.cpp`
- Modify: `src/renderer/vulkan/vulkan_report_submission.cpp`

- [x] **Step 1: Add failing bridge split tests**

Require the new bridge files to exist and require the old bridge files to be
dispatch-only:

```cpp
line_count(read_source("src/platform/win32/win32_window_proc.cpp")) <= 80
line_count(read_source("src/platform/linux/wayland_application_windows.cpp")) <= 100
line_count(read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp")) <= 100
line_count(read_source("src/renderer/vulkan/vulkan_report_submission.cpp")) <= 100
```

- [x] **Step 2: Run RED tests**

Run:

```powershell
xmake test -y -P . win32_window_source_test/default platform_source_structure_test/default renderer_source_structure_test/default
```

Expected: FAIL on missing files and line thresholds.

- [x] **Step 3: Split bridge implementations**

Move Win32 test-drag, lifecycle/paint, pointer, and keyboard/text message
handlers out of `win32_window_proc.cpp`. Move Wayland registry and creation
logic out of `wayland_application_windows.cpp`. Move Vulkan surface query and
create-info construction out of `vulkan_swapchain_create.cpp`. Move text page
submission and statistics out of `vulkan_report_submission.cpp`.

- [x] **Step 4: Run GREEN focused tests**

Run:

```powershell
xmake test -y -P . win32_window_source_test/default win32_input_event_test/default win32_text_input_test/default platform_source_structure_test/default renderer_source_structure_test/default vulkan_resize_test/default vulkan_frame_lifetime_test/default
```

Expected: PASS on Windows. Run WSL Wayland/renderer available-target subset
before moving to public headers.

### Task 3: Public UI Header Surgery

**Files:**
- Modify: `tests/architecture/ui_source_structure_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`
- Create: `include/cgpui/ui/text_edit_actions.hpp`
- Create: `include/cgpui/ui/text_shape.hpp`
- Create: `include/cgpui/ui/text_glyphs.hpp`
- Create: `include/cgpui/ui/text_measurement.hpp`
- Create: `include/cgpui/ui/text_wrapping.hpp`
- Create: `include/cgpui/ui/text_hit_testing.hpp`
- Create: `include/cgpui/ui/runtime_callbacks.hpp`
- Create: `include/cgpui/ui/runtime_ids.hpp`
- Create: `include/cgpui/ui/runtime_handles.hpp`
- Create: `include/cgpui/ui/runtime_window_options.hpp`
- Create: `include/cgpui/ui/runtime_app_context.hpp`
- Create: `include/cgpui/ui/runtime_actions.hpp`
- Create: `include/cgpui/ui/runtime_events.hpp`
- Create: `include/cgpui/ui/runtime_diagnostics.hpp`
- Create: `include/cgpui/ui/runtime_input_state.hpp`
- Create: `include/cgpui/ui/runtime_context.hpp`
- Create: `src/ui/window_runtime_internal.hpp`
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `include/cgpui/ui/text_model.hpp`
- Modify: `include/cgpui/ui/text_layout.hpp`
- Modify: `include/cgpui/ui/runtime.hpp`
- Modify: `include/cgpui/ui/runtime_types.hpp`
- Modify: `include/cgpui/ui/window_runtime.hpp`

- [x] **Step 1: Add failing public header tests**

Require the new public headers and lower line thresholds:

```cpp
line_count(read_source("include/cgpui/ui/text_model.hpp")) <= 220
line_count(read_source("include/cgpui/ui/text_layout.hpp")) <= 220
line_count(read_source("include/cgpui/ui/runtime_types.hpp")) <= 220
line_count(read_source("include/cgpui/ui/window_runtime.hpp")) <= 220
```

- [x] **Step 2: Run RED tests**

Run:

```powershell
xmake test -y -P . ui_source_structure_test/default ui_header_cleanliness/default text_model_test/default window_runtime_test/default
```

Expected: FAIL on missing split headers and current line thresholds.

- [x] **Step 3: Move inline non-template bodies to source files**

Create or expand:

```text
src/ui/text_model.cpp
src/ui/text_model_history.cpp
src/ui/text_model_navigation.cpp
src/ui/text_model_selection.cpp
src/ui/text_shape.cpp
src/ui/text_glyph_raster.cpp
src/ui/text_measurement.cpp
src/ui/text_wrapping.cpp
src/ui/text_hit_testing.cpp
```

Move non-template public inline bodies out of the public headers. Keep template
definitions in template implementation headers only when required by C++.

- [x] **Step 4: Shrink runtime public declarations**

Move private `WindowRuntime` state and private helper declarations into
`src/ui/window_runtime_internal.hpp`. Keep `window_runtime.hpp` focused on the
public runtime facade. If the class remains above threshold, introduce focused
runtime facade types and move public calls through those domains.

- [x] **Step 5: Run GREEN public header tests**

Run:

```powershell
xmake test -y -P . ui_source_structure_test/default ui_header_cleanliness/default prelude_header_cleanliness/default window_runtime_test/default text_model_test/default app_runner_test/default render_view_test/default style_test/default
```

Expected: PASS on Windows and WSL.

- [x] **Follow-up audit: complete text implementation split**

After review, Task 3 now also requires and verifies the five text implementation
translation units:

```text
src/ui/text_shape.cpp
src/ui/text_glyph_raster.cpp
src/ui/text_measurement.cpp
src/ui/text_wrapping.cpp
src/ui/text_hit_testing.cpp
```

`ui_source_structure_test` guards their existence and rejects the `" inline "`
token in the matching public text leaf headers. These files are compiled into
`cgpui_renderer` and removed from `cgpui_ui` because Vulkan renderer code
directly consumes fallback glyph rasterization and text paint metadata.

### Task 4: Public Element and Style Header Surgery

**Files:**
- Modify: `tests/architecture/ui_source_structure_test.cpp`
- Create: `include/cgpui/ui/element_tree_templates.hpp`
- Create: `include/cgpui/ui/element_layout_nodes.hpp`
- Create: `include/cgpui/ui/element_style_nodes.hpp`
- Create: `include/cgpui/ui/element_button_nodes.hpp`
- Create: `include/cgpui/ui/element_pointer_nodes.hpp`
- Create: `include/cgpui/ui/element_focus_nodes.hpp`
- Create: `include/cgpui/ui/element_builder_core.hpp`
- Create: `include/cgpui/ui/text_input_builder.hpp`
- Create: `include/cgpui/ui/label_builder.hpp`
- Create: `include/cgpui/ui/button_builder.hpp`
- Create: `include/cgpui/ui/scrollable_list_builder.hpp`
- Create: `include/cgpui/ui/style_box.hpp`
- Create: `include/cgpui/ui/style_text.hpp`
- Create: `include/cgpui/ui/style_layout.hpp`
- Create: `include/cgpui/ui/style_animation.hpp`
- Create: `include/cgpui/ui/style_overlay.hpp`
- Create: `include/cgpui/ui/style_state.hpp`
- Create: `src/ui/element_tree.cpp`
- Create: `src/ui/style_tween.cpp`
- Modify: current element and style aggregate headers.

- [x] **Step 1: Add failing element/style header tests**

Require the new element and style headers and enforce:

```cpp
line_count(read_source("include/cgpui/ui/element_tree.hpp")) <= 220
line_count(read_source("include/cgpui/ui/element_containers.hpp")) <= 220
line_count(read_source("include/cgpui/ui/element_interaction_nodes.hpp")) <= 220
line_count(read_source("include/cgpui/ui/element_builder.hpp")) <= 220
line_count(read_source("include/cgpui/ui/widget_builders.hpp")) <= 220
line_count(read_source("include/cgpui/ui/style_core.hpp")) <= 220
```

- [x] **Step 2: Run RED tests**

Run:

```powershell
xmake test -y -P . ui_source_structure_test/default ui_header_cleanliness/default element_test/default style_test/default render_view_test/default
```

Expected: FAIL on missing files and oversized headers.

- [x] **Step 3: Move element and style bodies**

Move non-template `ElementTree` bodies to focused `src/ui/element_tree_*`
files, keep template element-state helpers in `element_tree_templates.hpp`, and
split node, builder, paint, and style domains according to the file list above.

- [x] **Step 4: Run GREEN focused tests**

Run:

```powershell
xmake test -y -P . ui_source_structure_test/default ui_header_cleanliness/default element_test/default style_test/default render_view_test/default window_runtime_test/default
```

Expected: PASS on Windows and WSL.

### Task 5: Renderer, Platform, and Core Public Header Surgery

**Files:**
- Modify: `tests/architecture/renderer_source_structure_test.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`
- Create: renderer split headers listed in the structural optimization design.
- Create: platform split headers listed in the structural optimization design.
- Create: core event split headers listed in the structural optimization design.
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `include/cgpui/core/events.hpp`

- [x] **Step 1: Add failing renderer/platform/core header tests**

Require renderer aggregate headers to stay thin and leaf headers to be under
220 lines. Require platform and core event aggregates to be thin.

- [x] **Step 2: Run RED tests**

Run:

```powershell
xmake test -y -P . renderer_source_structure_test/default core_header_cleanliness/default vulkan_solid_rect_test/default vulkan_surface_validation_test/default win32_window_source_test/default wayland_window_source_test/default
```

Expected: FAIL on missing split headers or current aggregate contents.

- [x] **Step 3: Split public renderer/platform/core headers**

Move glyph atlas, renderer reports, platform accessibility/menu/dialog/chrome,
and core event declarations into focused public headers. Keep the old public
headers as compatibility aggregates.

- [x] **Step 4: Run GREEN focused tests**

Run:

```powershell
xmake test -y -P . renderer_source_structure_test/default core_header_cleanliness/default vulkan_solid_rect_test/default vulkan_surface_validation_test/default vulkan_resize_test/default win32_window_source_test/default wayland_window_source_test/default clipboard_test/default
```

Expected: PASS on Windows and WSL.

### Task 6: Test Suite Structure Split

**Files:**
- Modify: `tests/architecture/ui_source_structure_test.cpp`
- Create: `tests/ui/window_runtime_test_support.hpp`
- Create: `tests/ui/window_runtime_input_test.cpp`
- Create: `tests/ui/window_runtime_focus_test.cpp`
- Create: `tests/ui/window_runtime_actions_test.cpp`
- Create: `tests/ui/window_runtime_text_test.cpp`
- Create: `tests/ui/window_runtime_rendering_test.cpp`
- Create: `tests/ui/window_runtime_scheduling_test.cpp`
- Create: `tests/ui/window_runtime_multiwindow_test.cpp`
- Create: `tests/ui/window_runtime_theme_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `xmake.lua`

- [x] **Step 1: Add failing test structure guard**

Add a structure test that requires the new test files and enforces that
`tests/ui/window_runtime_test.cpp` no longer owns all runtime behavior cases.

- [x] **Step 2: Run RED tests**

Run:

```powershell
xmake test -y -P . ui_source_structure_test/default window_runtime_test/default
```

Expected: FAIL on missing split test files.

- [x] **Step 3: Move runtime tests by domain**

Move existing test cases from `window_runtime_test.cpp` into the new files. Keep
`window_runtime_test.cpp` as a small smoke and shared-runtime regression entry.

- [x] **Step 4: Run GREEN focused tests**

Run:

```powershell
xmake test -y -P . window_runtime_test/default window_runtime_input_test/default window_runtime_focus_test/default window_runtime_actions_test/default window_runtime_text_test/default window_runtime_rendering_test/default window_runtime_scheduling_test/default window_runtime_multiwindow_test/default window_runtime_theme_test/default ui_source_structure_test/default
```

Expected: PASS on Windows and WSL after xmake target registration.

### Task 7: Final Aggregation and Merge Readiness

**Files:**
- Modify: `findings.md`
- Modify: `progress.md`

- [x] **Step 1: Run Windows aggregation**

Run:

```powershell
xmake f -c -m debug -P .
xmake test -P .
```

Expected: all Windows tests pass.

- [x] **Step 2: Run WSL aggregation**

Run:

```powershell
wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/structural-optimization -- bash -lc 'XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'
```

Expected: all WSL Arch Linux tests pass.

- [x] **Step 3: Run whitespace checks**

Run:

```powershell
git diff --check
```

Expected: exit code 0. CRLF normalization warnings are acceptable only when the
exit code remains 0.

- [x] **Step 4: Update durable notes**

Update `findings.md` and `progress.md` with final line-count outcomes, split
files, and verification commands.

## Self-Review

- Spec coverage: covers private headers, implementation bridges, public UI,
  renderer/platform/core headers, test structure, and final verification.
- Placeholder scan: no unresolved placeholder names are present.
- Type consistency: all named paths match the current CGPUI directory layout.
- Scope: structure-only. No macOS parity and no new GPUI behavior are part of
  this plan.
