# Complete GPUI Parity Ledger

Pinned upstream revision:
`5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`.

This ledger is the source of truth for full CGPUI replication work. It maps
upstream GPUI concepts to the current C++23 surface, assigns a status, and
records the next phase that must close each required gap.

Status meanings:

- `Required`: must be implemented for the active Windows/Linux target.
- `Adapted`: GPUI concept exists in C++ form but the spelling or ownership is
  intentionally C++-native.
- `Deferred`: real upstream surface, but not active before a later platform or
  production-depth gate.
- `Non-goal`: intentionally out of scope for the current user-approved target.

## Platform Targets

- Windows: Win32 + Vulkan.
- Linux: Wayland + Vulkan.
- macOS: Cocoa + Metal after Windows/Linux core APIs stabilize.
- X11 remains deferred unless the user explicitly chooses strict upstream Linux
  backend parity.

| platform_surface | status |
|---|---|
| gpui Windows backend | Required |
| gpui_platform wayland feature | Required |
| gpui_platform x11 feature | Deferred |
| gpui macOS backend | Deferred |

## Required Surface Summary

| upstream_gpui | cgpui_target | status | evidence | next_step |
|---|---|---|---|---|
| gpui_platform::application | `create_platform_application` + `run_app` | Adapted | `include/cgpui/platform/platform_application.hpp`, `include/cgpui/ui/runtime_rendering.hpp` | Step 259 facade keeps low-level APIs |
| gpui::Application | `Application` | Adapted | `include/cgpui/app/application.hpp`, `src/app/application.cpp`, `tests/api_parity/application_facade_test.cpp` | Phase B app service depth |
| gpui::App | `App` facade from `AppContext::app()` and `Context<T>::app_context()` app-domain capability, plus `try_open_window(...) -> Result<AppOpenedWindow>` public window-opening result conventions on `App`, `AppContext`, and `WindowRuntime`, `try_create_renderer(...) -> Result<Renderer*>` renderer-creation result conventions on `WindowRuntime`, `try_resize_surface(...) -> Result<void>` renderer-resize result conventions on `WindowRuntime`, and `try_draw_frame() -> Result<void>` renderer frame/redraw result conventions on `WindowRuntime` | Adapted | `include/cgpui/app/app_facade.hpp`, `src/app/app_facade.cpp`, `src/app/app_context_facade.cpp`, `src/ui/runtime_window_results.cpp`, `src/ui/runtime_renderer_results.cpp`, `src/ui/runtime_renderer_resize_results.cpp`, `src/ui/runtime_renderer_frame_results.cpp`, `tests/api_parity/app_window_context_test.cpp`, `tests/api_parity/app_context_capability_test.cpp`, `tests/api_parity/public_result_conventions_test.cpp`, `tests/api_parity/renderer_result_conventions_test.cpp`, `tests/api_parity/renderer_resize_result_conventions_test.cpp`, `tests/api_parity/renderer_frame_result_conventions_test.cpp` | Phase B public API compatibility examples |
| gpui::Window | `Window` facade from `WindowRuntimeContext::window()` / `current_window()` plus `WindowContextCapability` from `Context<T>::window_context()` | Adapted | `include/cgpui/app/window.hpp`, `include/cgpui/ui/window_context.hpp`, `src/ui/window.cpp`, `src/ui/window_context.cpp`, `tests/api_parity/app_window_context_test.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/window_context_capability_test.cpp` | Phase B/F window service depth |
| gpui::WindowOptions | `WindowOptions` | Adapted | `include/cgpui/ui/runtime_window_options.hpp` | Phase B/F |
| gpui::Context<T> | `Context<T>` alias with app/window/entity helpers, `Context<T>::app_context()` app-domain capability, `Context<T>::view_context<T>()` view-domain capability, `Context<T>::window_context()` window-domain capability, `ElementContextCapability` from `Context<T>::element_context(ElementId)`, `AsyncContextCapability` from `Context<T>::async_context()`, `TestContextCapability` from `Context<T>::test_context()`, `Context<T>::new_entity<T>(...)` creation, weak entity upgrades, `observe_entity(...)` helpers, entity-to-entity observation helpers, window/view observation helpers, `update_entity(...)` transaction helpers, `invalidate_entity(...)` helpers, entity deletion helpers, deterministic subscription lifetime/unsubscribe behavior, view-removal subscription cleanup, and runtime-token cross-context boundaries | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/async_context.hpp`, `include/cgpui/ui/test_context.hpp`, `include/cgpui/ui/element_context.hpp`, `include/cgpui/ui/view_context.hpp`, `include/cgpui/ui/window_context.hpp`, `include/cgpui/ui/runtime_context.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/runtime_observations.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/app_context_capability_test.cpp`, `tests/api_parity/view_context_capability_test.cpp`, `tests/api_parity/window_context_capability_test.cpp`, `tests/api_parity/element_context_capability_test.cpp`, `tests/api_parity/async_context_capability_test.cpp`, `tests/api_parity/test_context_capability_test.cpp`, `tests/api_parity/entity_lifecycle_creation_test.cpp`, `tests/api_parity/entity_weak_handle_semantics_test.cpp`, `tests/api_parity/entity_observation_test.cpp`, `tests/api_parity/entity_to_entity_observation_test.cpp`, `tests/api_parity/window_view_observation_test.cpp`, `tests/api_parity/subscription_lifetime_test.cpp`, `tests/ui/window_runtime_observation_closure_test.cpp`, `tests/api_parity/entity_transaction_test.cpp`, `tests/api_parity/entity_invalidation_test.cpp`, `tests/api_parity/entity_deletion_test.cpp` | Phase B observation depth |
| gpui::View<T> | `ViewContextCapability<T>` plus runtime-token-scoped `ViewHandle<T>` and `WeakViewHandle<T>` over `ViewId`, with view observation helpers and view-removal subscription cleanup | Adapted | `include/cgpui/ui/view_context.hpp`, `include/cgpui/ui/view_handle.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/runtime_observations.cpp`, `tests/api_parity/view_handle_spelling_test.cpp`, `tests/api_parity/view_context_capability_test.cpp`, `tests/api_parity/window_view_observation_test.cpp`, `tests/ui/window_runtime_view_handle_token_test.cpp`, `tests/ui/window_runtime_observation_closure_test.cpp` | Phase B view lifecycle depth |
| gpui::AsyncApp | async task handles, cancellation, and foreground/background task Result conventions through `try_spawn_task(...) -> Result<TaskHandle>` and `try_spawn_background_task(...) -> Result<TaskHandle>` on `WindowRuntime`, `WindowRuntimeContext`, and `AsyncContextCapability` | Required | `src/ui/runtime_tasks.cpp`, `src/ui/runtime_task_results.cpp`, `src/ui/runtime_context_scheduling.cpp`, `src/ui/async_context.cpp`, `tests/api_parity/async_spawn_result_conventions_test.cpp`; not full async app context | Phase G |
| gpui::AsyncWindowContext | `AsyncContextCapability` groups defer, timer, animation, foreground task, background task, update-batch scheduling, and async-spawn Result conventions from `Context<T>::async_context()` | Adapted | `include/cgpui/ui/async_context.hpp`, `src/ui/async_context.cpp`, `src/ui/runtime_task_results.cpp`, `tests/api_parity/async_context_capability_test.cpp`, `tests/api_parity/async_spawn_result_conventions_test.cpp`, `tests/ui/window_runtime_scheduling_test.cpp` | Phase G full async executor depth |
| gpui::TestAppContext | `TestContextCapability` groups runtime id/view id, live input/event/action observability, invalidation/diagnostics snapshots with entity/window/view observer counts, timer advancement, timer cancellation, task completion draining, run-until-parked timer/async advancement helpers, redraw/frame pump helpers including `try_draw_frame() -> Result<void>`, direct `dispatch_keystroke(...)`, grammar-backed `simulate_keystrokes(...)`, direct pointer move/button/scroll simulation, direct window activation/focus dispatch, element focus/release helpers, direct clipboard read/write helpers, and copy/cut/paste clipboard forwarding from `Context<T>::test_context()` | Adapted | `include/cgpui/ui/test_context.hpp`, `include/cgpui/ui/runtime_diagnostics.hpp`, `src/ui/test_context.cpp`, `src/ui/test_context_scheduling.cpp`, `src/ui/test_context_rendering.cpp`, `src/ui/test_context_keystrokes.cpp`, `src/ui/test_context_pointer.cpp`, `src/ui/test_context_focus.cpp`, `src/ui/test_context_clipboard.cpp`, `src/ui/runtime_clipboard.cpp`, `src/ui/runtime_diagnostic_snapshot.cpp`, `tests/api_parity/test_context_capability_test.cpp`, `tests/api_parity/renderer_frame_result_conventions_test.cpp`, `tests/ui/test_context_time_async_test.cpp`, `tests/ui/test_context_frame_pump_test.cpp`, `tests/ui/test_context_keystroke_simulation_test.cpp`, `tests/ui/test_context_pointer_simulation_test.cpp`, `tests/ui/test_context_focus_activation_test.cpp`, `tests/ui/test_context_clipboard_test.cpp`, `tests/ui/window_runtime_scheduling_test.cpp`, `tests/ui/window_runtime_observation_diagnostics_test.cpp` | Phase G fuller simulated input/test macro depth |
| gpui::Entity<T> | `Entity<T>`, `Model<T>`, `EntityHandle<T>`, `WeakEntity<T>`, context-created entity handles, weak upgrade/read semantics, entity observation helpers, entity-to-entity observation helpers, deterministic `Subscription` lifetime/unsubscribe behavior, value-returning update transactions, entity invalidation helpers, entity deletion helpers, and runtime-token cross-context boundaries | Adapted | `include/cgpui/core/entity.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/subscription.cpp`, `src/ui/runtime_subscriptions.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/entity_lifecycle_creation_test.cpp`, `tests/api_parity/entity_weak_handle_semantics_test.cpp`, `tests/api_parity/entity_observation_test.cpp`, `tests/api_parity/entity_to_entity_observation_test.cpp`, `tests/api_parity/subscription_lifetime_test.cpp`, `tests/api_parity/entity_transaction_test.cpp`, `tests/api_parity/entity_invalidation_test.cpp`, `tests/api_parity/entity_deletion_test.cpp` | Phase B observation depth |
| gpui::Render | `Render<T>` concept over `View::render(Context<T>&)` | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/view.hpp`, `tests/api_parity/context_render_spelling_test.cpp` | Phase B view lifecycle depth |
| gpui::IntoElement | `IntoElement` alias plus `into_element` builders | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/element_builder_core.hpp`, `tests/api_parity/context_render_spelling_test.cpp` | Phase C |
| gpui::div | `div()` | Adapted | `include/cgpui/ui/element_builder_core.hpp` | Phase C style vocabulary |
| gpui::prelude | `cgpui/prelude.hpp` plus thin `cgpui/cgpui.hpp` compatibility aggregate | Adapted | `include/cgpui/prelude.hpp`, `include/cgpui/cgpui.hpp`, `tests/api_parity/public_authoring_surface_test.cpp`, `tests/header_cleanliness/prelude_header_cleanliness.cpp` | Phase B public authoring surface gate |
| gpui::actions! / action macro | `Action<T>` typed action concept, `action_name<T>()` / `action_name_v<T>`, typed register/dispatch overloads over the existing string action registry, action registration metadata for general/app/window/view/focused-element scopes, typed action command-palette metadata binding, action enablement metadata that suppresses disabled action dispatch, action bubbling through focused routes, key binding grammar parsing for `Context<T>::bind_key("ctrl-shift-s", "action")`, platform modifier semantics for `secondary-*` and `platform-*` bindings, keymap context filtering for app/window/view/focused-element bindings, partial key sequence matching for multi-chord bindings, disabled key scope filtering, and command palette key integration that parses `CommandPaletteEntry::key_binding` and installs derived or explicit `KeyBindingContext` bindings | Required | `include/cgpui/ui/action.hpp`, `include/cgpui/ui/key_binding.hpp`, `include/cgpui/ui/runtime_action_templates.hpp`, `include/cgpui/ui/runtime_action_enablement_templates.hpp`, `include/cgpui/ui/runtime_command_palette_templates.hpp`, `include/cgpui/ui/runtime_actions.hpp`, `src/ui/runtime_action_dispatch.cpp`, `src/ui/runtime_action_registration.cpp`, `src/ui/runtime_action_metadata.cpp`, `src/ui/runtime_command_palette_keys.cpp`, `src/ui/runtime_key_binding_grammar.cpp`, `src/ui/runtime_key_binding_modifiers.cpp`, `src/ui/runtime_key_binding_contexts.cpp`, `src/ui/runtime_key_binding_sequences.cpp`, `tests/api_parity/typed_action_surface_test.cpp`, `tests/ui/typed_action_dispatch_test.cpp`, `tests/ui/action_scope_metadata_test.cpp`, `tests/ui/typed_action_command_metadata_test.cpp`, `tests/ui/action_enablement_metadata_test.cpp`, `tests/ui/action_bubbling_test.cpp`, `tests/ui/key_binding_grammar_test.cpp`, `tests/ui/key_binding_platform_modifier_test.cpp`, `tests/ui/keymap_context_test.cpp`, `tests/ui/key_binding_partial_match_test.cpp`, `tests/ui/key_binding_disabled_scope_test.cpp`, `tests/ui/command_palette_key_integration_test.cpp`; action macro payloads still missing | Phase B action metadata/key dispatch depth |
| gpui key_context | `KeyBindingContext` for app/window/view/focused-element key binding activation | Required | context-aware key bindings exist without the full upstream key-context tree API | Phase B |
| gpui keymap dispatch | `KeyBinding`, `KeyBindingChord`, `KeyBindingContext`, `parse_key_binding(...)`, `parse_key_binding(..., DesktopPlatformTarget)`, `Context<T>::bind_key("ctrl-shift-s", "action")`, `Context<T>::bind_key("ctrl-k ctrl-s", "action")`, `Context<T>::bind_key(..., KeyBindingContext)`, command-palette-owned key binding metadata, and action registry dispatch | Required | deterministic key binding plus GPUI-style chord grammar, platform modifier semantics, active context filtering, pending partial sequence matching, disabled key scope filtering, and command palette key integration exist; `secondary-*` maps to Ctrl on Windows/Linux and Super on macOS, while `platform-*` / `cmd-*` / `win-*` map to the platform key. Action macro payloads remain missing | Phase B |
| gpui element styling | `Style`, `StyleOverlay`, builders | Required | many primitives exist, tailwind-style vocabulary incomplete | Phase C |
| gpui uniform_list | `scrollable_list` skeleton | Required | stable items exist, virtualization incomplete | Phase C |
| gpui text system | `TextModel`, shaping, wrap, glyph records | Required | deterministic text depth exists, production shaping incomplete | Phase D |
| gpui image assets | image paint/upload skeleton | Required | in-memory RGBA8 only | Phase G |
| gpui SVG | no production SVG element/render path | Required | SVG is only tracked as future asset path | Phase C/G |
| gpui animation | deterministic animation clock/tween | Required | upstream animation graph not complete | Phase G |
| gpui::test | no macro-equivalent yet; public `TestContextCapability` exists with keystroke, pointer, window activation/focus, element focus, clipboard, run-until-parked timer/async, and redraw/frame pump simulation helpers | Required | internal tests plus public test-context facade, `simulate_keystrokes(...)`, `dispatch_pointer_*`, `dispatch_window_*`, `focus(...)`, `release_focus(...)`, `write_to_clipboard(...)`, `read_from_clipboard(...)`, copy/cut/paste helpers, `run_until_parked()`, `advance_time_until_parked(...)`, `request_redraw()`, `try_draw_frame()`, and `draw_frame()`, but no upstream-style test macro | Phase G |
| gpui accessibility | snapshot + UIA/AT-SPI facades | Required | production providers missing | Phase G |
| gpui platform services | menu/dialog/chrome skeletons plus `try_install_native_menu(...) -> Result<NativeMenuInstallation>` and `try_show_native_file_dialog(...) -> Result<NativeFileDialogResult>` on `WindowRuntime`, `AppContext`, and `ViewContext`/`WindowRuntimeContext` | Required | `src/ui/runtime_platform_service_results.cpp`, `src/ui/runtime_platform_services.cpp`, `src/ui/app_context_services.cpp`, `src/ui/runtime_context_platform.cpp`, `tests/api_parity/platform_service_result_conventions_test.cpp`; native services not production-depth | Phase B result conventions, then Phase F production services |
| gpui Windows backend | Win32 + Vulkan | Required | active target | Phase F |
| gpui_platform wayland feature | Wayland + Vulkan | Required | active Linux target | Phase F |
| gpui_platform x11 feature | Deferred | Deferred | not active by user decision | Optional Phase I |
| gpui macOS backend | Cocoa + Metal | Deferred | later platform track | Phase H |
| gpui wasm backend | Non-goal | Non-goal | not requested for CGPUI desktop target | no active phase |

## Categories

### Application and app context

Required rows: `gpui_platform::application`, `gpui::Application`,
`gpui::App`, `gpui::Window`, `gpui::WindowOptions`, platform service methods,
global state, window activation, quit/reopen, and error/result conventions.

### Entities and state

Required rows: `gpui::Entity<T>`, weak handles, reservations, update
transactions, observers, deterministic subscriptions, deletion, window/view
observation, and async/test context access to entities.

### Views and Render

Required rows: `gpui::Render`, root view creation, view lifecycle, invalidation,
render-to-element flow, child views, and public examples using only prelude
APIs.

### Elements and styling

Required rows: `gpui::div`, `IntoElement`, element lifecycle, layout, hit
testing, focus, pointer capture, scroll, style cascade, shadows, SVG/image
front-end elements, built-in widgets, and uniform list virtualization.

### Actions and key dispatch

Required rows: typed action authoring surface, typed registration and dispatch,
action macro equivalent, key contexts, keymap grammar, focus/key routing,
command palette integration, and disabled-context behavior.

### Platform services

Required rows: Win32 and Wayland window lifecycle, clipboard, drag/drop, IME,
menus, file dialogs, window chrome, cursor, accessibility, wakeups, native
diagnostics, and multi-window routing.

### Async executor

Required rows: app/window async contexts, task pool, priorities, structured
groups, cancellation propagation, async I/O hooks, and cross-thread entity
access rules.

### Test support

Required rows: GPUI test macro equivalent, `TestAppContext`, simulated input,
timer advancement, async advancement, platform service fakes, and example
smoke/pixel tests.

### Examples

The first example parity target is
`examples/api_parity/hello_world/main.cpp`, based on
`examples/hello_world.rs`. The tracked upstream example inventory includes:

- `hello_world`
- `animation`
- `gif_viewer`
- `image`
- `input`
- `opacity`
- `set_menus`
- `shadow`
- `svg`
- `text_wrapper`
- `uniform_list`
- `window`
- `window_positioning`
- `window_shadow`

## Phase A Closure

Phase A is complete when this ledger, the JSON export, the extractor, the
first parity example, and `gpui_parity_ledger_test/default` all pass on
Windows and WSL. Later phases close required rows by moving their `status` or
`next_step` fields only when tests prove the new behavior.
