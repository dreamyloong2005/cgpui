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
| gpui::Application | `Application` plus final prelude-only public surface closure example coverage | Adapted | `include/cgpui/app/application.hpp`, `src/app/application.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `tests/api_parity/application_facade_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp` | Phase B final verification/freeze |
| gpui::App | `App` facade from `AppContext::app()` and `Context<T>::app_context()` app-domain capability, plus `try_open_window(...) -> Result<AppOpenedWindow>` public window-opening result conventions on `App`, `AppContext`, and `WindowRuntime`, `try_create_renderer(...) -> Result<Renderer*>` renderer-creation result conventions on `WindowRuntime`, `try_resize_surface(...) -> Result<void>` renderer-resize result conventions on `WindowRuntime`, and `try_draw_frame() -> Result<void>` renderer frame/redraw result conventions on `WindowRuntime` | Adapted | `include/cgpui/app/app_facade.hpp`, `src/app/app_facade.cpp`, `src/app/app_context_facade.cpp`, `src/ui/runtime_window_results.cpp`, `src/ui/runtime_renderer_results.cpp`, `src/ui/runtime_renderer_resize_results.cpp`, `src/ui/runtime_renderer_frame_results.cpp`, `examples/api_parity/public_api_compatibility/main.cpp`, `examples/api_parity/public_authoring_workflow/main.cpp`, `examples/api_parity/public_context_capabilities/main.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `docs/gpui-public-authoring-vocabulary.md`, `tests/api_parity/app_window_context_test.cpp`, `tests/api_parity/app_context_capability_test.cpp`, `tests/api_parity/public_result_conventions_test.cpp`, `tests/api_parity/renderer_result_conventions_test.cpp`, `tests/api_parity/renderer_resize_result_conventions_test.cpp`, `tests/api_parity/renderer_frame_result_conventions_test.cpp`, `tests/api_parity/public_api_compatibility_examples_test.cpp`, `tests/api_parity/public_api_example_expansion_test.cpp`, `tests/api_parity/public_context_capability_example_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp` | Phase B final verification/freeze |
| gpui::Window | `Window` facade from `WindowRuntimeContext::window()` / `current_window()` plus `WindowContextCapability` from `Context<T>::window_context()` | Adapted | `include/cgpui/app/window.hpp`, `include/cgpui/ui/window_context.hpp`, `src/ui/window.cpp`, `src/ui/window_context.cpp`, `tests/api_parity/app_window_context_test.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/window_context_capability_test.cpp` | Phase B/F window service depth |
| gpui::WindowOptions | `WindowOptions` | Adapted | `include/cgpui/ui/runtime_window_options.hpp` | Phase B/F |
| gpui::Context<T> | `Context<T>` alias with app/window/entity helpers, `Context<T>::app_context()` app-domain capability, `Context<T>::view_context<T>()` view-domain capability, `Context<T>::window_context()` window-domain capability, `ElementContextCapability` from `Context<T>::element_context(ElementId)`, `AsyncContextCapability` from `Context<T>::async_context()`, `TestContextCapability` from `Context<T>::test_context()`, `Context<T>::new_entity<T>(...)` creation, weak entity upgrades, `observe_entity(...)` helpers, entity-to-entity observation helpers, window/view observation helpers, `update_entity(...)` transaction helpers, `invalidate_entity(...)` helpers, entity deletion helpers, deterministic subscription lifetime/unsubscribe behavior, view-removal subscription cleanup, runtime-token cross-context boundaries, prelude-only public API compatibility example coverage, public authoring workflow example coverage, public context capability example coverage, public async/test workflow example coverage, final public surface closure example coverage, and a public authoring vocabulary freeze document | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/async_context.hpp`, `include/cgpui/ui/test_context.hpp`, `include/cgpui/ui/element_context.hpp`, `include/cgpui/ui/view_context.hpp`, `include/cgpui/ui/window_context.hpp`, `include/cgpui/ui/runtime_context.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/runtime_observations.cpp`, `examples/api_parity/public_api_compatibility/main.cpp`, `examples/api_parity/public_authoring_workflow/main.cpp`, `examples/api_parity/public_context_capabilities/main.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `docs/gpui-public-authoring-vocabulary.md`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/app_context_capability_test.cpp`, `tests/api_parity/view_context_capability_test.cpp`, `tests/api_parity/window_context_capability_test.cpp`, `tests/api_parity/element_context_capability_test.cpp`, `tests/api_parity/async_context_capability_test.cpp`, `tests/api_parity/test_context_capability_test.cpp`, `tests/api_parity/entity_lifecycle_creation_test.cpp`, `tests/api_parity/entity_weak_handle_semantics_test.cpp`, `tests/api_parity/entity_observation_test.cpp`, `tests/api_parity/entity_to_entity_observation_test.cpp`, `tests/api_parity/window_view_observation_test.cpp`, `tests/api_parity/subscription_lifetime_test.cpp`, `tests/ui/window_runtime_observation_closure_test.cpp`, `tests/api_parity/entity_transaction_test.cpp`, `tests/api_parity/entity_invalidation_test.cpp`, `tests/api_parity/entity_deletion_test.cpp`, `tests/api_parity/public_api_compatibility_examples_test.cpp`, `tests/api_parity/public_api_example_expansion_test.cpp`, `tests/api_parity/public_context_capability_example_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp` | Phase B final verification/freeze |
| gpui::View<T> | `ViewContextCapability<T>` plus runtime-token-scoped `ViewHandle<T>` and `WeakViewHandle<T>` over `ViewId`, with view observation helpers and view-removal subscription cleanup | Adapted | `include/cgpui/ui/view_context.hpp`, `include/cgpui/ui/view_handle.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/runtime_observations.cpp`, `tests/api_parity/view_handle_spelling_test.cpp`, `tests/api_parity/view_context_capability_test.cpp`, `tests/api_parity/window_view_observation_test.cpp`, `tests/ui/window_runtime_view_handle_token_test.cpp`, `tests/ui/window_runtime_observation_closure_test.cpp` | Phase B view lifecycle depth |
| gpui::AsyncApp | async task handles, cancellation, and foreground/background task Result conventions through `try_spawn_task(...) -> Result<TaskHandle>` and `try_spawn_background_task(...) -> Result<TaskHandle>` on `WindowRuntime`, `WindowRuntimeContext`, and `AsyncContextCapability` | Required | `src/ui/runtime_tasks.cpp`, `src/ui/runtime_task_results.cpp`, `src/ui/runtime_context_scheduling.cpp`, `src/ui/async_context.cpp`, `tests/api_parity/async_spawn_result_conventions_test.cpp`; not full async app context | Phase G |
| gpui::AsyncWindowContext | `AsyncContextCapability` groups defer, timer, animation, foreground task, background task, update-batch scheduling, and async-spawn Result conventions from `Context<T>::async_context()` | Adapted | `include/cgpui/ui/async_context.hpp`, `src/ui/async_context.cpp`, `src/ui/runtime_task_results.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `tests/api_parity/async_context_capability_test.cpp`, `tests/api_parity/async_spawn_result_conventions_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/ui/window_runtime_scheduling_test.cpp` | Phase G full async executor depth |
| gpui::TestAppContext | `TestContextCapability` groups runtime id/view id, live input/event/action observability, invalidation/diagnostics snapshots with entity/window/view observer counts, timer advancement, timer cancellation, task completion draining, run-until-parked timer/async advancement helpers, redraw/frame pump helpers including `try_draw_frame() -> Result<void>`, direct `dispatch_keystroke(...)`, grammar-backed `simulate_keystrokes(...)`, direct pointer move/button/scroll simulation, direct window activation/focus dispatch, element focus/release helpers, direct clipboard read/write helpers, and copy/cut/paste clipboard forwarding from `Context<T>::test_context()` | Adapted | `include/cgpui/ui/test_context.hpp`, `include/cgpui/ui/runtime_diagnostics.hpp`, `src/ui/test_context.cpp`, `src/ui/test_context_scheduling.cpp`, `src/ui/test_context_rendering.cpp`, `src/ui/test_context_keystrokes.cpp`, `src/ui/test_context_pointer.cpp`, `src/ui/test_context_focus.cpp`, `src/ui/test_context_clipboard.cpp`, `src/ui/runtime_clipboard.cpp`, `src/ui/runtime_diagnostic_snapshot.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `tests/api_parity/test_context_capability_test.cpp`, `tests/api_parity/renderer_frame_result_conventions_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/ui/test_context_time_async_test.cpp`, `tests/ui/test_context_frame_pump_test.cpp`, `tests/ui/test_context_keystroke_simulation_test.cpp`, `tests/ui/test_context_pointer_simulation_test.cpp`, `tests/ui/test_context_focus_activation_test.cpp`, `tests/ui/test_context_clipboard_test.cpp`, `tests/ui/window_runtime_scheduling_test.cpp`, `tests/ui/window_runtime_observation_diagnostics_test.cpp` | Phase G fuller simulated input/test macro depth |
| gpui::Entity<T> | `Entity<T>`, `Model<T>`, `EntityHandle<T>`, `WeakEntity<T>`, context-created entity handles, weak upgrade/read semantics, entity observation helpers, entity-to-entity observation helpers, deterministic `Subscription` lifetime/unsubscribe behavior, value-returning update transactions, entity invalidation helpers, entity deletion helpers, and runtime-token cross-context boundaries | Adapted | `include/cgpui/core/entity.hpp`, `include/cgpui/ui/runtime_templates.hpp`, `src/ui/subscription.cpp`, `src/ui/runtime_subscriptions.cpp`, `tests/api_parity/context_capabilities_test.cpp`, `tests/api_parity/entity_lifecycle_creation_test.cpp`, `tests/api_parity/entity_weak_handle_semantics_test.cpp`, `tests/api_parity/entity_observation_test.cpp`, `tests/api_parity/entity_to_entity_observation_test.cpp`, `tests/api_parity/subscription_lifetime_test.cpp`, `tests/api_parity/entity_transaction_test.cpp`, `tests/api_parity/entity_invalidation_test.cpp`, `tests/api_parity/entity_deletion_test.cpp` | Phase B observation depth |
| gpui::Render | `Render<T>` concept over `View::render(Context<T>&)` | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/view.hpp`, `tests/api_parity/context_render_spelling_test.cpp` | Phase B view lifecycle depth |
| gpui::IntoElement | `IntoElement` alias plus `into_element` builders | Adapted | `include/cgpui/ui/render.hpp`, `include/cgpui/ui/element_builder_core.hpp`, `tests/api_parity/context_render_spelling_test.cpp` | Phase C |
| gpui::div | `div()` plus repeated `.child(...)` multi-child retention, `ElementBuilder::children(...)` collection authoring, `StyledElement::children()` child-list ownership, upstream-style flex vocabulary aliases `ElementBuilder::items_start()`, `ElementBuilder::items_center()`, `ElementBuilder::items_end()`, `ElementBuilder::justify_start()`, `ElementBuilder::justify_center()`, `ElementBuilder::justify_end()`, `ElementBuilder::justify_between()`, and `ElementBuilder::flex_1()`, upstream-style sizing/color/border aliases `ElementBuilder::w(...)`, `ElementBuilder::h(...)`, `ElementBuilder::bg(...)`, `ElementBuilder::text_color(...)`, `ElementBuilder::border_1()`, and `ElementBuilder::rounded(...)`, upstream-style overflow/opacity/position aliases `ElementBuilder::overflow(...)`, `ElementBuilder::overflow_hidden()`, `ElementBuilder::overflow_visible()`, `ElementBuilder::opacity(...)`, `ElementBuilder::z_index(...)`, `ElementBuilder::relative()`, `ElementBuilder::top(...)`, `ElementBuilder::right(...)`, `ElementBuilder::bottom(...)`, and `ElementBuilder::left(...)`, upstream-style text aliases `ElementBuilder::text_size(...)` and `ElementBuilder::font_family(...)`, focused shadow vocabulary/storage through `BoxShadow`, `Style::box_shadow`, `StyleOverlay::box_shadow`, `ElementBuilder::shadow(...)`, `ElementBuilder::shadow_sm()`, and `PaintCommandKind::box_shadow` paint-list observability, focused layout constraint vocabulary through `Style::min_size`, `Style::max_size`, `StyleOverlay::min_size`, `StyleOverlay::max_size`, `ElementBuilder::min_size(...)`, `ElementBuilder::max_size(...)`, `ElementBuilder::min_w(...)`, `ElementBuilder::min_h(...)`, `ElementBuilder::max_w(...)`, `ElementBuilder::max_h(...)`, and `StyledElement::layout` min/max constraint merging, focused percentage-like sizing through `PercentageSize`, `Style::percentage_size`, `StyleOverlay::percentage_size`, `ElementBuilder::size_pct(...)`, `ElementBuilder::w_pct(...)`, `ElementBuilder::h_pct(...)`, and `StyledElement::layout` resolution against finite external max constraints with unconstrained fallback, focused margin/padding shorthand vocabulary through `ElementBuilder::p(...)`, `ElementBuilder::px(...)`, `ElementBuilder::py(...)`, `ElementBuilder::pt(...)`, `ElementBuilder::pr(...)`, `ElementBuilder::pb(...)`, `ElementBuilder::pl(...)`, `ElementBuilder::m(...)`, `ElementBuilder::mx(...)`, `ElementBuilder::my(...)`, `ElementBuilder::mt(...)`, `ElementBuilder::mr(...)`, `ElementBuilder::mb(...)`, `ElementBuilder::ml(...)`, and existing `StyledElement::layout` gap composition, focused absolute/fixed positioning through `Position::fixed`, `ElementBuilder::fixed()`, and out-of-flow absolute/fixed child layout in `StyledElement::layout` and `FlexElement::layout`, and focused direct overlay child ordering through `z_order()` for direct `StyledElement`, flex, and vertical-stack child paint, hit-test, and event dispatch via `paint_ordered_children(...)`, `hit_test_ordered_children(...)`, and `event_ordered_children(...)`, and focused nested scroll clipping through PaintList::push_clip(...) effective clip intersection, private paint_clip helpers, hidden-overflow plus scrollable-list clip propagation, and nested clip-stack metadata, and focused style cascade active-state support through `StyleState::active`, `StyleStateFlags::active`, `ElementBuilder::active_style(...)`, `ButtonBuilder::active_style(...)`, and hover/focus/active/disabled resolution order for local and class styles with inline overlays remaining last, and focused class-style reuse depth through `StyleClassRule`, `StyleCascade::set_class_rule(...)`, `StyleCascade::class_rule(...)`, `src/ui/style_cascade.cpp` ownership, and depth-first reused class resolution with cycle protection, and focused theme token fallback through `StyleThemeTokens`, `Style::with_background_color_token(...)`, `StyleOverlay::with_background_color_token(...)`, theme-aware `resolved_style(...)` overloads, `StyledElement::resolved_style(..., Theme)`, `src/ui/style_theme_tokens.cpp` token lookup, `src/ui/style_theme_cascade.cpp` ordering, and missing-token fallback to existing concrete style values, and focused inherited text style through `StyleAuthoredTextFields`, `LabelElement` / `TextElement` effective text style storage, private `text_style_inheritance` helpers, styled/flex/vertical-stack/wrapper inheritance forwarding, and explicit child text-style precedence, and Phase C Step 335 dynamic style invalidation through `WindowRuntime::request_style_state_invalidation(...)`, hover-target transition calls from `src/ui/runtime_event_input.cpp`, keyboard-focus transition calls from `src/ui/runtime_focus.cpp`, and focused `src/ui/runtime_style_invalidation.cpp` ownership | Adapted | `include/cgpui/ui/element_builder_core.hpp`, `include/cgpui/ui/element_style_nodes.hpp`, `include/cgpui/ui/style_tokens.hpp`, `include/cgpui/ui/style_values.hpp`, `include/cgpui/ui/style_box.hpp`, `include/cgpui/ui/style_overlay.hpp`, `include/cgpui/ui/style_state.hpp`, `include/cgpui/ui/style_cascade.hpp`, `include/cgpui/ui/button_builder.hpp`, `src/ui/element_builder_interaction.cpp`, `src/ui/element_builder_layout.cpp`, `src/ui/element_builder_style.cpp`, `src/ui/button_builder.cpp`, `src/ui/element_builder_build.cpp`, `src/ui/element_flex_layout.cpp`, `src/ui/element_style_nodes.cpp`, `src/ui/element_style_paint.cpp`, `src/ui/element_layer_ordering.hpp`, `src/ui/element_layer_ordering.cpp`, `src/ui/element_flex_node.cpp`, `src/ui/element_vertical_stack_node.cpp`, `src/ui/text_style_inheritance.hpp`, `src/ui/text_style_inheritance.cpp`, `src/ui/element_text_paint.cpp`, `src/ui/element_focus_nodes.cpp`, `src/ui/element_pointer_nodes.cpp`, `src/ui/element_button_nodes.cpp`, `src/ui/style_box.cpp`, `src/ui/style_overlay.cpp`, `src/ui/style_cascade.cpp`, `src/ui/style_cascade_overlays.hpp`, `src/ui/style_cascade_overlays.cpp`, `src/ui/style_theme_tokens.hpp`, `src/ui/style_theme_tokens.cpp`, `src/ui/style_theme_cascade.cpp`, `src/ui/paint_shadow.cpp`, `tests/ui/element_test.cpp`, `tests/ui/style_test.cpp`, `tests/architecture/ui_source_structure_test.cpp`, `src/ui/runtime_style_invalidation.cpp`, `tests/ui/window_runtime_input_test.cpp`, `tests/ui/window_runtime_focus_test.cpp`, src/ui/paint.cpp, src/ui/paint_clip.hpp, src/ui/paint_clip.cpp, src/ui/element_scroll_paint.cpp | Phase C Step 336 style cascade depth closeout |
| gpui::prelude | `cgpui/prelude.hpp` plus thin `cgpui/cgpui.hpp` compatibility aggregate, prelude-only public API compatibility examples, and a public authoring vocabulary freeze document | Adapted | `include/cgpui/prelude.hpp`, `include/cgpui/cgpui.hpp`, `examples/api_parity/public_api_compatibility/main.cpp`, `examples/api_parity/public_authoring_workflow/main.cpp`, `examples/api_parity/public_context_capabilities/main.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `docs/gpui-public-authoring-vocabulary.md`, `tests/api_parity/public_authoring_surface_test.cpp`, `tests/api_parity/public_api_compatibility_examples_test.cpp`, `tests/api_parity/public_api_example_expansion_test.cpp`, `tests/api_parity/public_context_capability_example_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`, `tests/header_cleanliness/prelude_header_cleanliness.cpp` | Phase B final verification/freeze |
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
`examples/hello_world.rs`. Step 307 adds
`examples/api_parity/public_api_compatibility/main.cpp` as a prelude-only
compile target guarded by
`tests/api_parity/public_api_compatibility_examples_test.cpp`; it fails if the
example reaches into `WindowRuntime` internals or private headers. Step 308 adds
`docs/gpui-public-authoring-vocabulary.md`, guarded by
`tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`, to freeze the
public authoring names before Phase C. Step 309 adds
`examples/api_parity/public_authoring_workflow/main.cpp`, guarded by
`tests/api_parity/public_api_example_expansion_test.cpp`, to keep a second
prelude-only workflow example compiling against the frozen vocabulary without
touching private runtime headers. Step 310 adds
`examples/api_parity/public_context_capabilities/main.cpp`, guarded by
`tests/api_parity/public_context_capability_example_test.cpp`, to keep app,
view, window, and element context capability authoring examples prelude-only
before Phase C. Step 311 adds
`examples/api_parity/public_async_test_workflow/main.cpp`, guarded by
`tests/api_parity/public_async_test_workflow_example_test.cpp`, to keep
async/test capability workflow examples prelude-only and free of runtime
internals before Phase C. Step 312 adds
`examples/api_parity/public_phase_b_surface_closure/main.cpp`, guarded by
`tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, as the
final Phase B public surface closure example before the verification/freeze
band. Steps 313-318 add the final Phase B public vocabulary completion audit,
guarded by `tests/api_parity/public_phase_b_completion_audit_test.cpp`, so the
roadmap, vocabulary document, ledger, public examples, and xmake targets stay
aligned before Phase C. The tracked
upstream example inventory includes:

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
