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
| gpui::div | `div()` plus repeated `.child(...)` multi-child retention, `ElementBuilder::children(...)` collection authoring, `StyledElement::children()` child-list ownership, upstream-style flex vocabulary aliases `ElementBuilder::items_start()`, `ElementBuilder::items_center()`, `ElementBuilder::items_end()`, `ElementBuilder::justify_start()`, `ElementBuilder::justify_center()`, `ElementBuilder::justify_end()`, `ElementBuilder::justify_between()`, and `ElementBuilder::flex_1()`, upstream-style sizing/color/border aliases `ElementBuilder::w(...)`, `ElementBuilder::h(...)`, `ElementBuilder::bg(...)`, `ElementBuilder::text_color(...)`, `ElementBuilder::border_1()`, and `ElementBuilder::rounded(...)`, upstream-style overflow/opacity/position aliases `ElementBuilder::overflow(...)`, `ElementBuilder::overflow_hidden()`, `ElementBuilder::overflow_visible()`, `ElementBuilder::opacity(...)`, `ElementBuilder::z_index(...)`, `ElementBuilder::relative()`, `ElementBuilder::top(...)`, `ElementBuilder::right(...)`, `ElementBuilder::bottom(...)`, and `ElementBuilder::left(...)`, upstream-style text aliases `ElementBuilder::text_size(...)` and `ElementBuilder::font_family(...)`, focused shadow vocabulary/storage through `BoxShadow`, `Style::box_shadow`, `StyleOverlay::box_shadow`, `ElementBuilder::shadow(...)`, `ElementBuilder::shadow_sm()`, and `PaintCommandKind::box_shadow` paint-list observability, focused layout constraint vocabulary through `Style::min_size`, `Style::max_size`, `StyleOverlay::min_size`, `StyleOverlay::max_size`, `ElementBuilder::min_size(...)`, `ElementBuilder::max_size(...)`, `ElementBuilder::min_w(...)`, `ElementBuilder::min_h(...)`, `ElementBuilder::max_w(...)`, `ElementBuilder::max_h(...)`, and `StyledElement::layout` min/max constraint merging, focused percentage-like sizing through `PercentageSize`, `Style::percentage_size`, `StyleOverlay::percentage_size`, `ElementBuilder::size_pct(...)`, `ElementBuilder::w_pct(...)`, `ElementBuilder::h_pct(...)`, and `StyledElement::layout` resolution against finite external max constraints with unconstrained fallback, focused margin/padding shorthand vocabulary through `ElementBuilder::p(...)`, `ElementBuilder::px(...)`, `ElementBuilder::py(...)`, `ElementBuilder::pt(...)`, `ElementBuilder::pr(...)`, `ElementBuilder::pb(...)`, `ElementBuilder::pl(...)`, `ElementBuilder::m(...)`, `ElementBuilder::mx(...)`, `ElementBuilder::my(...)`, `ElementBuilder::mt(...)`, `ElementBuilder::mr(...)`, `ElementBuilder::mb(...)`, `ElementBuilder::ml(...)`, and existing `StyledElement::layout` gap composition, focused absolute/fixed positioning through `Position::fixed`, `ElementBuilder::fixed()`, and out-of-flow absolute/fixed child layout in `StyledElement::layout` and `FlexElement::layout`, and focused direct overlay child ordering through `z_order()` for direct `StyledElement`, flex, and vertical-stack child paint, hit-test, and event dispatch via `paint_ordered_children(...)`, `hit_test_ordered_children(...)`, and `event_ordered_children(...)`, and focused nested scroll clipping through PaintList::push_clip(...) effective clip intersection, private paint_clip helpers, hidden-overflow plus scrollable-list clip propagation, and nested clip-stack metadata, and focused style cascade active-state support through `StyleState::active`, `StyleStateFlags::active`, `ElementBuilder::active_style(...)`, `ButtonBuilder::active_style(...)`, and hover/focus/active/disabled resolution order for local and class styles with inline overlays remaining last, and focused class-style reuse depth through `StyleClassRule`, `StyleCascade::set_class_rule(...)`, `StyleCascade::class_rule(...)`, `src/ui/style_cascade.cpp` ownership, and depth-first reused class resolution with cycle protection, and focused theme token fallback through `StyleThemeTokens`, `Style::with_background_color_token(...)`, `StyleOverlay::with_background_color_token(...)`, theme-aware `resolved_style(...)` overloads, `StyledElement::resolved_style(..., Theme)`, `src/ui/style_theme_tokens.cpp` token lookup, `src/ui/style_theme_cascade.cpp` ordering, and missing-token fallback to existing concrete style values, and focused inherited text style through `StyleAuthoredTextFields`, `LabelElement` / `TextElement` effective text style storage, private `text_style_inheritance` helpers, styled/flex/vertical-stack/wrapper inheritance forwarding, and explicit child text-style precedence, and Phase C Step 335 dynamic style invalidation through `WindowRuntime::request_style_state_invalidation(...)`, hover-target transition calls from `src/ui/runtime_event_input.cpp`, keyboard-focus transition calls from `src/ui/runtime_focus.cpp`, and focused `src/ui/runtime_style_invalidation.cpp` ownership, and Phase C Step 336 style cascade depth closeout through `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp` guarding the Steps 331-335 style-cascade evidence before Phase C Step 337 focusable/interactable semantics, and Phase C Step 337 pointer-active input semantics through `ViewInputState::active_element_id`, `WindowRuntime::update_active_state_for_event(...)`, left-button press/release `active_element_id` tracking in `src/ui/runtime_active_state.cpp`, active style-state invalidation via `request_style_state_invalidation(...)`, and focused `tests/ui/window_runtime_input_test.cpp` coverage, and Phase C Step 338 tab-order/focus-ring metadata through `FocusMetadata`, `FocusRingVisibility`, `ElementBuilder::tab_index(...)`, `ElementBuilder::focus_ring(...)`, `AccessibilityNode::tab_index` / `focus_ring` reporting, and `WindowRuntime::ordered_focusable_element_ids()` traversal ordering in `src/ui/runtime_focus_order.cpp` so positive tab indices precede default tree order and negative tab indices are skipped by Tab traversal, with focused `tests/ui/element_test.cpp` and `tests/ui/window_runtime_focus_test.cpp` coverage, and Phase C Step 339 click/drag gesture synthesis through `ElementGestureKind::click`, `ElementEventContext::gesture`, `ViewInputState::pointer_down_element_id`, `ViewInputState::clicked_element_id`, `ViewInputState::dragging_element_id`, `ViewInputState::dragging`, focused `src/ui/runtime_gesture_synthesis.cpp` ownership, `WindowRuntime::update_active_state_for_event(...)` gesture-state updates, `dispatch_synthesized_click_event(...)` after raw routed release handlers, `ClickElement` / `ButtonElement` synthesized-click gating, and focused `tests/ui/window_runtime_input_test.cpp` coverage including `test_runtime_synthesizes_click_on_release_over_pressed_element` and `test_runtime_suppresses_click_after_drag_gesture`, plus `tests/ui/element_test.cpp` coverage, and Phase C Step 340 keyboard activation semantics through `should_dispatch_synthesized_keyboard_activation_event(...)`, focused Enter/Space synthesized-click activation, raw key handler first-refusal suppression, `KeyElement` synthesized-click delegation, and focused `tests/ui/window_runtime_focus_test.cpp` coverage including `test_runtime_enter_space_activate_focused_button` and `test_runtime_raw_key_handler_controls_keyboard_activation`, and Phase C Step 341 disabled interaction semantics through `WindowRuntime::refresh_disabled_interaction_state()`, focused `src/ui/runtime_disabled_interaction.cpp` ownership, stale hover/active/focus/pointer-capture/pointer-down/click/drag state cleanup for disabled or missing elements, default cursor restoration, and focused `tests/ui/window_runtime_input_test.cpp` coverage including `test_runtime_clears_disabled_interaction_state`, and Phase C Step 342 focusable/interactable band closeout through `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp` guarding the Steps 337-341 focusable/interactable evidence before Phase C Step 343 built-in widget expansion, and Phase C Step 343 built-in widget expansion through ButtonBuilder::label(...), focused src/ui/widgets/button_builder.cpp, src/ui/widgets/label_builder.cpp, and src/ui/widgets/text_input_builder.cpp ownership, xmake src/ui/widgets/*.cpp compilation, tests/ui/builtin_widget_test.cpp, and tests/architecture/widget_source_structure_test.cpp | Adapted | `include/cgpui/ui/element_builder_core.hpp`, `include/cgpui/ui/element_style_nodes.hpp`, `include/cgpui/ui/style_tokens.hpp`, `include/cgpui/ui/style_values.hpp`, `include/cgpui/ui/style_box.hpp`, `include/cgpui/ui/style_overlay.hpp`, `include/cgpui/ui/style_state.hpp`, `include/cgpui/ui/style_cascade.hpp`, `include/cgpui/ui/button_builder.hpp`, `src/ui/element_builder_interaction.cpp`, `src/ui/element_builder_layout.cpp`, `src/ui/element_builder_style.cpp`, `src/ui/widgets/button_builder.cpp`, `src/ui/element_builder_build.cpp`, `src/ui/element_flex_layout.cpp`, `src/ui/element_style_nodes.cpp`, `src/ui/element_style_paint.cpp`, `src/ui/element_layer_ordering.hpp`, `src/ui/element_layer_ordering.cpp`, `src/ui/element_flex_node.cpp`, `src/ui/element_vertical_stack_node.cpp`, `src/ui/text_style_inheritance.hpp`, `src/ui/text_style_inheritance.cpp`, `src/ui/element_text_paint.cpp`, `src/ui/element_focus_nodes.cpp`, `src/ui/element_pointer_nodes.cpp`, `src/ui/element_button_nodes.cpp`, `src/ui/style_box.cpp`, `src/ui/style_overlay.cpp`, `src/ui/style_cascade.cpp`, `src/ui/style_cascade_overlays.hpp`, `src/ui/style_cascade_overlays.cpp`, `src/ui/style_theme_tokens.hpp`, `src/ui/style_theme_tokens.cpp`, `src/ui/style_theme_cascade.cpp`, `src/ui/paint_shadow.cpp`, `tests/ui/element_test.cpp`, `tests/ui/style_test.cpp`, `tests/architecture/ui_source_structure_test.cpp`, `src/ui/runtime_style_invalidation.cpp`, `src/ui/runtime_active_state.cpp`, `src/ui/runtime_gesture_synthesis.hpp`, `src/ui/runtime_gesture_synthesis.cpp`, `src/ui/runtime_event_route_dispatch.cpp`, `src/ui/runtime_disabled_interaction.cpp`, `include/cgpui/ui/runtime_input_state.hpp`, `include/cgpui/ui/focus_metadata.hpp`, `src/ui/element_focus_metadata.cpp`, `src/ui/runtime_focus_order.cpp`, `tests/ui/window_runtime_input_test.cpp`, `tests/ui/window_runtime_focus_test.cpp`, `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`, `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`, src/ui/paint.cpp, src/ui/paint_clip.hpp, src/ui/paint_clip.cpp, src/ui/element_scroll_paint.cpp | Phase C element/style/widget band closed |
| gpui::prelude | `cgpui/prelude.hpp` plus thin `cgpui/cgpui.hpp` compatibility aggregate, prelude-only public API compatibility examples, and a public authoring vocabulary freeze document | Adapted | `include/cgpui/prelude.hpp`, `include/cgpui/cgpui.hpp`, `examples/api_parity/public_api_compatibility/main.cpp`, `examples/api_parity/public_authoring_workflow/main.cpp`, `examples/api_parity/public_context_capabilities/main.cpp`, `examples/api_parity/public_async_test_workflow/main.cpp`, `examples/api_parity/public_phase_b_surface_closure/main.cpp`, `docs/gpui-public-authoring-vocabulary.md`, `tests/api_parity/public_authoring_surface_test.cpp`, `tests/api_parity/public_api_compatibility_examples_test.cpp`, `tests/api_parity/public_api_example_expansion_test.cpp`, `tests/api_parity/public_context_capability_example_test.cpp`, `tests/api_parity/public_async_test_workflow_example_test.cpp`, `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`, `tests/header_cleanliness/prelude_header_cleanliness.cpp` | Phase B final verification/freeze |
| gpui::actions! / action macro | `Action<T>` typed action concept, `action_name<T>()` / `action_name_v<T>`, typed register/dispatch overloads over the existing string action registry, action registration metadata for general/app/window/view/focused-element scopes, typed action command-palette metadata binding, action enablement metadata that suppresses disabled action dispatch, action bubbling through focused routes, key binding grammar parsing for `Context<T>::bind_key("ctrl-shift-s", "action")`, platform modifier semantics for `secondary-*` and `platform-*` bindings, keymap context filtering for app/window/view/focused-element bindings, partial key sequence matching for multi-chord bindings, disabled key scope filtering, and command palette key integration that parses `CommandPaletteEntry::key_binding` and installs derived or explicit `KeyBindingContext` bindings | Required | `include/cgpui/ui/action.hpp`, `include/cgpui/ui/key_binding.hpp`, `include/cgpui/ui/runtime_action_templates.hpp`, `include/cgpui/ui/runtime_action_enablement_templates.hpp`, `include/cgpui/ui/runtime_command_palette_templates.hpp`, `include/cgpui/ui/runtime_actions.hpp`, `src/ui/runtime_action_dispatch.cpp`, `src/ui/runtime_action_registration.cpp`, `src/ui/runtime_action_metadata.cpp`, `src/ui/runtime_command_palette_keys.cpp`, `src/ui/runtime_key_binding_grammar.cpp`, `src/ui/runtime_key_binding_modifiers.cpp`, `src/ui/runtime_key_binding_contexts.cpp`, `src/ui/runtime_key_binding_sequences.cpp`, `tests/api_parity/typed_action_surface_test.cpp`, `tests/ui/typed_action_dispatch_test.cpp`, `tests/ui/action_scope_metadata_test.cpp`, `tests/ui/typed_action_command_metadata_test.cpp`, `tests/ui/action_enablement_metadata_test.cpp`, `tests/ui/action_bubbling_test.cpp`, `tests/ui/key_binding_grammar_test.cpp`, `tests/ui/key_binding_platform_modifier_test.cpp`, `tests/ui/keymap_context_test.cpp`, `tests/ui/key_binding_partial_match_test.cpp`, `tests/ui/key_binding_disabled_scope_test.cpp`, `tests/ui/command_palette_key_integration_test.cpp`; action macro payloads still missing | Phase B action metadata/key dispatch depth |
| gpui key_context | `KeyBindingContext` for app/window/view/focused-element key binding activation | Required | context-aware key bindings exist without the full upstream key-context tree API | Phase B |
| gpui keymap dispatch | `KeyBinding`, `KeyBindingChord`, `KeyBindingContext`, `parse_key_binding(...)`, `parse_key_binding(..., DesktopPlatformTarget)`, `Context<T>::bind_key("ctrl-shift-s", "action")`, `Context<T>::bind_key("ctrl-k ctrl-s", "action")`, `Context<T>::bind_key(..., KeyBindingContext)`, command-palette-owned key binding metadata, and action registry dispatch | Required | deterministic key binding plus GPUI-style chord grammar, platform modifier semantics, active context filtering, pending partial sequence matching, disabled key scope filtering, and command palette key integration exist; `secondary-*` maps to Ctrl on Windows/Linux and Super on macOS, while `platform-*` / `cmd-*` / `win-*` map to the platform key. Action macro payloads remain missing | Phase B |
| gpui element styling | `Style`, `StyleOverlay`, builders | Required | many primitives exist, tailwind-style vocabulary incomplete | Phase C |
| gpui uniform_list | `scrollable_list` plus `UniformListVisibleRange`, `UniformListItemIdentity`, `UniformListItemMeasurement`, `UniformListItemMeasurementResult`, `UniformListRecyclingWindow`, `UniformListSelectionSource`, `UniformListSelectionDirection`, `UniformListSelection`, `UniformListSelectionState`, `UniformListLayoutSnapshot`, `UniformListScrollAnchor`, `UniformListItemMeasurementCache`, `calculate_uniform_list_visible_range(...)`, `measure_uniform_list_items(...)`, `calculate_uniform_list_recycling_window(...)`, `select_uniform_list_item_at_point(...)`, `move_uniform_list_selection(...)`, `capture_uniform_list_scroll_anchor(...)`, `apply_uniform_list_scroll_anchor(...)`, `ScrollableListElement::layout_snapshot()`, `ScrollableListElement::measurement_cache()`, and `ScrollableListElement::selection()` | Adapted | stable item identity, visible-range snapshots, keyed scroll anchoring, keyed item measurement cache stats, retained/recycled large-list paint windows, pointer hit selection, keyboard movement selection, snapshot selected flags, and Step 354 audit closeout exist | Phase C uniform-list band closed |
| gpui text system | `TextModel`, shaping, wrap, glyph records | Required | deterministic text depth exists; Phase D Steps 379-399 add `TextShapingBackend` selection, HarfBuzz capability reporting, explicit fallback reason metadata, glyph ids, glyph positioning offsets, shaping direction/script/language metadata, internal shaping dispatch/fallback backend boundaries, a guarded `src/ui/text_shaping_harfbuzz.cpp` insertion point, backend capability snapshots on selections/runs, wrapped glyph-id propagation, a text-shaping readiness audit, a focused `src/ui/text_font.cpp` implementation boundary for font database/discovery helpers, platform font discovery result/diagnostics boundaries through `PlatformFontDiscoveryResult`, `PlatformApplication::discover_font_discovery()`, `src/platform/platform_font_discovery.cpp`, `src/platform/win32/win32_font_discovery.cpp`, and `src/platform/linux/wayland_font_discovery.cpp`, real Win32 DirectWrite system font-family enumeration with native-available diagnostics and deterministic fallback on DirectWrite failure, a guarded Linux fontconfig backend insertion point in `src/platform/linux/wayland_fontconfig_discovery.cpp` with Wayland fallback/native diagnostics coverage, coverage-aware fallback records through `FontUnicodeRange` and `FontDatabase::resolve_chain_for_codepoint(...)`, explicit fallback-chain shaping via `shape_text(..., FontFallbackChain, ...)` and `TextShapeRun::font_fallback_faces`, glyph-level fallback face selection through `TextGlyphRun::font_fallback_face_index`, the Step 394 font fallback band audit, contiguous fallback font spans through `TextFontFallbackRun` and `TextShapeRun::font_runs`, explicit missing-glyph diagnostics through `TextMissingGlyphDiagnostic` and `TextShapeRun::missing_glyphs`, emoji-plane color glyph planning through `TextColorGlyphPlan` and `TextShapeRun::color_glyphs`, emoji presentation selector planning for `U+FE0F` through `is_emoji_presentation_selector(...)`, `codepoint_accepts_emoji_presentation(...)`, and `append_emoji_presentation_color_glyph_plan(...)`, and selector-span metadata through `TextColorGlyphPlan::has_emoji_presentation_selector`, `TextColorGlyphPlan::emoji_presentation_selector_byte_offset`, `TextColorGlyphPlan::emoji_presentation_selector_byte_length`, and `mark_emoji_presentation_selector_span(...)`; production HarfBuzz shaping, real color glyph rendering, and dependency-backed native Linux fontconfig/FreeType font enumeration remain incomplete | Phase D |
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
