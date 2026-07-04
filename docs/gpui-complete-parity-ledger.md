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
| gpui::App | `App` facade from `AppContext::app()` and `Context<T>::app_context()` app-domain capability | Adapted | `include/cgpui/app/app_facade.hpp`, `src/app/app_facade.cpp`, `src/app/app_context_facade.cpp`, `tests/api_parity/app_window_context_test.cpp`, `tests/api_parity/app_context_capability_test.cpp` | Phase B context API depth |
| gpui::Window | `Window` facade from `WindowRuntimeContext::window()` / `current_window()` plus `WindowContextCapability` from `Context<T>::window_context()` | Adapted | `include/cgpui/app/window.hpp`, `include/cgpui/ui/window_context.hpp`, `src/app/window.cpp`, `src/app/window_context.cpp`, `tests/api_parity/app_window_context_test.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/window_context_capability_test.cpp` | Phase B/F window service depth |
| gpui::WindowOptions | `WindowOptions` | Adapted | `include/cgpui/ui/runtime_window_options.hpp` | Phase B/F |
| gpui::Context<T> | `Context<T>` alias with app/window/entity helpers, `Context<T>::app_context()` app-domain capability, `Context<T>::view_context<T>()` view-domain capability, `Context<T>::window_context()` window-domain capability, `ElementContextCapability` from `Context<T>::element_context(ElementId)`, `AsyncContextCapability` from `Context<T>::async_context()`, `TestContextCapability` from `Context<T>::test_context()`, `Context<T>::new_entity<T>(...)` creation, weak entity upgrades, `observe_entity(...)` helpers, entity-to-entity observation helpers, `update_entity(...)` transaction helpers, `invalidate_entity(...)` helpers, entity deletion helpers, deterministic subscription lifetime/unsubscribe behavior, and runtime-token cross-context boundaries | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/async_context.hpp`, `include/cgpui/ui/test_context.hpp`, `include/cgpui/ui/element_context.hpp`, `include/cgpui/ui/view_context.hpp`, `include/cgpui/ui/window_context.hpp`, `include/cgpui/ui/runtime_context.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/app_context_capability_test.cpp`, `tests/api_parity/view_context_capability_test.cpp`, `tests/api_parity/window_context_capability_test.cpp`, `tests/api_parity/element_context_capability_test.cpp`, `tests/api_parity/async_context_capability_test.cpp`, `tests/api_parity/test_context_capability_test.cpp`, `tests/api_parity/entity_lifecycle_creation_test.cpp`, `tests/api_parity/entity_weak_handle_semantics_test.cpp`, `tests/api_parity/entity_observation_test.cpp`, `tests/api_parity/entity_to_entity_observation_test.cpp`, `tests/api_parity/subscription_lifetime_test.cpp`, `tests/api_parity/entity_transaction_test.cpp`, `tests/api_parity/entity_invalidation_test.cpp`, `tests/api_parity/entity_deletion_test.cpp` | Phase B observation depth |
| gpui::View<T> | `ViewContextCapability<T>` plus `ViewHandle<T>` and `WeakViewHandle<T>` over `ViewId` | Adapted | `include/cgpui/ui/view_context.hpp`, `include/cgpui/ui/view_handle.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `tests/api_parity/view_handle_spelling_test.cpp`, `tests/api_parity/view_context_capability_test.cpp` | Phase B view lifecycle depth |
| gpui::AsyncApp | async task handles and cancellation | Required | `runtime_tasks.cpp`, not full async app context | Phase G |
| gpui::AsyncWindowContext | `AsyncContextCapability` groups defer, timer, animation, foreground task, background task, and update-batch scheduling from `Context<T>::async_context()` | Adapted | `include/cgpui/ui/async_context.hpp`, `src/ui/async_context.cpp`, `tests/api_parity/async_context_capability_test.cpp`, `tests/ui/window_runtime_scheduling_test.cpp` | Phase G full async executor depth |
| gpui::TestAppContext | `TestContextCapability` groups runtime id/view id, input/event/action observability, invalidation/diagnostics snapshots, timer advancement, timer cancellation, and task completion draining from `Context<T>::test_context()` | Adapted | `include/cgpui/ui/test_context.hpp`, `src/ui/test_context.cpp`, `tests/api_parity/test_context_capability_test.cpp`, `tests/ui/window_runtime_scheduling_test.cpp` | Phase G fuller simulated input/test macro depth |
| gpui::Entity<T> | `Entity<T>`, `Model<T>`, `EntityHandle<T>`, `WeakEntity<T>`, context-created entity handles, weak upgrade/read semantics, entity observation helpers, entity-to-entity observation helpers, deterministic `Subscription` lifetime/unsubscribe behavior, value-returning update transactions, entity invalidation helpers, entity deletion helpers, and runtime-token cross-context boundaries | Adapted | `include/cgpui/core/entity.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/subscription.cpp`, `src/ui/runtime_subscriptions.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/entity_lifecycle_creation_test.cpp`, `tests/api_parity/entity_weak_handle_semantics_test.cpp`, `tests/api_parity/entity_observation_test.cpp`, `tests/api_parity/entity_to_entity_observation_test.cpp`, `tests/api_parity/subscription_lifetime_test.cpp`, `tests/api_parity/entity_transaction_test.cpp`, `tests/api_parity/entity_invalidation_test.cpp`, `tests/api_parity/entity_deletion_test.cpp` | Phase B observation depth |
| gpui::Render | `Render<T>` concept over `View::render(Context<T>&)` | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/view.hpp`, `tests/api_parity/context_render_spelling_test.cpp` | Phase B view lifecycle depth |
| gpui::IntoElement | `IntoElement` alias plus `into_element` builders | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/element_builder_core.hpp`, `tests/api_parity/context_render_spelling_test.cpp` | Phase C |
| gpui::div | `div()` | Adapted | `include/cgpui/ui/element_builder_core.hpp` | Phase C style vocabulary |
| gpui::prelude | `cgpui/prelude.hpp` plus thin `cgpui/cgpui.hpp` compatibility aggregate | Adapted | `include/cgpui/prelude.hpp`, `include/cgpui/cgpui.hpp`, `tests/api_parity/public_authoring_surface_test.cpp`, `tests/header_cleanliness/prelude_header_cleanliness.cpp` | Phase B public authoring surface gate |
| gpui::actions! / action macro | string action registry | Required | action dispatch exists, typed action parity missing | Phase B |
| gpui key_context | no key-context tree API yet | Required | key bindings exist without upstream key-context model | Phase B |
| gpui keymap dispatch | `KeyBinding` + action registry | Required | deterministic key binding exists, grammar incomplete | Phase B |
| gpui element styling | `Style`, `StyleOverlay`, builders | Required | many primitives exist, tailwind-style vocabulary incomplete | Phase C |
| gpui uniform_list | `scrollable_list` skeleton | Required | stable items exist, virtualization incomplete | Phase C |
| gpui text system | `TextModel`, shaping, wrap, glyph records | Required | deterministic text depth exists, production shaping incomplete | Phase D |
| gpui image assets | image paint/upload skeleton | Required | in-memory RGBA8 only | Phase G |
| gpui SVG | no production SVG element/render path | Required | SVG is only tracked as future asset path | Phase C/G |
| gpui animation | deterministic animation clock/tween | Required | upstream animation graph not complete | Phase G |
| gpui::test | no macro-equivalent yet; public `TestContextCapability` exists | Required | internal tests plus public test-context facade, but no upstream-style test macro | Phase G |
| gpui accessibility | snapshot + UIA/AT-SPI facades | Required | production providers missing | Phase G |
| gpui platform services | menu/dialog/chrome skeletons | Required | native services not production-depth | Phase F |
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

Required rows: typed actions, action macro equivalent, key contexts, keymap
grammar, focus/key routing, command palette integration, and disabled-context
behavior.

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
