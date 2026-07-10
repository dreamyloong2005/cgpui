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

## Scope Guard

The active track follows the post-Phase-C user scope decision. Deferred by
user decision: game-engine-specific integration, engine runtime embedding,
shared engine renderer/resource/asset/command systems, game editor/runtime UI
depth, Android, iOS, and X11. Required active-track work remains:
C++23-native public APIs without Rust FFI, Vulkan-first production renderer
depth, declarative widgets, reactive state/subscription depth, low-allocation
runtime structures, static and dynamic widget support, editor/AI-IDE-class
text and tool UI primitives, Win32 and Wayland production platform behavior,
and later macOS Cocoa + Metal parity.

Zero-cost abstraction is a hard active-track constraint: Static fast paths come
first, dynamic escape hatches must stay explicit, compact retained records or
command buffers should be the hot-path lowering target, and future slices must
avoid hidden allocation, broad type erasure, avoidable virtual dispatch, avoid
per-frame tree-wide scans, repeated string lookups, and avoidable
`std::function`/heap churn. The entry gate is locked by
`pre_phase_d_entry_gate_test/default`,
`phase_c_final_ledger_audit_test/default`,
`static_render_runtime_test/default`, `ui_source_structure_test/default`, and
`gpui_parity_ledger_test/default`.

