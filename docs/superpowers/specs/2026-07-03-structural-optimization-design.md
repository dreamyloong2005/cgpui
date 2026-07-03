# CGPUI Structural Optimization Design

**Goal:** Convert the current post-Step-218 codebase from "split enough to avoid
old monoliths" to a deliberately small-file architecture for Windows and Linux.

**User direction:** Prefer thorough structural optimization over compatibility
or low-risk migration. Breaking public header layout, moving inline bodies, and
introducing private implementation boundaries are acceptable when they produce
clearer long-term structure.

## Current Audit

The latest scan of `.worktrees/structural-optimization` shows that source
implementation files are no longer the dominant problem:

| Area | Files | Max lines | Files over 120 | Files over 150 |
| --- | ---: | ---: | ---: | ---: |
| `src/platform/linux` | 56 | 177 | 4 | 1 |
| `src/renderer/vulkan` | 35 | 151 | 4 | 1 |
| `src/ui` | 72 | 128 | 4 | 0 |
| `src/platform/win32` | 23 | 149 | 2 | 0 |
| `src/platform` | 20 | 121 | 1 | 0 |

The largest remaining implementation or private files are:

- `src/platform/linux/wayland_application_internal.hpp` at 177 lines.
- `src/renderer/vulkan/vulkan_internal.hpp` at 151 lines.
- `src/platform/win32/win32_window_proc.cpp` at 149 lines.
- `src/renderer/vulkan/vulkan_swapchain_create.cpp` at 145 lines.
- `src/platform/linux/wayland_application_windows.cpp` at 140 lines.
- `src/platform/linux/wayland_window_internal.hpp` at 138 lines.
- `src/renderer/vulkan/vulkan_report_submission.cpp` at 138 lines.
- `src/platform/win32/win32_application.cpp` at 138 lines.
- `src/platform/linux/wayland_window.cpp` at 135 lines.

Public headers are now the real hotspot:

| Area | Files | Max lines | Files over 200 | Files over 300 |
| --- | ---: | ---: | ---: | ---: |
| `include/cgpui/ui` | 29 | 688 | 13 | 11 |
| `include/cgpui/renderer` | 6 | 272 | 2 | 0 |
| `include/cgpui/platform` | 4 | 182 | 0 | 0 |
| `include/cgpui/core` | 5 | 160 | 0 | 0 |

The largest public headers are:

- `include/cgpui/ui/text_model.hpp` at 688 lines.
- `include/cgpui/ui/runtime_types.hpp` at 567 lines.
- `include/cgpui/ui/text_layout.hpp` at 481 lines.
- `include/cgpui/ui/window_runtime.hpp` at 464 lines.
- `include/cgpui/ui/element_tree.hpp` at 461 lines.
- `include/cgpui/ui/element_containers.hpp` at 401 lines.
- `include/cgpui/ui/element_interaction_nodes.hpp` at 380 lines.
- `include/cgpui/ui/element_builder.hpp` at 338 lines.
- `include/cgpui/ui/style_core.hpp` at 335 lines.
- `include/cgpui/ui/runtime_templates.hpp` at 329 lines.
- `include/cgpui/ui/widget_builders.hpp` at 301 lines.

Test files also need a later structure pass. `tests/ui/window_runtime_test.cpp`
is 8619 lines and has become the test-side equivalent of the old UI runtime
implementation monolith.

## Chosen Approach

Use an aggressive four-layer split.

1. Private declaration surgery first.

   Shrink private internal headers by moving state into focused private structs,
   moving protocol callback declarations to focused callback headers, and moving
   cross-file helper declarations into domain-specific private headers. This
   keeps source edits local before public API headers are disrupted.

2. Remaining long bridge implementation files second.

   Split message dispatchers and linear planners only after private declarations
   are smaller. The main targets are Win32 window proc dispatch, Vulkan
   swapchain creation, Vulkan submission planning, and Wayland application
   window registry/create paths.

3. Public API header surgery third.

   Compatibility aggregate headers remain thin, but public leaf headers stop
   being large buckets. Non-template inline bodies move to `.cpp` files.
   Template-heavy surfaces move to explicit template implementation headers.
   Large public classes either move private state behind private implementation
   objects or split public responsibilities into focused facade types.

4. Test-suite structure last.

   Once source and public headers are stable, split large tests by API domain
   so future structure regressions are easier to isolate.

## Target Structure

Hard targets for the aggressive pass:

- Public aggregate headers such as `ui.hpp`, `element.hpp`, `runtime.hpp`,
  `text.hpp`, `style.hpp`, and `renderer.hpp` stay under 40 lines.
- Public leaf headers should be under 220 lines after the public surgery pass.
- Private implementation headers should be under 120 lines.
- Normal `.cpp` implementation files should be under 120 lines.
- Message/protocol dispatch entry files should be under 90 lines and delegate
  to focused handlers.
- Architecture tests should encode these thresholds and file-boundary markers.

## Public Header Split Map

### Text

- Keep `include/cgpui/ui/text.hpp` as the aggregate.
- Split text model declarations and implementation:
  - `text_edit_actions.hpp`
  - `text_model.hpp` as the small public class declaration
  - `text_model_history.hpp`
  - `text_model_navigation.hpp`
  - `text_model_selection.hpp`
  - `src/ui/text_model.cpp`
  - `src/ui/text_model_history.cpp`
  - `src/ui/text_model_navigation.cpp`
  - `src/ui/text_model_selection.cpp`
- Split layout and glyph helpers:
  - `text_shape.hpp`
  - `text_glyphs.hpp`
  - `text_measurement.hpp`
  - `text_wrapping.hpp`
  - `text_hit_testing.hpp`
  - `src/ui/text_shape.cpp`
  - `src/ui/text_glyph_raster.cpp`
  - `src/ui/text_measurement.cpp`
  - `src/ui/text_wrapping.cpp`
  - `src/ui/text_hit_testing.cpp`

### Runtime

- Keep `include/cgpui/ui/runtime.hpp` as the aggregate.
- Split `runtime_types.hpp` into:
  - `runtime_callbacks.hpp`
  - `runtime_ids.hpp`
  - `runtime_handles.hpp`
  - `runtime_window_options.hpp`
  - `runtime_app_context.hpp`
  - `runtime_actions.hpp`
  - `runtime_events.hpp`
  - `runtime_diagnostics.hpp`
  - `runtime_input_state.hpp`
  - `runtime_context.hpp`
- Shrink `window_runtime.hpp` by moving private state and private helper
  declarations into `src/ui/window_runtime_internal.hpp`. If a public class
  declaration remains too large, introduce focused runtime facade types and move
  public calls through those domains.
- Split `runtime_templates.hpp` into:
  - `runtime_global_templates.hpp`
  - `runtime_entity_templates.hpp`
  - `runtime_subscription_templates.hpp`
  - `runtime_element_state_templates.hpp`

### Elements

- Keep `include/cgpui/ui/element.hpp`, `element_nodes.hpp`, and
  `element_builders.hpp` as thin aggregates.
- Move non-template `ElementTree` bodies to `src/ui/element_tree.cpp`.
- Keep template element-state helpers in `element_tree_templates.hpp`.
- Split nodes and builders by responsibility:
  - `element_layout_nodes.hpp`
  - `element_style_nodes.hpp`
  - `element_text_nodes.hpp`
  - `element_button_nodes.hpp`
  - `element_pointer_nodes.hpp`
  - `element_focus_nodes.hpp`
  - `element_scroll_nodes.hpp`
  - `element_builder_core.hpp`
  - `text_input_builder.hpp`
  - `label_builder.hpp`
  - `button_builder.hpp`
  - `scrollable_list_builder.hpp`

### Style

- Keep `style.hpp` as the aggregate.
- Split `style_core.hpp` into:
  - `style_box.hpp`
  - `style_text.hpp`
  - `style_layout.hpp`
  - `style_animation.hpp`
  - `style_overlay.hpp`
  - `style_state.hpp`