Verification cadence is also part of the scope guard: every slice keeps
Windows focused tests, touched ledger/JSON checks, and `git diff --check`;
WSL focused tests are required for shared public headers,
templates/constexpr/concepts, `xmake.lua`, shared UI runtime, renderer,
Vulkan, platform, Wayland, or cross-platform structure changes. WSL full debug
is batched at Phase closeout, user-requested verification points, or
intentional renderer/platform milestones instead of after every small step.
WSL xmake global/package/build/cache output must stay on D-drive project
paths, with transient temp such as `/dev/shm/cgpui`, so CGPUI work does not
consume C: drive space again.

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
| gpui::div | `div()` plus repeated `.child(...)` multi-child retention, `ElementBuilder::children(...)` collection authoring, `StyledElement::children()` child-list ownership, upstream-style flex vocabulary aliases `ElementBuilder::items_start()`, `ElementBuilder::items_center()`, `ElementBuilder::items_end()`, `ElementBuilder::justify_start()`, `ElementBuilder::justify_center()`, `ElementBuilder::justify_end()`, `ElementBuilder::justify_between()`, and `ElementBuilder::flex_1()`, upstream-style sizing/color/border aliases `ElementBuilder::w(...)`, `ElementBuilder::h(...)`, `ElementBuilder::bg(...)`, `ElementBuilder::text_color(...)`, `ElementBuilder::border_1()`, and `ElementBuilder::rounded(...)`, upstream-style overflow/opacity/position aliases `ElementBuilder::overflow(...)`, `ElementBuilder::overflow_hidden()`, `ElementBuilder::overflow_visible()`, `ElementBuilder::opacity(...)`, `ElementBuilder::z_index(...)`, `ElementBuilder::relative()`, `ElementBuilder::top(...)`, `ElementBuilder::right(...)`, `ElementBuilder::bottom(...)`, and `ElementBuilder::left(...)`, upstream-style text aliases `ElementBuilder::text_size(...)` and `ElementBuilder::font_family(...)`, focused shadow vocabulary/storage through `BoxShadow`, `Style::box_shadow`, `StyleOverlay::box_shadow`, `ElementBuilder::shadow(...)`, `ElementBuilder::shadow_sm()`, and `PaintCommandKind::box_shadow` paint-list observability, focused layout constraint vocabulary through `Style::min_size`, `Style::max_size`, `StyleOverlay::min_size`, `StyleOverlay::max_size`, `ElementBuilder::min_size(...)`, `ElementBuilder::max_size(...)`, `ElementBuilder::min_w(...)`, `ElementBuilder::min_h(...)`, `ElementBuilder::max_w(...)`, `ElementBuilder::max_h(...)`, and `StyledElement::layout` min/max constraint merging, focused percentage-like sizing through `PercentageSize`, `Style::percentage_size`, `StyleOverlay::percentage_size`, `ElementBuilder::size_pct(...)`, `ElementBuilder::w_pct(...)`, `ElementBuilder::h_pct(...)`, and `StyledElement::layout` resolution against finite external max constraints with unconstrained fallback, focused margin/padding shorthand vocabulary through `ElementBuilder::p(...)`, `ElementBuilder::px(...)`, `ElementBuilder::py(...)`, `ElementBuilder::pt(...)`, `ElementBuilder::pr(...)`, `ElementBuilder::pb(...)`, `ElementBuilder::pl(...)`, `ElementBuilder::m(...)`, `ElementBuilder::mx(...)`, `ElementBuilder::my(...)`, `ElementBuilder::mt(...)`, `ElementBuilder::mr(...)`, `ElementBuilder::mb(...)`, `ElementBuilder::ml(...)`, and existing `StyledElement::layout` gap composition, focused absolute/fixed positioning through `Position::fixed`, `ElementBuilder::fixed()`, and out-of-flow absolute/fixed child layout in `StyledElement::layout` and `FlexElement::layout`, and focused direct overlay child ordering through `z_order()` for direct `StyledElement`, flex, and vertical-stack child paint, hit-test, and event dispatch via `paint_ordered_children(...)`, `hit_test_ordered_children(...)`, and `event_ordered_children(...)`, and focused nested scroll clipping through PaintList::push_clip(...) effective clip intersection, private paint_clip helpers, hidden-overflow plus scrollable-list clip propagation, and nested clip-stack metadata, and focused style cascade active-state support through `StyleState::active`, `StyleStateFlags::active`, `ElementBuilder::active_style(...)`, `ButtonBuilder::active_style(...)`, and hover/focus/active/disabled resolution order for local and class styles with inline overlays remaining last, and focused class-style reuse depth through `StyleClassRule`, `StyleCascade::set_class_rule(...)`, `StyleCascade::class_rule(...)`, `src/ui/style_cascade.cpp` ownership, and depth-first reused class resolution with cycle protection, and focused theme token fallback through `StyleThemeTokens`, `Style::with_background_color_token(...)`, `StyleOverlay::with_background_color_token(...)`, theme-aware `resolved_style(...)` overloads, `StyledElement::resolved_style(..., Theme)`, `src/ui/style_theme_tokens.cpp` token lookup, `src/ui/style_theme_cascade.cpp` ordering, and missing-token fallback to existing concrete style values, and focused inherited text style through `StyleAuthoredTextFields`, `LabelElement` / `TextElement` effective text style storage, private `text_style_inheritance` helpers, styled/flex/vertical-stack/wrapper inheritance forwarding, and explicit child text-style precedence, and Phase C Step 335 dynamic style invalidation through `WindowRuntime::request_style_state_invalidation(...)`, hover-target transition calls from `src/ui/runtime_event_input.cpp`, keyboard-focus transition calls from `src/ui/runtime_focus.cpp`, and focused `src/ui/runtime_style_invalidation.cpp` ownership, and Phase C Step 336 style cascade depth closeout through `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp` guarding the Steps 331-335 style-cascade evidence before Phase C Step 337 focusable/interactable semantics, and Phase C Step 337 pointer-active input semantics through `ViewInputState::active_element_id`, `WindowRuntime::update_active_state_for_event(...)`, left-button press/release `active_element_id` tracking in `src/ui/runtime_active_state.cpp`, active style-state invalidation via `request_style_state_invalidation(...)`, and focused `tests/ui/window_runtime_input_test.cpp` coverage, and Phase C Step 338 tab-order/focus-ring metadata through `FocusMetadata`, `FocusRingVisibility`, `ElementBuilder::tab_index(...)`, `ElementBuilder::focus_ring(...)`, `AccessibilityNode::tab_index` / `focus_ring` reporting, and `WindowRuntime::ordered_focusable_element_ids()` traversal ordering in `src/ui/runtime_focus_order.cpp` so positive tab indices precede default tree order and negative tab indices are skipped by Tab traversal, with focused `tests/ui/element_test.cpp` and `tests/ui/window_runtime_focus_test.cpp` coverage, and Phase C Step 339 click/drag gesture synthesis through `ElementGestureKind::click`, `ElementEventContext::gesture`, `ViewInputState::pointer_down_element_id`, `ViewInputState::clicked_element_id`, `ViewInputState::dragging_element_id`, `ViewInputState::dragging`, focused `src/ui/runtime_gesture_synthesis.cpp` ownership, `WindowRuntime::update_active_state_for_event(...)` gesture-state updates, `dispatch_synthesized_click_event(...)` after raw routed release handlers, `ClickElement` / `ButtonElement` synthesized-click gating, and focused `tests/ui/window_runtime_input_test.cpp` coverage including `test_runtime_synthesizes_click_on_release_over_pressed_element` and `test_runtime_suppresses_click_after_drag_gesture`, plus `tests/ui/element_test.cpp` coverage, and Phase C Step 340 keyboard activation semantics through `should_dispatch_synthesized_keyboard_activation_event(...)`, focused Enter/Space synthesized-click activation, raw key handler first-refusal suppression, `KeyElement` synthesized-click delegation, and focused `tests/ui/window_runtime_focus_test.cpp` coverage including `test_runtime_enter_space_activate_focused_button` and `test_runtime_raw_key_handler_controls_keyboard_activation`, and Phase C Step 341 disabled interaction semantics through `WindowRuntime::refresh_disabled_interaction_state()`, focused `src/ui/runtime_disabled_interaction.cpp` ownership, stale hover/active/focus/pointer-capture/pointer-down/click/drag state cleanup for disabled or missing elements, default cursor restoration, and focused `tests/ui/window_runtime_input_test.cpp` coverage including `test_runtime_clears_disabled_interaction_state`, and Phase C Step 342 focusable/interactable band closeout through `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp` guarding the Steps 337-341 focusable/interactable evidence before Phase C Step 343 built-in widget expansion, and Phase C Step 343 built-in widget expansion through ButtonBuilder::label(...), focused src/ui/widgets/button_builder.cpp, src/ui/widgets/label_builder.cpp, and src/ui/widgets/text_input_builder.cpp ownership, xmake src/ui/widgets/*.cpp compilation, tests/ui/builtin_widget_test.cpp, and tests/architecture/widget_source_structure_test.cpp | Adapted | `include/cgpui/ui/element_builder_core.hpp`, `include/cgpui/ui/element_style_nodes.hpp`, `include/cgpui/ui/style_tokens.hpp`, `include/cgpui/ui/style_values.hpp`, `include/cgpui/ui/style_box.hpp`, `include/cgpui/ui/style_overlay.hpp`, `include/cgpui/ui/style_state.hpp`, `include/cgpui/ui/style_cascade.hpp`, `include/cgpui/ui/button_builder.hpp`, `src/ui/element_builder_interaction.cpp`, `src/ui/element_builder_layout.cpp`, `src/ui/element_builder_style.cpp`, `src/ui/widgets/button_builder.cpp`, `src/ui/element_builder_build.cpp`, `src/ui/element_flex_layout.cpp`, `src/ui/element_style_nodes.cpp`, `src/ui/element_style_paint.cpp`, `src/ui/element_layer_ordering.hpp`, `src/ui/element_layer_ordering.cpp`, `src/ui/element_flex_node.cpp`, `src/ui/element_vertical_stack_node.cpp`, `src/ui/text_style_inheritance.hpp`, `src/ui/text_style_inheritance.cpp`, `src/ui/element_text_paint.cpp`, `src/ui/element_focus_nodes.cpp`, `src/ui/element_pointer_nodes.cpp`, `src/ui/element_button_nodes.cpp`, `src/ui/style_box.cpp`, `src/ui/style_overlay.cpp`, `src/ui/style_cascade.cpp`, `src/ui/style_cascade_overlays.hpp`, `src/ui/style_cascade_overlays.cpp`, `src/ui/style_theme_tokens.hpp`, `src/ui/style_theme_tokens.cpp`, `src/ui/style_theme_cascade.cpp`, `src/ui/paint_shadow.cpp`, `tests/ui/element_test.cpp`, `tests/ui/style_test.cpp`, `tests/architecture/ui_source_structure_test.cpp`, `src/ui/runtime_style_invalidation.cpp`, `src/ui/runtime_active_state.cpp`, `src/ui/runtime_gesture_synthesis.hpp`, `src/ui/runtime_gesture_synthesis.cpp`, `src/ui/runtime_event_route_dispatch.cpp`, `src/ui/runtime_disabled_interaction.cpp`, `include/cgpui/ui/runtime_input_state.hpp`, `include/cgpui/ui/focus_metadata.hpp`, `src/ui/element_focus_metadata.cpp`, `src/ui/runtime_focus_order.cpp`, `tests/ui/window_runtime_input_test.cpp`, `tests/ui/window_runtime_focus_test.cpp`, `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`, `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`, src/ui/paint.cpp, src/ui/paint_clip.hpp, src/ui/paint_clip.cpp, src/ui/element_scroll_paint.cpp | Phase C element/style/widget band closed |
| gpui::prelude | `cgpui/prelude.hpp` plus thin `cgpui/cgpui.hpp` compatibility aggregate, prelude-only public API compatibility examples, and a public authoring vocabulary freeze document | Adapted | `include/cgpui/prelude.hpp`, `include/cgpui/cgpui.hpp`, `examples/api_parity/public_api_compatibility/main.cpp`, `examples/api_parity/public_authoring_workflow/main.cpp`, `examples/api_parity/public_context_capabilities/main.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `docs/gpui-public-authoring-vocabulary.md`, `tests/api_parity/public_authoring_surface_test.cpp`, `tests/api_parity/public_api_compatibility_examples_test.cpp`, `tests/api_parity/public_api_example_expansion_test.cpp`, `tests/api_parity/public_context_capability_example_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`, `tests/header_cleanliness/prelude_header_cleanliness.cpp` | Phase B final verification/freeze |
| gpui::actions! / action macro | `Action<T>` typed action concept, `action_name<T>()` / `action_name_v<T>`, typed register/dispatch overloads over the existing string action registry, action registration metadata for general/app/window/view/focused-element scopes, typed action command-palette metadata binding, action enablement metadata that suppresses disabled action dispatch, action bubbling through focused routes, key binding grammar parsing for `Context<T>::bind_key("ctrl-shift-s", "action")`, platform modifier semantics for `secondary-*` and `platform-*` bindings, keymap context filtering for app/window/view/focused-element bindings, partial key sequence matching for multi-chord bindings, disabled key scope filtering, and command palette key integration that parses `CommandPaletteEntry::key_binding` and installs derived or explicit `KeyBindingContext` bindings | Required | `include/cgpui/ui/action.hpp`, `include/cgpui/ui/key_binding.hpp`, `include/cgpui/ui/runtime_action_templates.hpp`, `include/cgpui/ui/runtime_action_enablement_templates.hpp`, `include/cgpui/ui/runtime_command_palette_templates.hpp`, `include/cgpui/ui/runtime_actions.hpp`, `src/ui/runtime_action_dispatch.cpp`, `src/ui/runtime_action_registration.cpp`, `src/ui/runtime_action_metadata.cpp`, `src/ui/runtime_command_palette_keys.cpp`, `src/ui/runtime_key_binding_grammar.cpp`, `src/ui/runtime_key_binding_modifiers.cpp`, `src/ui/runtime_key_binding_contexts.cpp`, `src/ui/runtime_key_binding_sequences.cpp`, `tests/api_parity/typed_action_surface_test.cpp`, `tests/ui/typed_action_dispatch_test.cpp`, `tests/ui/action_scope_metadata_test.cpp`, `tests/ui/typed_action_command_metadata_test.cpp`, `tests/ui/action_enablement_metadata_test.cpp`, `tests/ui/action_bubbling_test.cpp`, `tests/ui/key_binding_grammar_test.cpp`, `tests/ui/key_binding_platform_modifier_test.cpp`, `tests/ui/keymap_context_test.cpp`, `tests/ui/key_binding_partial_match_test.cpp`, `tests/ui/key_binding_disabled_scope_test.cpp`, `tests/ui/command_palette_key_integration_test.cpp`; action macro payloads still missing | Phase B action metadata/key dispatch depth |
| gpui key_context | `KeyBindingContext` for app/window/view/focused-element key binding activation | Required | context-aware key bindings exist without the full upstream key-context tree API | Phase B |
| gpui keymap dispatch | `KeyBinding`, `KeyBindingChord`, `KeyBindingContext`, `parse_key_binding(...)`, `parse_key_binding(..., DesktopPlatformTarget)`, `Context<T>::bind_key("ctrl-shift-s", "action")`, `Context<T>::bind_key("ctrl-k ctrl-s", "action")`, `Context<T>::bind_key(..., KeyBindingContext)`, command-palette-owned key binding metadata, and action registry dispatch | Required | deterministic key binding plus GPUI-style chord grammar, platform modifier semantics, active context filtering, pending partial sequence matching, disabled key scope filtering, and command palette key integration exist; `secondary-*` maps to Ctrl on Windows/Linux and Super on macOS, while `platform-*` / `cmd-*` / `win-*` map to the platform key. Action macro payloads remain missing | Phase B |
| gpui element styling | `Style`, `StyleOverlay`, builders | Required | many primitives exist, tailwind-style vocabulary incomplete | Phase C |
| gpui uniform_list | `scrollable_list` plus `UniformListVisibleRange`, `UniformListItemIdentity`, `UniformListItemMeasurement`, `UniformListItemMeasurementResult`, `UniformListRecyclingWindow`, `UniformListSelectionSource`, `UniformListSelectionDirection`, `UniformListSelection`, `UniformListSelectionState`, `UniformListLayoutSnapshot`, `UniformListScrollAnchor`, `UniformListItemMeasurementCache`, `calculate_uniform_list_visible_range(...)`, `measure_uniform_list_items(...)`, `calculate_uniform_list_recycling_window(...)`, `select_uniform_list_item_at_point(...)`, `move_uniform_list_selection(...)`, `capture_uniform_list_scroll_anchor(...)`, `apply_uniform_list_scroll_anchor(...)`, `ScrollableListElement::layout_snapshot()`, `ScrollableListElement::measurement_cache()`, and `ScrollableListElement::selection()` | Adapted | stable item identity, visible-range snapshots, keyed scroll anchoring, keyed item measurement cache stats, retained/recycled large-list paint windows, pointer hit selection, keyboard movement selection, snapshot selected flags, and Step 354 audit closeout exist | Phase C uniform-list band closed |
| gpui text system | `TextModel`, shaping, wrap, glyph records | Required | deterministic text depth exists; Phase D Steps 379-430 add `TextShapingBackend` selection, HarfBuzz capability reporting, explicit fallback reason metadata, glyph ids, glyph positioning offsets, shaping direction/script/language metadata, internal shaping dispatch/fallback backend boundaries, a guarded `src/ui/text_shaping_harfbuzz.cpp` insertion point, backend capability snapshots on selections/runs, wrapped glyph-id propagation, a text-shaping readiness audit, a focused `src/ui/text_font.cpp` implementation boundary for font database/discovery helpers, platform font discovery result/diagnostics boundaries through `PlatformFontDiscoveryResult`, `PlatformApplication::discover_font_discovery()`, `src/platform/platform_font_discovery.cpp`, `src/platform/win32/win32_font_discovery.cpp`, and `src/platform/linux/wayland_font_discovery.cpp`, real Win32 DirectWrite system font-family enumeration with native-available diagnostics and deterministic fallback on DirectWrite failure, a guarded Linux fontconfig backend insertion point in `src/platform/linux/wayland_fontconfig_discovery.cpp` with Wayland fallback/native diagnostics coverage plus system-optional fontconfig package wiring in `xmake.lua` through `add_requires("fontconfig", {system = true, optional = true})`, `has_package("fontconfig")`, `add_packages("fontconfig")`, and `CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND`, coverage-aware fallback records through `FontUnicodeRange` and `FontDatabase::resolve_chain_for_codepoint(...)`, explicit fallback-chain shaping via `shape_text(..., FontFallbackChain, ...)` and `TextShapeRun::font_fallback_faces`, glyph-level fallback face selection through `TextGlyphRun::font_fallback_face_index`, the Step 394 font fallback band audit, contiguous fallback font spans through `TextFontFallbackRun` and `TextShapeRun::font_runs`, explicit missing-glyph diagnostics through `TextMissingGlyphDiagnostic` and `TextShapeRun::missing_glyphs`, emoji-plane color glyph planning through `TextColorGlyphPlan` and `TextShapeRun::color_glyphs`, emoji presentation selector planning for `U+FE0F` through `is_emoji_presentation_selector(...)`, `codepoint_accepts_emoji_presentation(...)`, and `append_emoji_presentation_color_glyph_plan(...)`, selector-span metadata through `TextColorGlyphPlan::has_emoji_presentation_selector`, `TextColorGlyphPlan::emoji_presentation_selector_byte_offset`, `TextColorGlyphPlan::emoji_presentation_selector_byte_length`, and `mark_emoji_presentation_selector_span(...)`, emoji ZWJ joiner diagnostic suppression for `U+200D` through `is_emoji_sequence_joiner(...)`, `next_utf8_codepoint_after(...)`, and `is_emoji_sequence_joiner_between_emoji(...)`, deterministic script-run metadata through `TextScriptRun`, `TextShapeRun::script_runs`, `classify_text_shaping_script(...)`, and `append_script_run_span(...)`, the Step 402 fallback metadata band audit, grapheme column measurement metadata through `TextGraphemeColumn`, `TextMeasurement::grapheme_columns`, `build_text_grapheme_columns(...)`, and `text_grapheme_column_includes_codepoint(...)`, grapheme-column-aware soft wrap records through `TextWrapLine::column_start`, `TextWrapLine::column_end`, `text_wrap_line_for_column_range(...)`, and `wrap_text_measurement(...)`, hard-wrap records through `TextWrapBreakKind`, `TextWrapLine::break_kind`, and `text_wrap_column_is_hard_break(...)`, deterministic bidirectional planning metadata through `TextBidiRun`, `TextMeasurement::base_direction`, `TextMeasurement::bidi_runs`, `TextWrapLine::bidi_run_start`, `TextWrapLine::bidi_run_end`, `TextWrapLayout::base_direction`, `build_text_bidi_runs(...)`, and `classify_text_bidi_direction(...)`, deterministic line metrics/line-box metadata through `TextLineMetrics`, `TextMeasurement::line_metrics`, `TextWrapLine::metrics`, `text_line_metrics_for_shape_run(...)`, and `src/ui/text_line_metrics.cpp`, explicit paragraph layout/cache records through `TextParagraphLayout`, `TextParagraphLayoutResult`, `TextParagraphLayoutCache`, `layout_text_paragraph(...)`, and `src/ui/text_paragraph_layout.cpp`, CRLF hard-wrap normalization through `text_measurement_is_crlf_pair(...)`, CR/LF hard-break recognition in `text_wrap_column_is_hard_break(...)`, wrapped paint ranges that skip CRLF glyphs, the Step 410 text measurement/wrapping band audit, text selection drag records through `TextSelectionDragDirection`, `TextSelectionDrag`, `text_selection_drag_from_offsets(...)`, and `text_selection_drag_from_points(...)`, word-selection range helpers through `TextModel::word_selection_range_at(...)`, line-selection range helpers through `TextModel::line_selection_range_at(...)`, and multi-click selection granularity records through `PointerButton::click_count`, `TextSelectionGranularity`, and `text_selection_granularity_for_click_count(...)`, and runtime double-click word selection through `WindowRuntime::apply_text_pointer_selection(...)` consuming `PointerButton::click_count == 2` plus `TextModel::word_selection_range_at(...)`, runtime triple-click line selection through `PointerButton::click_count >= 3` plus `TextModel::line_selection_range_at(...)`, multiline selection/caret paint geometry through focused `src/ui/text_selection_paint_geometry.cpp` / `.hpp` helpers, per-line `TextSelectionPaint` commands, measured hard-wrap caret placement, `tests/ui/element_test.cpp`, and `tests/architecture/ui_source_structure_test.cpp`, preferred-column vertical navigation through `TextModel::preferred_line_column_` and `preferred_line_column_for_vertical_navigation(...)`, explicit scroll-to-caret primitives through `ScrollModel::scroll_rect_into_view(...)`, `TextElement::caret_rect(...)`, `TextElement::scroll_caret_into_view(...)`, and shared `text_caret_rect(...)`, runtime route scrolling for `ScrollableListElement`, focused text clipboard forwarding through `paste_clipboard_text()`, `copy_selection_to_clipboard()`, and `cut_selection_to_clipboard()`, and the `tests/api_parity/phase_d_selection_caret_audit_test.cpp` closeout, adjacent typing history coalescing through `TextInsertHistoryPolicy::merge_adjacent_typing`, explicit paste separation through `TextInsertHistoryPolicy::separate_edit`, composition-history grouping through `TextInsertHistoryPolicy::composition_commit`, `TextModel::begin_composition_history_group(...)`, `TextModel::delete_surrounding_text(...)`, and `tests/ui/text_model_test.cpp`, undo-manager integration points through `TextEditHistoryStatus`, `TextModel::edit_history_status()`, `TextModel::edit_history_clean()`, `TextModel::mark_edit_history_clean()`, clean history markers, edit history revisions, redo invalidation diagnostics through `TextEditHistoryRedoInvalidationReason`, `TextEditHistoryRedoInvalidation`, `TextEditHistoryStatus::last_redo_invalidation`, and `TextModel::invalidate_redo_history(...)`, edit transaction diagnostics through `TextEditHistoryTransactionKind`, `TextEditHistoryTransactionDiagnostic`, `TextEditHistoryStatus::last_transaction`, and `TextModel::record_edit_history_transaction(...)`, the Step 424 edit-history band audit through `tests/api_parity/phase_d_edit_history_audit_test.cpp`, and edit-history grouping ownership in `src/ui/text_model_history.cpp`, focused IME surrounding-text placement through `ImeTextInputPlacement::surrounding_text`, `ImeTextInputPlacement::selection_anchor`, `ImeTextInputPlacement::content_hint`, `ImeTextInputPlacement::content_purpose`, runtime focused `TextModel` placement filling in `src/ui/runtime_platform_services.cpp`, Wayland text-input v3 `set_surrounding_text` / `set_content_type` submissions in `src/platform/linux/wayland_text_input_requests.cpp`, Win32 IMM placement state preservation in `src/platform/win32/win32_window_ime.cpp`, Wayland `done(serial)` propagation through `ImeComposition::serial`, `ImeDeleteSurroundingText::serial`, `src/platform/linux/wayland_text_input_events.cpp`, `src/platform/linux/wayland_window_bridge.cpp`, and `src/platform/linux/wayland_window_text_events.cpp`, Wayland preedit cursor metadata through `ImeComposition::preedit_cursor_begin`, `ImeComposition::preedit_cursor_end`, `WaylandTextInput::PendingPreedit`, `wayland_window_text_input_preedit(...)`, fixed-capacity preedit style metadata through `ImePreeditStyleSpan`, `kImePreeditStyleSpanCapacity`, `append_ime_preedit_style(...)`, `append_ime_default_preedit_style(...)`, `src/core/event_text.cpp`, Wayland default underline styling in `src/platform/linux/wayland_window_text_events.cpp`, and `tests/platform/wayland_keyboard_test.cpp`, `tests/platform/win32_text_input_test.cpp`, `tests/ui/window_runtime_text_test.cpp`, and `tests/architecture/platform_source_structure_test.cpp`; rich-text run core through `RichTextSpan`, `RichTextRun`, `RichTextAttributes`, `RichTextDecoration`, `RichTextLinkId`, and `build_rich_text_runs(...)`, byte hit metadata through `RichTextRunHit`, `RichTextLinkHit`, `rich_text_run_at_byte_offset(...)`, and `rich_text_link_at_byte_offset(...)`, point hit metadata through `RichTextRunPointHit`, `RichTextLinkPointHit`, `rich_text_run_at_point(...)`, and `rich_text_link_at_point(...)`, inline image metadata through `RichTextInlineImageSpan`, `RichTextInlineImageRun`, and `build_rich_text_inline_image_runs(...)`, syntax-theme metadata through `RichTextSyntaxRole`, `RichTextSyntaxToken`, `RichTextSyntaxTheme`, `rich_text_syntax_attributes_for_role(...)`, and `build_rich_text_syntax_spans(...)`, rich-text paint metadata transport through `PaintList::fill_rich_text(...)`, `TextPaint::rich_text_runs`, `TextPaint::rich_text_inline_images`, `TextDraw::rich_text_runs`, `TextDraw::rich_text_inline_images`, `src/ui/paint_rich_text.cpp`, and `src/ui/render_view_commands.cpp`, link activation metadata through `RichTextLinkActivation`, `rich_text_pointer_button_can_activate_link(...)`, and `rich_text_link_activation_at_point(...)`, and the Step 442 rich-text metadata band audit through `tests/api_parity/phase_d_rich_text_audit_test.cpp`; guarded HarfBuzz backend now shapes through hb_shape with file-backed font faces when available and deterministic fallback on shaping failure through src/ui/text_shaping_harfbuzz.cpp and tests/api_parity/phase_d_harfbuzz_backend_audit_test.cpp; DirectWrite font-file extraction remains later work, along with real color glyph rendering, native ZWJ ligature shaping depth, full Unicode script data, full Unicode bidirectional shaping/reordering, paragraph shaping, cache eviction policy, platform-derived font metrics, Unicode line-break classes, dependency-backed native Linux fontconfig/FreeType font enumeration, Win32 TSF message depth, richer Wayland serial policy, platform-specific preedit styling, production candidate UI policy, runtime rich-text element dispatch, actual link command handling, inline image drawing/loading, syntax parsing/editor token source integration, actual multi-color glyph painting, and Phase D final text examples/verification closeout through tests/api_parity/phase_d_final_closeout_test.cpp | Phase D |
| gpui image assets | image paint/upload skeleton plus public source/registry authoring | Required | in-memory RGBA8 and deterministic registry only; production loading/upload remains later | Phase C/G |
| gpui SVG | public SVG source and registry authoring | Required | SVG source strings are preserved for authoring; production decoding/rendering remains later | Phase C/G |
| gpui animation | deterministic animation clock/tween | Required | upstream animation graph not complete | Phase G |
| gpui::test | no macro-equivalent yet; public `TestContextCapability` exists with keystroke, pointer, window activation/focus, element focus, clipboard, run-until-parked timer/async, and redraw/frame pump simulation helpers | Required | internal tests plus public test-context facade, `simulate_keystrokes(...)`, `dispatch_pointer_*`, `dispatch_window_*`, `focus(...)`, `release_focus(...)`, `write_to_clipboard(...)`, `read_from_clipboard(...)`, copy/cut/paste helpers, `run_until_parked()`, `advance_time_until_parked(...)`, `request_redraw()`, `try_draw_frame()`, and `draw_frame()`, but no upstream-style test macro | Phase G |
| gpui accessibility | snapshot + UIA/AT-SPI facades | Required | production providers missing | Phase G |
| gpui platform services | menu/dialog/chrome skeletons plus `try_install_native_menu(...) -> Result<NativeMenuInstallation>` and `try_show_native_file_dialog(...) -> Result<NativeFileDialogResult>` on `WindowRuntime`, `AppContext`, and `ViewContext`/`WindowRuntimeContext` | Required | `src/ui/runtime_platform_service_results.cpp`, `src/ui/runtime_platform_services.cpp`, `src/ui/app_context_services.cpp`, `src/ui/runtime_context_platform.cpp`, `tests/api_parity/platform_service_result_conventions_test.cpp`; native services not production-depth | Phase B result conventions, then Phase F production services |
| gpui Windows backend | Win32 + Vulkan | Required | active target | Phase F |
| gpui_platform wayland feature | Wayland + Vulkan | Required | active Linux target | Phase F |
| gpui_platform x11 feature | Deferred | Deferred | not active by user decision | Optional Phase I |
| gpui macOS backend | Cocoa + Metal | Deferred | later platform track | Phase H |
| gpui wasm backend | Non-goal | Non-goal | not requested for CGPUI desktop target | no active phase |

## Phase D Text Evidence Addendum

- Step 431 adds explicit IME candidate placement metadata through
  `ImeTextInputPlacement::candidate_rect`. Runtime focused-text placement fills
  it from `ImeCandidateRect`, Wayland text-input v3 prefers it for cursor
  rectangles, and Win32 IMM applies it to `CANDIDATEFORM` while keeping `rect`
  for `COMPOSITIONFORM`.
- Evidence: `include/cgpui/core/window.hpp`,
  `src/ui/runtime_platform_services.cpp`,
  `src/platform/linux/wayland_text_input_requests.cpp`,
  `src/platform/win32/win32_window_ime.cpp`,
  `tests/ui/window_runtime_text_test.cpp`,
  `tests/platform/wayland_keyboard_test.cpp`,
  `tests/platform/win32_text_input_test.cpp`, and
  `tests/architecture/platform_source_structure_test.cpp`.
- Remaining gap: production candidate UI policy remains later Phase D work.
- Step 432 adds Win32 IMM composition/result string ingestion. Evidence:
  `src/platform/win32/win32_window_proc_lifecycle.cpp` keeps
  `WM_IME_COMPOSITION` as a thin dispatch,
  `src/platform/win32/win32_window_ime.cpp` reads `GCS_COMPSTR` and
  `GCS_RESULTSTR` with `ImmGetCompositionStringW`, emits update/commit
  `ImeComposition` events, and emits cancel on `WM_IME_ENDCOMPOSITION`.
  `tests/platform/win32_text_input_test.cpp` covers the stable cancel message,
  while `tests/architecture/platform_source_structure_test.cpp` freezes the
  `GCS_*` production parsing boundary.
- Remaining gap: deeper Win32 TSF integration and richer platform-specific
  composition styling remain later Phase D work.
- Step 433 adds Wayland text-input v3 stale serial policy. Evidence:
  `src/platform/linux/wayland_text_input_internal.hpp` stores
  `last_done_serial_`, `src/platform/linux/wayland_text_input_events.cpp`
  rejects non-increasing `done(serial)` values and clears pending events,
  `src/platform/linux/wayland_text_input_core.cpp` resets serial/pending state
  with the text-input object, and `tests/platform/wayland_keyboard_test.cpp`
  uses request-specific serials from `tests/platform/wayland_test_compositor.*`
  to prove a stale commit is ignored while later increasing serials are
  accepted.
- Remaining gap: broader Wayland IME policy beyond stale serial rejection
  remains later Phase D work.
- Step 434 closes the active-target IME platform band through
  `tests/api_parity/phase_d_ime_platform_audit_test.cpp`, freezing the
  Step 427-433 evidence for surrounding text/content hints, Wayland serial
  propagation, stale serial policy based on the last accepted `done(serial)`,
  preedit cursor/style metadata, candidate-placement metadata, Win32 IMM
  composition/result string ingestion, and the structure tests that keep those
  boundaries focused.
- Remaining gap: production candidate UI policy, deeper Win32 TSF integration,
  and richer platform-specific composition styling remain later Phase D work.
- Step 435 starts the rich-text band with a focused run-core leaf:
  `include/cgpui/ui/text_rich_text.hpp` and `src/ui/text_rich_text.cpp` define
  `RichTextSpan`, `RichTextRun`, `RichTextRunBuildScratch`,
  `RichTextAttributes`, decoration metadata, numeric `RichTextLinkId`, and
  `build_rich_text_runs(...)` overloads that normalize caller-provided spans
  into clipped, merged, coalesced byte runs with caller-owned output/scratch
  reuse.
  `tests/ui/rich_text_run_test.cpp` covers plain-text fallback, overlapping
  foreground/background/decorations/link attributes, out-of-range span
  clipping, empty span dropping, and adjacent equal-run coalescing.
- Remaining gap: inline images, rich-text paint integration, link activation,
  syntax-theme wiring, and run-aware hit testing remain later Phase D work.
- Step 436 adds byte-offset run/link hit metadata on the same rich-text leaf:
  `RichTextRunHit`, `RichTextLinkHit`,
  `rich_text_run_at_byte_offset(...)`, and
  `rich_text_link_at_byte_offset(...)` return small optional records over
  half-open `RichTextRun` byte ranges. `tests/ui/rich_text_run_test.cpp`
  verifies run boundary selection, no-link misses, link id propagation, and
  end-exclusive behavior.
- Remaining gap: point-based rich-text hit testing, click activation wiring,
  inline images, and rich-text paint integration remain later Phase D work.
- Step 437 adds point-based rich-text hit metadata through the focused
  `text_rich_text_hit_testing` leaf. `RichTextRunPointHit`,
  `RichTextLinkPointHit`, `rich_text_run_at_point(...)`, and
  `rich_text_link_at_point(...)` compose `hit_test_text_position(...)` byte
  offsets with the Step 436 run/link lookup helpers without runtime state,
  renderer state, or string link targets. `tests/ui/rich_text_run_test.cpp`
  verifies point-to-run mapping, link id propagation, and end-exclusive misses
  past the measured text.
- Remaining gap: click activation wiring, inline images, syntax-theme
  integration, and rich-text paint integration remain later Phase D work.
- Step 438 adds inline image metadata through the focused
  `text_rich_text_inline_image` leaf. `RichTextInlineImageSpan`,
  `RichTextInlineImageRun`, and `build_rich_text_inline_image_runs(...)`
  expose numeric `ImageAssetId` anchors with byte-span clipping, zero-length
  insertion anchors, deterministic sorting, baseline offset, and caller-owned
  output storage. `tests/ui/rich_text_run_test.cpp` verifies clipping,
  filtering, sorting, and baseline metadata without adding image loading or
  paint behavior.
- Remaining gap: inline image paint integration, image asset loading/
  registration policy inside rich text, click activation wiring, syntax-theme
  integration, and rich-text paint integration remain later Phase D work.
- Step 439 adds syntax-theme metadata through the focused
  `text_rich_text_syntax` leaf. `RichTextSyntaxRole`,
  `RichTextSyntaxToken`, `RichTextSyntaxTheme`,
  `rich_text_syntax_attributes_for_role(...)`, and
  `build_rich_text_syntax_spans(...)` convert caller-provided token ranges
  into deterministic `RichTextSpan` records without maps, reflection, parser
  ownership, runtime state, or renderer state. `tests/ui/rich_text_run_test.cpp`
  verifies clipping, filtering, sorting, and composition with
  `build_rich_text_runs(...)`.
- Remaining gap: syntax parsing, editor token source integration, rich-text
  paint integration, click activation wiring, and inline image paint
  integration remain later Phase D work.
- Step 440 adds rich-text paint metadata integration without changing actual
  drawing semantics. `PaintList::fill_rich_text(...)` lives in focused
  `src/ui/paint_rich_text.cpp`, `TextPaint` and `TextDraw` carry
  `rich_text_runs` plus `rich_text_inline_images`, and
  `src/ui/render_view_commands.cpp` preserves those records into render-frame
  text draws. `tests/ui/render_view_test.cpp` verifies run/link and inline-image
  metadata retention, and `tests/architecture/ui_source_structure_test.cpp`
  keeps the entry out of broad paint/runtime files.
- Remaining gap: actual multi-color glyph painting, inline image drawing/
  loading, click activation wiring, syntax parsing, and editor token source
  integration remain later Phase D work.
- Step 441 adds focused rich-text link activation metadata. The public leaf
  `include/cgpui/ui/text_rich_text_activation.hpp` and implementation
  `src/ui/text_rich_text_activation.cpp` expose `RichTextLinkActivation`,
  `rich_text_pointer_button_can_activate_link(...)`, and
  `rich_text_link_activation_at_point(...)`, converting primary single-button
  release point hits into numeric link activation records by composing the
  existing rich-text point-hit helper. `tests/ui/rich_text_run_test.cpp`
  verifies activation and press/right-button/multi-click/outside misses, while
  `tests/architecture/ui_source_structure_test.cpp` keeps the logic out of
  runtime, renderer, and string link targets.
- Remaining gap: runtime rich-text element dispatch, actual link command
  handling, inline image drawing/loading, syntax parsing, and editor token
  source integration remain later Phase D work.
- Step 442 closes the rich-text metadata band through
  `tests/api_parity/phase_d_rich_text_audit_test.cpp`, freezing Steps 435-441
  evidence for rich-text run normalization, byte and point hit metadata, inline
  image metadata, syntax-theme metadata, paint metadata transport, link
  activation metadata, focused source ownership, and explicit remaining gaps.
- Remaining gap: runtime rich-text element dispatch, actual link command
  handling, inline image drawing/loading, syntax parsing/editor token source
  integration, and actual multi-color glyph painting remain later Phase D work.
- Phase D Step 443 text input public examples start this slice. Step 443 starts the text-input public examples band with
  `examples/api_parity/public_text_input_examples/main.cpp` and
  `api_parity_public_text_input_examples`, demonstrating TextModel public editing,
  selection ranges, edit-history status, text-input builder use, IME placement and composition metadata, delete-surrounding metadata, command
  palette entries, and key bindings through `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_text_input_public_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs.
- Remaining gap: text wrapper examples, richer official input workflows,
  rich-text examples, and final Phase D verification remain later Phase D work.
- Phase D Step 444 text wrapper public examples extend this slice. Step 444 extends the text-input examples band with
  `examples/api_parity/public_text_wrapper_examples/main.cpp` and
  `api_parity_public_text_wrapper_examples`, demonstrating label and TextElement wrappers,
  TextModel-backed public text elements, measurement/wrapping/glyph paint metadata,
  TextMeasurementCache use, layout sizing, and accessibility text through
  `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_text_wrapper_public_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs.
- Remaining gap: richer official input workflows, rich-text examples, and final
  Phase D verification remain later Phase D work.
- Phase D Step 445 text input workflow public example extends this slice.
  Step 445 extends the text-input examples band with
  `examples/api_parity/public_text_input_workflow/main.cpp` and
  `api_parity_public_text_input_workflow`, demonstrating undo/redo and redo invalidation,
  edit-history clean markers, line/word navigation and composition cancellation,
  selection text, backspace/delete-forward, TextEditAction dispatch, and history
  diagnostics through `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_text_input_workflow_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs.
- Remaining gap: rich-text examples, final examples closeout, and final Phase D
  verification remain later Phase D work.
- Phase D Step 446 rich text public example extends this slice. Step 446 extends the text-input examples band with
  `examples/api_parity/public_rich_text_examples/main.cpp` and
  `api_parity_public_rich_text_examples`, demonstrating rich-text run, syntax, inline-image, hit, activation, and paint metadata
  through `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_rich_text_public_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs.
- Remaining gap: renderer glyph coloring and inline image drawing remain later work, alongside
  final examples closeout and final Phase D verification.
- Phase D guarded HarfBuzz backend now shapes through hb_shape in
  `src/ui/text_shaping_harfbuzz.cpp` when
  `CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND` is enabled, using file-backed font faces when available and deterministic fallback on shaping failure.
  `tests/api_parity/phase_d_harfbuzz_backend_audit_test.cpp` keeps the guarded
  backend from regressing into a `#error` insertion point.
- Remaining gap: DirectWrite font-file extraction remains later work, along
  with native ZWJ ligature shaping depth, full Unicode script data,
  bidirectional shaping, and paragraph shaping.
- Phase D font discovery now has system-optional fontconfig package wiring in
  `xmake.lua`, enabling `CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND` for the
  Wayland platform target when the system `fontconfig` package is available
  while preserving deterministic fallback when it is absent.
- Remaining gap: FreeType metrics extraction and richer per-face coverage
  remain later work.
- Phase D text examples closeout lands in
  `tests/api_parity/phase_d_text_examples_closeout_test.cpp`. Step 450 closes the text examples band by freezing the
  public-only prelude examples: `api_parity_public_text_input_examples`,
  `api_parity_public_text_wrapper_examples`,
  `api_parity_public_text_input_workflow`, and
  `api_parity_public_rich_text_examples`.
- Remaining gap: renderer glyph coloring and inline image drawing remain later work.
- Phase D final closeout lands in
  `tests/api_parity/phase_d_final_closeout_test.cpp`. Windows full debug suite
  passed 139/139, and WSL Arch Linux full debug suite passed 136/136 using
  D-drive WSL build/cache output plus `/dev/shm/cgpui` transient temp.
- Remaining Phase D text gaps stay explicit for later work: DirectWrite
  font-file extraction, real color glyph rendering, native ZWJ ligature shaping
  depth, full Unicode script data, full Unicode bidirectional
  shaping/reordering, paragraph shaping, cache eviction policy,
  platform-derived font metrics, Unicode line-break classes, FreeType metrics
  and richer per-face coverage, platform-specific preedit styling, production
  candidate UI policy, runtime rich-text element dispatch, actual link command
  handling, inline image drawing/loading, syntax parsing/editor token source
  integration, and actual multi-color glyph painting.
- Phase E Step 459 Vulkan glyph atlas production starts the renderer
  production path through the focused public leaf
  `include/cgpui/renderer/glyph_atlas_production.hpp` and the focused Vulkan
  source `src/renderer/vulkan/vulkan_glyph_atlas_production.cpp`.
  `GlyphAtlasProductionResourceState` and
  `vulkan_plan_glyph_atlas_production_resources(...)` convert glyph atlas
  upload batches into alpha8 atlas page image readiness, memory allocation and bind readiness, image-view and sampler readiness, and dirty upload command path readiness while preserving handle-free public records. descriptor set binding remains Step 460, along with private Vulkan renderer-state handle
  ownership and command-buffer recording.
- Phase E Step 460 Vulkan glyph atlas descriptor binding is implemented through
  private handle-bearing renderer modules. `VulkanRendererState` now owns
  `VulkanGlyphAtlasResources`; the descriptor module creates the combined image
  sampler layout/pool in `vulkan_glyph_atlas_descriptors.cpp`, allocates
  per-page descriptor sets, and calls
  `vkUpdateDescriptorSets`; the image module creates R8_UNORM images,
  device-local memory, image views, and the shared sampler; and the resource
  module reconciles/destroys page lifetimes after the in-flight fence completes.
  Public atlas leaves remain free of `Vk*` handles. A real text-frame smoke test
  exercises atlas page and descriptor creation.
- Phase E Step 461 Vulkan glyph atlas dirty uploads are implemented through
  private `VulkanGlyphAtlasUploadResources` ownership.
  `vulkan_glyph_atlas_staging.cpp` creates host-visible/coherent transfer-source
  buffers, repacks each dirty glyph at a 4-byte-aligned buffer offset, maps only
  the dirty alpha payload, and retires staging after the in-flight fence.
  `vulkan_glyph_atlas_upload_recording.cpp` records image barriers,
  `vkCmdCopyBufferToImage`, and the final shader-readable layout. Atlas planner
  and image-layout state commit only after `vkQueueSubmit` succeeds.
- Phase E Step 462 records only the acquired command buffer and exercises
  multi-frame incremental atlas reuse. Presentation order is now
  wait/prepare/acquire/record/reset-fence/submit; record failure after acquire
  routes through `recover_after_failed_record(...)` and blocks presentation
  until swapchain recreation. The Win32 smoke retains frame-outlives-renderer
  behavior and adds `ab -> ab -> abc` frames covering initial upload, no-dirty
  reuse, and incremental shader-readable upload.
- Phase E Step 463 covers multi-page atlas allocation and cross-page uploads.
  A 9-glyph synthetic workload fills three atlas pages with independent upload
  batches, staging buffers, descriptor sets, and buffer-to-image copy lists.
  The private `vulkan_glyph_atlas_descriptor_capacity` constant drives both
  descriptor-pool sizing and a preflight that rejects an over-capacity plan
  before existing resources change. The Win32 Vulkan smoke submits the same
  three-page workload.
- Phase E Step 464 adds private `VulkanGlyphAtlasDrawBinding` records that map
  each text draw and atlas page to the renderer-owned descriptor set.
  `vulkan_resolve_glyph_atlas_draw_bindings(...)` rejects missing descriptors;
  command recording validates the resolved bindings against live resources in
  the live command buffer before the render pass. Public renderer headers remain
  free of `Vk*` handles.
- Phase E Step 465 adds private `VulkanGlyphAtlasDrawData` ownership. Flat
  textured glyph quads retain draw order, while `first_quad_index` and glyph
  counts describe contiguous page runs for the descriptor bindings. Live
  command recording rejects out-of-range bindings or quad/page mismatches.
- Phase E Step 466 glyph atlas integration closeout is frozen by
  `tests/api_parity/phase_e_glyph_atlas_integration_closeout_test.cpp`. The
  audit covers Steps 459-465 evidence for alpha8 page resources,
  descriptor capacity, dirty uploads, acquired command buffer recording, three
  atlas pages, private `VulkanGlyphAtlasDrawBinding` and
  `VulkanGlyphAtlasDrawData` ownership, and contiguous page runs.
- Remaining Phase E glyph-atlas gap: Step 467 text shader pipeline starts the
  real shader, vertex-buffer, pipeline, and textured draw-call work.
- Handoff: Phase E Step 467 text shader pipeline.
- Phase E Step 467 establishes the private fixed-function text pipeline state.
  `VulkanTextVertex` carries position, atlas UV, and color attributes;
  `vulkan_text_pipeline_state.cpp` freezes triangle-list input, no culling or
  depth, one-sample rasterization, dynamic viewport/scissor, and
  straight alpha blending.
  Step 468 adds validated embedded shader modules before pipeline-handle
  ownership and draw recording.
- Handoff: Phase E Step 468 embedded text shader modules.
- Phase E Step 468 adds reviewable vertex/fragment GLSL and validated
  embedded SPIR-V. `vulkan_text_vertex_shader_spirv` and its fragment companion
  feed focused `vulkan_create_text_shader_modules`, destroy, and stage helpers;
  both optimized binaries pass `spirv-val` on Windows and WSL without a host
  GLSL-compiler requirement.
- Handoff: Phase E Step 469 text pipeline layout and graphics pipeline handles.
- Phase E Step 469 adds private swapchain-owned
  `VulkanTextPipelineResources`. `vulkan_create_text_pipeline_resources`
  creates the glyph-atlas descriptor-compatible pipeline layout and graphics
  pipeline with an 8-byte vertex push constant, destroys transient shader
  modules after creation, and follows swapchain create/install/resize/destroy.
- Handoff: Phase E Step 470 text vertex-buffer upload resources.
- Phase E Step 470 adds `VulkanTextVertexBufferResources`, deterministic six-
  vertex expansion per `TexturedGlyphQuad`, and
  `vulkan_upload_text_vertex_buffer`. Frame preparation rebuilds one host-
  visible/coherent vertex buffer only after the in-flight fence completes, and
  existing contiguous page-run indices remain valid.
- Handoff: Phase E Step 471 descriptor-bound textured glyph draw recording.
- Phase E Step 471 adds pre-render-pass text draw planning and focused
  `vulkan_record_text_draws`. The recorder binds pipeline, viewport/scissor,
  vertex buffer, framebuffer-size push constants, each atlas page descriptor,
  and one `vkCmdDraw` per contiguous page run; real Win32 text frames submit.
- Handoff: Phase E Step 472 explicit subpixel positioning policy.
- Phase E Step 472 adds `VulkanTextPositioningPolicy`. Production defaults to
  `preserve_subpixel`; the optional device-pixel snap mode rounds quad outer
  edges before vertex expansion, leaves atlas UVs unchanged, and is recorded in
  `VulkanTextVertexBufferResources`.
- Handoff: Phase E Step 473 glyph coverage gamma and alpha handling.
- Phase E Step 473 adds `VulkanTextCoveragePolicy` over linear `R8_UNORM`
  atlas samples. The default transfer is linear, the reference path supports a
  deterministic power transfer, and `straight_color_coverage_alpha` preserves
  straight RGB while resolved coverage multiplies output alpha. The validated
  embedded fragment shader mirrors that contract.
- Handoff: Phase E Step 474 text pipeline integration closeout.
- Phase E Step 474 text pipeline integration closeout is audit-only through
  `tests/api_parity/phase_e_text_pipeline_integration_closeout_test.cpp`. It
  freezes Steps 467-473, including descriptor-bound textured glyph draws,
  `preserve_subpixel`, and `straight_color_coverage_alpha`, then hands Phase E
  to Step 475 rounded rectangle geometry without adding renderer behavior.
- Phase E Step 475 adds `VulkanRoundedRectGeometry` in focused private source
  files. It creates contiguous vertex/index buffers and stable draw ranges for
  all valid `RoundedRectDraw` records, pre-reserves once, and skips empty
  rectangles.
- Handoff: Phase E Step 476 Vulkan rounded rectangle buffer uploads.
- Phase E Step 476 adds `VulkanRoundedRectBufferResources` and
  `vulkan_upload_rounded_rect_buffers`. Fence-safe frame preparation uploads
  paired host-visible/coherent vertex and index buffers, retains geometry draw
  ranges, and cleans them up through `VulkanRendererState`.
- Handoff: Phase E Step 477 rounded rectangle shader pipeline.
- Phase E Step 477 adds `VulkanRoundedRectPipelineResources` with a dedicated
  position/color vertex ABI, framebuffer-size push constants, dynamic
  viewport/scissor state, and straight-alpha blending. Reviewable GLSL is
  compiled into embedded rounded rectangle SPIR-V, while transient shader
  modules and swapchain-owned layout/pipeline handles keep lifetime explicit.
- Handoff: Phase E Step 478 indexed rounded rectangle draw recording.
- Phase E Step 478 adds zero-allocation draw-range validation and
  `vulkan_record_rounded_rect_draws`. The focused recorder binds the rounded
  rectangle pipeline, vertex/index buffers, viewport, scissor, and framebuffer
  push constants before issuing `vkCmdDrawIndexed` for each retained range.
  Existing solid clear recording now lives in its own focused module so the
  general frame recorder remains below its structure limit.
- Handoff: Phase E Step 479 rounded rectangle anti-aliasing strategy.
- Phase E Step 479 adds `VulkanRoundedRectAntialiasingPolicy` with a default
  one-device-pixel coverage fringe. Rounded rectangle geometry emits
  full-coverage inner and zero-coverage outer rings, the vertex ABI carries
  coverage, and validated embedded shaders modulate straight alpha without
  adding MSAA state or descriptors.
- Handoff: Phase E Step 480 border radius clipping and normalization.
- Phase E Step 480 adds `VulkanRoundedRectRadiiResolution`. Negative values
  clamp to zero and one CSS-style normalization scale guarantees
  adjacent corner sums fit the rectangle width and height. Geometry consumes the
  resolved radii for both its inner contour and coverage fringe.
- Handoff: Phase E Step 481 border stroke geometry.
- Phase E Step 481 adds `VulkanRoundedRectStrokeResolution`. A uniform border
  width clamps to half the smaller rectangle dimension, then normalized outer
  radii produce an inset stroke contour and fitted inner radii. Stroked geometry
  emits distinct fill-color and border-color inner rings, a full-coverage border
  outer ring, and a zero-coverage fringe without enlarging the fill-only path.
- Handoff: Phase E Step 482 fill variants and rounded-rectangle band closeout.
- Phase E Step 482 closes rounded rectangle fill variants. `PaintList` exposes
  fill-only, fill-plus-stroke, and stroke-only commands; uniform styled borders
  become one rounded draw, nonuniform borders retain edge fallback, and render
  submission plus tessellation diagnostics preserve the fill/stroke fields.
  Vulkan stroke-only geometry emits only the inset border, outer border, and
  coverage fringe rings, skipping invisible fill vertices and indices.
- Handoff: Phase E Step 483 clip-stack command recording.
- Phase E Step 483 adds `vulkan_resolve_clip_stack_scissor` as a focused private
  Vulkan boundary. It intersects retained clip-stack entries, the current clip,
  and scalar fallback without allocation, clamps the result to the framebuffer,
  and returns visibility plus an integer `VkRect2D`.
- Solid clears now reuse the shared resolution. Rounded geometry ranges and
  text page-run commands retain one effective rectangle, then install a
  per-draw dynamic scissor and skip empty clip results before issuing GPU work.
- Handoff: Phase E Step 484 nested opacity command recording.
- Phase E Step 484 adds `vulkan_apply_composed_opacity` as an allocation-free
  private policy. Because UI metadata already carries precomposed opacity, the
  renderer reads the final current/scalar value once, normalizes it to `[0, 1]`,
  and multiplies only the authored alpha channel.
- Solid clear attachments, rounded fill/stroke geometry, and production text
  atlas quads now bake the resolved alpha before GPU work. Rounded/text pipelines
  blend; solid clear writes do not blend. Diagnostic textured glyph quads keep
  authored color plus composition metadata.
- Handoff: Phase E Step 485 blend-capable solid rectangle geometry.
- Phase E Step 485 adds `vulkan_build_solid_rect_geometry`. Valid solid draws
  emit compact four-vertex/six-index quads with Step 483 effective clips and
  Step 484 alpha already baked into vertex colors.
- Separate fence-safe solid buffers reuse the rounded pipeline and indexed draw
  recorder. The frame path no longer calls `vkCmdClearAttachments` for authored
  rectangles, so blend-capable solid geometry now participates in straight-
  alpha composition without a temporary adapted-draw vector.
- Handoff: Phase E Step 486 composed affine transform application.
- Phase E Step 486 adds `vulkan_apply_composed_transform` in a focused private
  Vulkan module. Solid, rounded, and text production vertices consume the
  precomposed affine transform once; non-finite matrices resolve to identity.
- Text positioning policy resolves before all four glyph quad corners are
  transformed. Existing clip rectangles remain framebuffer-space in this
  slice rather than silently changing clip semantics.
- Handoff: Phase E Step 487 transform/clip interaction policy.
- Phase E Step 487 adds `transform_clip_rect_to_framebuffer_aabb` in a focused
  private paint leaf. Each local clip becomes a conservative
  push-time framebuffer AABB using the precomposed transform active at that scope before
  it intersects already captured parent clips.
- Invalid transforms retain the authored clip. A clip pushed before a later
  transform remains framebuffer-space, and Vulkan continues to resolve the
  captured rectangles through the existing allocation-free dynamic scissor.
- Handoff: Phase E Step 488 stable renderer command ordering.
- Phase E Step 488 adds compact frame order entries and a zero-allocation
  `VulkanFrameDrawOrderCursor`. `VulkanFrame` records only primitive kind plus
  per-type command index while existing vectors retain command ownership.
- Actual command recording now preserves stable authored interleaving across
  solid, rounded, and text draws. Missing geometry is skipped, text atlas page
  runs expand at their source position, and pipeline/buffer state rebinds only
  when the resolved resource kind changes.
- Handoff: Phase E Step 489 explicit z/layer command ordering.
- Phase E Step 489 freezes explicit z/layer command ordering end to end.
  `Element::z_order()` retains explicit nonzero z-index precedence over layer,
  while tree and direct-child traversal preserve stable UI paint order for equal
  values.
- PaintList commands pass through `submit_paint_command_to_frame(...)` in that
  authored order. `VulkanFrame` records compact per-type indices and
  `VulkanFrameDrawOrderCursor` resolves the mixed solid, rounded, and text
  sequence without a renderer-side z sort.
- `vulkan_layer_ordering_test` locks negative layers, explicit z-index override,
  equal-order stable siblings, UI submission, and Vulkan cursor output.
- Handoff: Phase E Step 490 clip/composition integration closeout.
- Phase E Step 490 clip/composition integration closeout is audit-only in
  `tests/api_parity/phase_e_clip_composition_integration_closeout_test.cpp`.
  It freezes Steps 475-489 across contiguous rounded geometry, paired resources,
  indexed drawing, coverage/radius/stroke/fill policies, allocation-free
  dynamic scissor, single-application composed opacity, blend-capable solid
  geometry, affine transforms, push-time framebuffer AABB capture, stable
  authored interleaving, and explicit z/layer command ordering.
- Rectangular clip stacks remain covered by per-draw Vulkan scissor resolution.
  A future non-rectangular clip still requires a stencil or shader-mask path.
- Handoff: Phase E Step 491 image texture resources.
- Phase E Step 491 adds focused private `VulkanImageTextureResources` ownership.
  Descriptor-keyed resources create and retain `VkImage`, device-local memory,
  and `VkImageView` handles for `VK_FORMAT_R8G8B8A8_UNORM`, providing
  device-local RGBA image/view ownership without sampler or descriptor state.
- Request preflight rejects invalid descriptors and conflicting allocation
  identities for one asset id. Reused dimensions/format preserve the image while
  refreshing stride/byte-size metadata; absent images remain cached for the later
  lifetime policy.
- `VulkanRendererState` reconciles image resources after the frame fence and
  destroys them during device teardown. Resources remain in
  `VK_IMAGE_LAYOUT_UNDEFINED` because pixel transport, staging buffers, copy
  commands, and layout transitions belong to Step 492.
- `vulkan_frame_lifetime_test` submits a valid image descriptor through a live
  Vulkan frame and presents successfully, exercising real image/memory/view
  allocation without claiming upload or sampling.
- Handoff: Phase E Step 492 image upload staging.
- Phase E Step 492 adds explicit bitmap transport through
  `RenderFrame::upload_image`. `VulkanFrame` converts caller-owned assets into
  frame-owned upload batches, replacing duplicate uploads for one asset id.
- Focused image upload modules validate RGBA descriptors, stride, and byte
  ranges; allocate host-visible RGBA staging buffers; map and copy pixel bytes;
  and record buffer-to-image copy commands with transfer-destination and
  shader-readable layout barriers before the render pass.
- Image texture layout state commits only after successful queue submission,
  and the prior frame's staging buffers retire after the in-flight fence. A live
  Vulkan frame exercises actual staging allocation, upload recording, and
  teardown without claiming image sampling or draw recording.
- Handoff: Phase E Step 493 image sampler modes and descriptor binding.
- Phase E Step 493 adds the focused public `ImageSamplingMode` leaf. `ImageDraw`
  defaults to linear sampling while retaining an explicit nearest mode for
  pixel-aligned content.
- Vulkan image texture resources now own persistent nearest/linear samplers, one
  combined-image-sampler descriptor layout and pool, and descriptor set binding
  for both modes on every cached texture. The fixed capacity is 256 textures and
  512 descriptor sets.
- Resource binding allocates and writes both sets after image/view creation;
  resource teardown frees the sets before destroying the view/image/memory, then
  state teardown destroys samplers, pool, and layout.
- Handoff: Phase E Step 494 image graphics pipeline and draw recording.
- Phase E Step 494 adds a swapchain-owned image graphics pipeline with dedicated
  full-RGBA shaders, frame-owned transformed quad vertices, normalized pixel
  source rectangles, sampling descriptor selection, and stable authored image
  interleaving through the existing frame draw-order cursor.
- `ImageBuilder::sampling(...)` now propagates through `ImageElement`,
  `ImagePaint`, and `ImageDraw`; command recording performs actual Vulkan image draw recording
  with descriptor binding, clip-stack scissor resolution, and
  `vkCmdDraw`.
- The draw planner records only textures that are already shader-readable or
  have a pending upload in the same command buffer; first-frame descriptor-only
  resources are skipped instead of sampling `VK_IMAGE_LAYOUT_UNDEFINED`.
- Handoff: Step 495 image tint and composition opacity.
- Phase E Step 495 adds multiplicative image tint with an explicit RGBA image
  vertex attribute. The vertex shader forwards the tint and the fragment shader
  multiplies it by the sampled full-RGBA texel; an absent tint is opaque white.
- Composition opacity reuses `vulkan_apply_composed_opacity(...)` for single application
  to tint alpha, preserving authored RGB and texture alpha through
  the existing straight-alpha blend pipeline.
- Handoff: Step 496 image cache lifetime.
- Phase E Step 496 adds a frame-generation image cache. Each Vulkan texture
  records its last-used frame, active draw/upload requests are touched before
  fence-safe eviction, and the default policy retains 120 idle frames.
- Generation wrap rebases live resources. A real upload -> idle -> draw frame
  sequence proves the shader-readable texture survives without re-upload.
- Handoff: Step 497 image invalidation.
- Phase E Step 497 adds `RenderFrame::invalidate_image(...)` with frame-owned
  deduplicated image invalidations. Vulkan applies them after the in-flight
  fence and staging cleanup for fence-safe resource destruction.
- Invalidation precedes cache touch and resource ensure. Same-frame uploads
  recreate and refresh the texture; invalidated draw-only textures remain
  undefined and are skipped by the existing shader-readiness guard.
- Handoff: Step 498 image integration closeout.
- Phase E Step 498 image integration closeout is audit-only in
  `tests/api_parity/phase_e_image_integration_closeout_test.cpp`. It freezes
  Steps 491-497 across persistent image texture resources,
  explicit bitmap upload transport, nearest/linear sampling descriptors,
  stable authored image interleaving, multiplicative image tint, the
  frame-generation image cache, and deduplicated image invalidations without
  adding renderer behavior.
- Handoff: Step 499 SVG rendering strategy.
- Phase E Step 499 adds `SvgRasterizationRequest` and `SvgRasterizationPlan` in
  a focused public renderer leaf with non-template planning in
  `src/renderer/svg_rasterization.cpp`. It produces an
  explicit RGBA8 output plan from logical size and DPI scale while enforcing a
  bounded raster byte budget before backend allocation.
- Handoff: Step 500 LunaSVG raster backend.
- Phase E Step 500 adds `SvgRasterizationResult` and `rasterize_svg(...)` in
  `src/renderer/svg_rasterization_lunasvg.cpp` using LunaSVG v3.5.0. The backend
  parses length-aware markup, renders the Step 499 plan dimensions, converts to
  plain RGBA pixel output, validates bitmap stride, and returns an `ImageAsset`.
- Handoff: Step 501 SVG raster cache.
- Phase E Step 501 adds `SvgRasterCache` with explicit hit/miss statistics and
  cache identity over asset id, logical size, scale, and SVG source. Identical
  requests return a cache-owned raster result without copying RGBA pixels, and
  failed rasterizations are not cached.
- Handoff: Step 502 SVG viewport scaling.
- Phase E Step 502 adds `SvgViewportScalingPlan`: an optional raster viewport falls back to intrinsic logical size, produces ceil-rounded viewport device pixels, records effective x/y raster scales, and participates in normalized SVG cache identity.
- Handoff: Step 503 SVG recolor/tint.
- Phase E Step 503 adds `SvgRasterColorizationPlan` with a validated RGBA current color normalized to RGBA8 for raster planning and cache identity. The LunaSVG currentColor recolor sets the document-root CSS color, while existing image color stays draw-time multiplicative tint.
- Handoff: Step 504 SVG image upload integration.
- Phase E Step 504 adds `SvgImageUploadResult` and consumes a cache-owned raster ImageAsset through RenderFrame::upload_image(...) integration. Cache hits resubmit the ready asset for the current frame, while failed rasterization skips upload.
- Handoff: Step 505 SVG public example.
- Phase E Step 505 adds the prelude-only `public_svg_raster_upload` executable. It turns a registered SVG source into a viewport-aware raster request, proves cache miss/hit behavior, and performs cached upload and image draw through public APIs.
- Handoff: Step 506 SVG integration closeout.
- Phase E Step 506 SVG integration closeout is audit-only in `tests/api_parity/phase_e_svg_integration_closeout_test.cpp`. It freezes Steps 499-505 across bounded RGBA raster planning, the LunaSVG raster backend, cache-owned raster results, viewport-aware scaling, currentColor recolor, RenderFrame image upload, and the prelude-only public example. Step 507 batching and frame scheduling is next.
- Phase E Step 507 adds `VulkanFrameGeometryBufferResources` and reusable host-visible vertex/index buffers for text, image, solid, and rounded geometry. Empty frames keep retained capacity, matching-capacity uploads remap the existing allocation, and growth replaces buffers geometrically after the single in-flight fence. Step 508 command reuse is next.
- Phase E Step 508 adds per-swapchain-image recorded command reuse guarded by an exact semantic command signature. Matching upload-free frames resubmit the recorded buffer without reset or recording, while pending uploads force recording and invalidate reuse state. Step 509 pipeline-switch batching is next.
- Phase E Step 509 adds pipeline-switch batching with authored draw order preserved. Adjacent solid and rounded rectangle draws reuse the shared rounded-rectangle pipeline while rebinding only their distinct geometry buffers. Step 510 resource barriers are next.
- Phase E Step 510 adds ordered upload barrier waves with batched transfer and shader-read transitions. Unique glyph-atlas or image-texture targets share two barrier calls around their copies, while duplicate image targets start a new wave with shader-read old-layout continuity. Step 511 swapchain recovery is next.
- Phase E Step 511 adds automatic swapchain recreation from acquire/present result plans. The out-of-date results return a retryable frame error after recreating, suboptimal frames recreate after submission, and presentation remains unblocked after successful recovery. Step 512 present pacing is next.
- Phase E Step 512 adds a focused Vulkan present pacing policy: MAILBOX with FIFO fallback, saturation-safe swapchain image depth, and one CPU frame in flight via shared fence/acquire waits. Step 513 next-frame scheduling is next.
- Phase E Step 513 adds next-frame scheduling: render-time invalidation survives frame completion and repeated requests are coalesced into exactly one platform redraw. Step 514 batching and scheduling closeout is next.
- Phase E Step 514 closes the batching and frame scheduling integration closeout for Steps 507-513, freezing reusable geometry buffers through next-frame scheduling. Step 515 renderer diagnostics is next.
- Phase E Step 515 adds `RendererFrameWork` and `RendererFrameDiagnostics` to compare planned and submitted renderer work across command and batch counts with saturation-safe pending and unexpected counts. Step 516 upload-byte accounting is next.
- Phase E Step 516 adds `RendererUploadByteCounts` for glyph and image upload payload bytes, saturation-safe upload-byte accounting, and pending/unexpected upload-byte comparison. Step 517 draw-count accounting is next.
- Phase E Step 517 adds `RendererDrawCounts` for primitive-aware GPU draw counts, saturation-safe draw-count accounting, and pending/unexpected draw comparison. Step 518 dropped-resource accounting is next.
- Phase E Step 518 adds `RendererDroppedResourceDiagnostics` for ordered planned-resource submission gaps, classifying unsupported and missing submission resources while preserving command/resource identity. Step 519 frame-timing diagnostics is next.
- Phase E Step 519 adds `RendererFrameTimingDiagnostics` for explicit CPU frame-stage nanoseconds, saturation-safe timing accumulation, and frame-budget comparison. Step 520 live Vulkan diagnostic snapshots are next.

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

## Phase C Widget Evidence

- Phase C Step 344 checkbox/radio/switch widgets: `ToggleBuilder`,
  `checkbox(...)`, `radio(...)`, `toggle_switch(...)`,
  `ToggleControlElement`, and `ToggleControlKind` expose focused built-in
  toggle controls after the Step 343 button/label/text-input boundary.
- State and accessibility evidence: `AccessibilityRole::checkbox`,
  `AccessibilityRole::radio`, `AccessibilityRole::switch_control`,
  `AccessibilityNode::value`, and `PlatformAccessibilityRole::checkbox` /
  `radio` / `switch_control` carry checked/selected/on state as
  checked/unchecked or on/off values.
- Source ownership evidence:
  `include/cgpui/ui/toggle_builder.hpp`,
  `include/cgpui/ui/element_choice_nodes.hpp`,
  `src/ui/widgets/toggle_builder.cpp`,
  `src/ui/element_choice_nodes.cpp`,
  `src/ui/element_choice_layout.cpp`,
  `src/ui/element_choice_paint.cpp`,
  `tests/ui/builtin_widget_test.cpp`, and
  `tests/architecture/widget_source_structure_test.cpp`.
- Phase C Step 345 slider widget: `SliderBuilder`, `slider(...)`, and
  `SliderElement` expose focused slider authoring after the Step 344
  checkbox/radio/switch boundary.
- Slider behavior and accessibility evidence: range/value/step clamping,
  click-to-value change handling, `AccessibilityRole::slider`,
  `PlatformAccessibilityRole::slider`, and string accessibility values are
  guarded by `tests/ui/builtin_widget_test.cpp`.
- Slider source ownership evidence:
  `include/cgpui/ui/slider_builder.hpp`,
  `include/cgpui/ui/element_slider_nodes.hpp`,
  `src/ui/widgets/slider_builder.cpp`,
  `src/ui/element_slider_nodes.cpp`,
  `src/ui/element_slider_layout.cpp`, and
  `src/ui/element_slider_paint.cpp`.
- Phase C Step 346 list/menu widget follow-up: `ItemBuilder`,
  `list_item(...)`, `menu_item(...)`, and `ItemElement` expose focused item
  authoring after the Step 345 slider boundary.
- Item behavior and accessibility evidence: list items report selected or
  unselected values, menu items dispatch action clicks without selection
  state, disabled items ignore synthesized clicks, and
  `AccessibilityRole::list_item`, `AccessibilityRole::menu_item`,
  `PlatformAccessibilityRole::list_item`, and
  `PlatformAccessibilityRole::menu_item` are guarded by
  `tests/ui/builtin_widget_test.cpp`.
- Item source ownership evidence:
  `include/cgpui/ui/item_builder.hpp`,
  `include/cgpui/ui/element_item_nodes.hpp`,
  `src/ui/widgets/item_builder.cpp`,
  `src/ui/element_item_nodes.cpp`,
  `src/ui/element_item_layout.cpp`, and
  `src/ui/element_item_paint.cpp`.
- Phase C Step 347 icon/image widget follow-up: `ImageBuilder`, `image(...)`,
  `icon(...)`, `ImageElement`, and `ImageElementKind` expose focused image and
  icon authoring over existing image asset descriptors after the Step 346
  list/menu boundary.
- Image/icon behavior and accessibility evidence: image widgets preserve asset
  descriptors, source-rect cropping, alternate text, and non-focusable image
  accessibility via `AccessibilityRole::image` and
  `PlatformAccessibilityRole::image`; icon widgets add square sizing and
  optional tint metadata that flows through `ImagePaint` and `ImageDraw`.
- Image/icon source ownership evidence:
  `include/cgpui/ui/image_builder.hpp`,
  `include/cgpui/ui/element_image_nodes.hpp`,
  `src/ui/widgets/image_builder.cpp`,
  `src/ui/element_image_nodes.cpp`,
  `src/ui/element_image_layout.cpp`, and
  `src/ui/element_image_paint.cpp`.
- Phase C Step 348 container primitive follow-up: `div()`, `h_flex()`,
  `v_flex()`, `h_stack()`, and `v_stack()` expose focused container primitive
  authoring through a public widget leaf after the Step 347 image/icon
  boundary.
- Container primitive source ownership evidence:
  `include/cgpui/ui/container_builder.hpp`,
  `src/ui/widgets/container_builder.cpp`,
  `include/cgpui/ui/widget_builders.hpp`, and focused coverage in
  `tests/ui/builtin_widget_test.cpp` plus
  `tests/architecture/widget_source_structure_test.cpp`.
- Phase C Step 349 uniform list parity: `UniformListVisibleRange`,
  `UniformListItemIdentity`, `UniformListLayoutSnapshot`,
  `calculate_uniform_list_visible_range(...)`, and
  `ScrollableListElement::layout_snapshot()` expose stable item identity and
  the first virtualized visible-range boundary over the existing
  `scrollable_list` path.
- Uniform list source ownership evidence:
  `include/cgpui/ui/uniform_list.hpp`, `src/ui/uniform_list.cpp`,
  `include/cgpui/ui/element_scroll_nodes.hpp`,
  `src/ui/element_scroll_layout.cpp`, `tests/ui/scroll_test.cpp`,
  `tests/ui/element_test.cpp`, and
  `tests/architecture/ui_source_structure_test.cpp`.
- Phase C Step 350 scroll anchoring: `UniformListScrollAnchor`,
  `capture_uniform_list_scroll_anchor(...)`, and
  `apply_uniform_list_scroll_anchor(...)` preserve a keyed visible item's
  viewport offset after preceding item size changes. `ScrollableListElement`
  captures the previous snapshot anchor before relayout, applies it after the
  new snapshot is built, and recalculates the visible range from the adjusted
  scroll state.
- Phase C Step 351 item measurement cache: `UniformListItemMeasurement`,
  `UniformListItemMeasurementResult`, `UniformListItemMeasurementCache`,
  `measure_uniform_list_items(...)`, and
  `ScrollableListElement::measurement_cache()` record keyed item sizes and
  cache hit/miss stats over the existing snapshot/layout boundary. The focused
  implementation lives in `src/ui/uniform_list_measurement.cpp`, with layout
  integration in `src/ui/element_scroll_layout.cpp` and coverage in
  `tests/ui/scroll_test.cpp`, `tests/ui/element_test.cpp`, and
  `tests/architecture/ui_source_structure_test.cpp`.
- Phase C Step 352 large-list recycling: `UniformListRecyclingWindow` and
  `calculate_uniform_list_recycling_window(...)` expand the visible range by a
  focused overscan window, record recycled before/after counts and measured
  extents, store `UniformListLayoutSnapshot::recycling_window`, and mark
  `UniformListItemIdentity::recycled` so `ScrollableListElement::paint(...)`
  skips items outside the retained large-list window. The focused
  implementation lives in `src/ui/uniform_list_recycling.cpp`, with layout and
  paint integration in `src/ui/element_scroll_layout.cpp` and
  `src/ui/element_scroll_paint.cpp`.
- Phase C Step 353 keyboard/pointer selection:
  `UniformListSelectionSource`, `UniformListSelectionDirection`,
  `UniformListSelection`, `UniformListSelectionState`,
  `select_uniform_list_item_at_point(...)`,
  `move_uniform_list_selection(...)`, and
  `ScrollableListElement::selection()` expose focused pointer hit selection and
  keyboard previous/next/first/last movement. The focused implementation lives
  in `include/cgpui/ui/uniform_list_selection.hpp`,
  `src/ui/uniform_list_selection.cpp`, and
  `src/ui/element_scroll_events.cpp`, with layout-selected snapshot flags in
  `src/ui/element_scroll_layout.cpp`.
- Phase C Step 354 uniform list closeout:
  `tests/api_parity/phase_c_uniform_list_audit_test.cpp` guards the Steps
  349-353 uniform-list evidence, freezes the focused public/source ownership
  boundary, and moves the Phase C handoff to Step 355 window/examples widgets.
- Phase C Step 355 window/examples widgets:
  `examples/api_parity/public_window_examples/main.cpp` and
  `api_parity_public_window_examples` start the public example band for menu demos,
  shadow, window positioning, window shadow, and input examples using public
  APIs only. `tests/api_parity/phase_c_window_examples_public_api_test.cpp`
  guards the source against private headers and keeps the roadmap, ledger,
  vocabulary, and xmake target aligned.
- Phase C Step 356 window/examples workflow:
  `examples/api_parity/public_window_examples_workflow/main.cpp` and
  `api_parity_public_window_examples_workflow` deepen the window/examples band
  with a public test-context workflow for menu installation, window
  activation/focus, key binding simulation, pointer dispatch, text input, and
  shadow/fixed positioning examples. The guard
  `tests/api_parity/phase_c_window_examples_workflow_test.cpp` keeps the source
  prelude-only and synchronized with the roadmap, ledger, vocabulary, and xmake
  target.
- Phase C Step 357 window/examples widget catalog:
  `examples/api_parity/public_window_examples_widget_catalog/main.cpp` and
  `api_parity_public_window_examples_widget_catalog` continue the
  window/examples widget band with checkbox/radio/switch, slider, list/menu,
  image/icon, and container widgets using public APIs. The guard
  `tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp` keeps the
  source prelude-only and synchronized with the roadmap, ledger, vocabulary,
  and xmake target. The frozen Step 357 handoff remains: Step 358 should continue the window/examples widget band.
- Phase C Step 358 window/examples interaction states:
  `examples/api_parity/public_window_examples_interaction_states/main.cpp` and
  `api_parity_public_window_examples_interaction_states` continue the
  window/examples widget band with hover/focus/active/disabled interaction states,
  focus ring and tab-index examples, keyed controls, and click handlers
  using public APIs. The guard
  `tests/api_parity/phase_c_window_examples_interaction_states_test.cpp` keeps
  the source prelude-only and synchronized with the roadmap, ledger,
  vocabulary, and xmake target.
- Phase C Step 359 window/examples service matrix:
  `examples/api_parity/public_window_examples_service_matrix/main.cpp` and
  `api_parity_public_window_examples_service_matrix` continue the
  window/examples widget band with menu accelerators and command palette service matrix
  plus window options, shadow, fixed positioning, and text input service examples
  using public APIs. The guard
  `tests/api_parity/phase_c_window_examples_service_matrix_test.cpp` keeps the
  source prelude-only and synchronized with the roadmap, ledger, vocabulary,
  and xmake target.
- Phase C Step 360 window/examples closeout:
  `tests/api_parity/phase_c_window_examples_closeout_test.cpp` guards the
  Steps 355-359 window/examples evidence, freezes the prelude-only public
  example boundary, and closes the window/examples widget band before Phase C
  Step 361 SVG/image element front-end APIs.
- Handoff: Phase C Step 361 SVG/image element front-end APIs is next.
- Phase C Step 361 SVG/image front-end source APIs:
  `ImageSource`, `ImageSourceKind`, `image_source(...)`,
  `svg_image_source(...)`, and `svg(...)` provide the author-facing source
  boundary for raster image descriptors and SVG source strings. The focused
  leaf `include/cgpui/ui/image_source.hpp` and implementation
  `src/ui/image_source.cpp` preserve SVG metadata on `ImageElement` while
  paint continues to feed the existing `ImageAssetDescriptor` pipeline. The
  behavior is guarded by `tests/api_parity/phase_c_svg_image_front_end_test.cpp`,
  `tests/ui/builtin_widget_test.cpp`, and
  `tests/architecture/widget_source_structure_test.cpp`.
- Phase C Step 362 SVG/image asset registration: `ImageAssetRegistry`,
  `RegisteredImageAsset`, `register_image(...)`, `register_svg(...)`,
  `ImageAssetRegistry::find(...)`, `registrations()`, and `raster_assets()`
  provide deterministic public asset registration over the Step 361
  `ImageSource` boundary. Raster registrations can allocate ids for id-less
  `ImageAsset` values and retain raster assets for later upload planning; SVG
  registrations preserve source strings and descriptor metadata without decoding
  or renderer upload behavior. Evidence lives in
  `include/cgpui/ui/image_asset_registry.hpp`,
  `src/ui/image_asset_registry.cpp`,
  `tests/api_parity/phase_c_svg_image_asset_registration_test.cpp`, and
  `tests/architecture/widget_source_structure_test.cpp`.
- Phase C Step 363 SVG/image public example coverage:
  `examples/api_parity/public_svg_image_sources/main.cpp`,
  `api_parity_public_svg_image_sources`, and
  `tests/api_parity/phase_c_svg_image_public_examples_test.cpp` keep the
  registered raster/SVG source workflow prelude-only. The
  `public_svg_image_sources` example registers an in-memory raster asset and an
  SVG source string with `ImageAssetRegistry`, checks the resulting
  `ImageSourceKind` values, uses `registry.raster_assets()` for upload-planning
  observability, and feeds `RegisteredImageAsset::source()` into `image(...)`
  and `svg(...)`.
- Phase C Step 364 SVG/image band closeout:
  `tests/api_parity/phase_c_svg_image_closeout_test.cpp` guards the Step 361
  front-end source APIs, Step 362 asset registry, and Step 363 prelude-only
  public registered-source example evidence. This closes the Phase C SVG/image
  front-end authoring band while keeping SVG decoding, PNG/JPEG loading,
  renderer upload, GPU texture lifetime, private runtime headers, and direct
  `WindowRuntime` use out of scope for this band.
- Phase C Step 367 widget family structure tests:
  `tests/architecture/widget_source_structure_test.cpp` now uses a
  `WidgetFamilyBoundary` table requiring every widget family to name its
  public leaf header, focused source file, and focused behavior tests. The table
  covers label, button, text input, toggle controls, slider, list/menu items,
  image/icon/SVG, container primitives, and scrollable list families.
  `tests/api_parity/phase_c_widget_family_structure_test.cpp` guards the
  roadmap, ledger, and public vocabulary evidence for this structure freeze.
- Phase C Step 373 final element/style/widget ledger audit:
  `tests/api_parity/phase_c_final_ledger_audit_test.cpp` closes the Phase C
  element/style/widget band by checking the prior closeout gates:
  `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`,
  `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`,
  `tests/api_parity/phase_c_uniform_list_audit_test.cpp`,
  `tests/api_parity/phase_c_window_examples_closeout_test.cpp`,
  `tests/api_parity/phase_c_svg_image_closeout_test.cpp`, and
  `tests/api_parity/phase_c_widget_family_structure_test.cpp`. The final audit
  keeps `gpui::div` and `gpui uniform_list` adapted/closed, leaves image/SVG
  production decoding/loading/upload/lifetime work explicitly deferred, and
  hands the roadmap to Phase D Step 379 text/font shaping.
- Handoff: Phase C element/style/widget band closed.

## Phase A Closure

Phase A is complete when this ledger, the JSON export, the extractor, the
first parity example, and `gpui_parity_ledger_test/default` all pass on
Windows and WSL. Later phases close required rows by moving their `status` or
`next_step` fields only when tests prove the new behavior.