- Move non-template style tween helpers to `src/ui/style_tween.cpp`.

### Renderer

- Keep `renderer.hpp` as the aggregate.
- Split `glyph_atlas.hpp` into:
  - `glyph_atlas_types.hpp`
  - `glyph_uploads.hpp`
  - `image_uploads.hpp`
  - `glyph_texture_resources.hpp`
  - `glyph_cache.hpp`
- Split `renderer_reports.hpp` into:
  - `renderer_text_reports.hpp`
  - `renderer_image_reports.hpp`
  - `renderer_submission_reports.hpp`
  - `renderer_geometry_reports.hpp`
  - `renderer_frame_reports.hpp`

### Platform and Core

- Split `include/cgpui/platform/platform.hpp` into platform-neutral
  accessibility, native menu, file dialog, window chrome, diagnostics, window,
  and application headers.
- Split `include/cgpui/core/events.hpp` into window, pointer, drag/drop,
  keyboard, text/IME, and `PlatformEvent` aggregate headers.

## Private Header Split Map

### Wayland Application

- Replace `wayland_application_internal.hpp` as a broad class declaration with
  smaller private boundaries:
  - `wayland_application_core_internal.hpp`
  - `wayland_application_registry_internal.hpp`
  - `wayland_application_input_internal.hpp`
  - `wayland_application_cursor_internal.hpp`
  - `wayland_application_window_registry_internal.hpp`
  - `wayland_application_services_internal.hpp`
- Move listener callback declarations to focused private headers and keep the
  class declaration itself under 120 lines.

### Wayland Window

- Split `wayland_window_internal.hpp` into:
  - `wayland_window_internal.hpp` for the primary window only
  - `wayland_registered_window_internal.hpp`
  - `wayland_window_configure_internal.hpp`
  - `wayland_window_event_sink_internal.hpp`

### Vulkan

- Split `vulkan_internal.hpp` into:
  - `vulkan_platform_internal.hpp`
  - `vulkan_state_internal.hpp`
  - `vulkan_swapchain_internal.hpp`
  - `vulkan_device_internal.hpp`
  - `vulkan_sync_internal.hpp`
  - `vulkan_command_internal.hpp`
- The public `VulkanRendererState` declaration should stop carrying every
  private helper declaration in one file.

## Verification Strategy

Each pass should start with RED architecture coverage. A structure test should
fail for the currently oversized file or missing split file before the
implementation split lands.

Focused Windows command:

```powershell
xmake test -y -P . platform_source_structure_test/default renderer_source_structure_test/default ui_source_structure_test/default win32_window_source_test/default wayland_window_source_test/default clipboard_test/default window_runtime_test/default text_model_test/default app_runner_test/default render_view_test/default style_test/default
```

Focused WSL command:

```powershell
wsl -d archlinux --cd /mnt/d/Dev/Projects/cgpui/.worktrees/structural-optimization -- bash -lc 'XMAKE_ROOT=y xmake test -y -P . platform_source_structure_test/default renderer_source_structure_test/default ui_source_structure_test/default win32_window_source_test/default wayland_window_source_test/default clipboard_test/default wayland_pointer_button_test/default wayland_pointer_scroll_test/default wayland_keyboard_test/default wayland_vulkan_surface_test/default window_runtime_test/default text_model_test/default app_runner_test/default render_view_test/default style_test/default'
```

Full gates remain Windows and WSL `xmake f -c -m debug -P .` plus
`xmake test -P .` when a pass touches public headers or shared runtime state.

## Design Self-Review

- Placeholder scan: no placeholder tasks or unresolved names are used.
- Scope check: the plan is structure-only and does not add GPUI behavior.
- Platform check: Windows and Linux remain the active targets. macOS is not part
  of this pass.
- Risk check: public API compatibility is intentionally lower priority than
  long-term file structure.
