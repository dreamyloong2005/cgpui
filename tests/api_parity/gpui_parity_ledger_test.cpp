#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

int require_contains_all(
    const std::string& text,
    const std::vector<const char*>& values,
    int exit_code) {
  for (const char* value : values) {
    if (!contains(text, value)) {
      return exit_code;
    }
  }
  return 0;
}

} // namespace

int main() {
  const std::string pinned =
      read_source("docs/gpui-upstream-pinned-revision.md");
  if (pinned.empty()) {
    return 1;
  }
  if (const int result = require_contains_all(
          pinned,
          {
              "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0",
              "gpui = 0.2.2",
              "gpui_platform = 0.1.0",
              "https://github.com/zed-industries/zed/tree/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui",
              "README.md",
              "src/gpui.rs",
              "docs/contexts.md",
              "docs/key_dispatch.md",
              "examples/hello_world.rs",
          },
          2);
      result != 0) {
    return result;
  }

  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  if (ledger.empty()) {
    return 3;
  }
  if (const int result = require_contains_all(
          ledger,
          {
              "# Complete GPUI Parity Ledger",
              "Pinned upstream revision",
              "Required",
              "Adapted",
              "Deferred",
              "Non-goal",
              "Application and app context",
              "Entities and state",
              "Views and Render",
              "Elements and styling",
              "Actions and key dispatch",
              "Platform services",
              "Async executor",
              "Test support",
              "Examples",
              "Windows: Win32 + Vulkan",
              "Linux: Wayland + Vulkan",
              "macOS: Cocoa + Metal",
              "X11",
          },
          4);
      result != 0) {
    return result;
  }
  if (!contains(ledger, "| upstream_gpui | cgpui_target | status |") ||
      !contains(ledger, "| gpui::Application |") ||
      !contains(ledger, "| gpui::App |") ||
      !contains(ledger, "| gpui::Window |") ||
      !contains(ledger, "| gpui::Context<T> |") ||
      !contains(ledger, "| gpui::View<T> |") ||
      !contains(ledger, "| gpui::Render |") ||
      !contains(ledger, "| gpui::prelude |") ||
      !contains(ledger, "| gpui::div |") ||
      !contains(ledger, "| gpui::test |") ||
      !contains(ledger, "| gpui_platform::application |")) {
    return 5;
  }
  if (!contains(ledger, "`App` facade from `AppContext::app()`") ||
      !contains(ledger, "`Context<T>::app_context()` app-domain capability") ||
      !contains(ledger, "`try_open_window(...) -> Result<AppOpenedWindow>`") ||
      !contains(ledger, "src/ui/runtime_window_results.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_result_conventions_test.cpp") ||
      !contains(ledger, "`try_create_renderer(...) -> Result<Renderer*>`") ||
      !contains(ledger, "src/ui/runtime_renderer_results.cpp") ||
      !contains(ledger,
                "tests/api_parity/renderer_result_conventions_test.cpp") ||
      !contains(ledger, "`try_resize_surface(...) -> Result<void>`") ||
      !contains(ledger, "src/ui/runtime_renderer_resize_results.cpp") ||
      !contains(ledger,
                "tests/api_parity/renderer_resize_result_conventions_test.cpp") ||
      !contains(ledger, "`try_draw_frame() -> Result<void>`") ||
      !contains(ledger, "src/ui/runtime_renderer_frame_results.cpp") ||
      !contains(ledger,
                "tests/api_parity/renderer_frame_result_conventions_test.cpp") ||
      !contains(ledger, "`try_install_native_menu(...) -> "
                        "Result<NativeMenuInstallation>`") ||
      !contains(ledger, "`try_show_native_file_dialog(...) -> "
                        "Result<NativeFileDialogResult>`") ||
      !contains(ledger, "src/ui/runtime_platform_service_results.cpp") ||
      !contains(ledger,
                "tests/api_parity/platform_service_result_conventions_test.cpp") ||
      !contains(ledger, "`try_spawn_task(...) -> Result<TaskHandle>`") ||
      !contains(ledger,
                "`try_spawn_background_task(...) -> Result<TaskHandle>`") ||
      !contains(ledger, "src/ui/runtime_task_results.cpp") ||
      !contains(ledger,
                "tests/api_parity/async_spawn_result_conventions_test.cpp") ||
      !contains(ledger, "`ElementBuilder::children(...)`") ||
      !contains(ledger, "`StyledElement::children()`") ||
      !contains(ledger, "`ElementBuilder::items_center()`") ||
      !contains(ledger, "`ElementBuilder::justify_between()`") ||
      !contains(ledger, "`ElementBuilder::flex_1()`") ||
      !contains(ledger, "`ElementBuilder::w(...)`") ||
      !contains(ledger, "`ElementBuilder::bg(...)`") ||
      !contains(ledger, "`ElementBuilder::border_1()`") ||
      !contains(ledger, "`ElementBuilder::rounded(...)`") ||
      !contains(ledger, "`ElementBuilder::overflow_hidden()`") ||
      !contains(ledger, "`ElementBuilder::overflow_visible()`") ||
      !contains(ledger, "`ElementBuilder::opacity(...)`") ||
      !contains(ledger, "`ElementBuilder::z_index(...)`") ||
      !contains(ledger, "`ElementBuilder::relative()`") ||
      !contains(ledger, "`ElementBuilder::fixed()`") ||
      !contains(ledger, "`Position::fixed`") ||
      !contains(ledger, "`ElementBuilder::top(...)`") ||
      !contains(ledger, "src/ui/element_builder_interaction.cpp") ||
      !contains(ledger, "src/ui/element_builder_layout.cpp") ||
      !contains(ledger, "src/ui/element_builder_style.cpp") ||
      !contains(ledger, "src/ui/element_flex_layout.cpp") ||
      !contains(ledger, "src/ui/element_style_nodes.cpp") ||
      !contains(ledger, "tests/architecture/ui_source_structure_test.cpp") ||
      !contains(ledger, "tests/ui/element_test.cpp") ||
      !contains(ledger,
                "`Window` facade from `WindowRuntimeContext::window()`") ||
      !contains(ledger, "tests/api_parity/app_window_context_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/app_context_capability_test.cpp")) {
    return 15;
  }
  if (!contains(ledger, "`Context<T>` alias with app/window/entity helpers") ||
      !contains(ledger, "`Context<T>::app_context()`") ||
      !contains(ledger, "`Context<T>::view_context<T>()`") ||
      !contains(ledger, "`Context<T>::window_context()`") ||
      !contains(ledger, "`Context<T>::element_context(ElementId)`") ||
      !contains(ledger, "`Context<T>::async_context()`") ||
      !contains(ledger, "`Context<T>::test_context()`") ||
      !contains(ledger, "`ElementContextCapability`") ||
      !contains(ledger, "`AsyncContextCapability`") ||
      !contains(ledger, "`TestContextCapability`") ||
      !contains(ledger, "`EntityHandle<T>`") ||
      !contains(ledger, "`Context<T>::new_entity<T>(...)`") ||
      !contains(ledger, "weak upgrade/read semantics") ||
      !contains(ledger, "`observe_entity(...)` helpers") ||
      !contains(ledger, "entity observation helpers") ||
      !contains(ledger, "entity-to-entity observation helpers") ||
      !contains(ledger, "window/view observation helpers") ||
      !contains(ledger, "`update_entity(...)` transaction helpers") ||
      !contains(ledger, "value-returning update transactions") ||
      !contains(ledger, "`invalidate_entity(...)` helpers") ||
      !contains(ledger, "entity invalidation helpers") ||
      !contains(ledger, "entity deletion helpers") ||
      !contains(ledger, "deterministic subscription lifetime/unsubscribe behavior") ||
      !contains(ledger, "deterministic `Subscription` lifetime/unsubscribe behavior") ||
      !contains(ledger, "view-removal subscription cleanup") ||
      !contains(ledger, "runtime-token cross-context boundaries") ||
      !contains(ledger, "`ViewContextCapability<T>`") ||
      !contains(ledger, "`ViewHandle<T>` and `WeakViewHandle<T>`") ||
      !contains(ledger, "`Render<T>` concept over") ||
      !contains(ledger, "`IntoElement` alias plus `into_element`") ||
      !contains(ledger, "include/cgpui/ui/render.hpp") ||
      !contains(ledger, "include/cgpui/ui/async_context.hpp") ||
      !contains(ledger, "include/cgpui/ui/test_context.hpp") ||
      !contains(ledger, "include/cgpui/ui/element_context.hpp") ||
      !contains(ledger, "include/cgpui/ui/view_handle.hpp") ||
      !contains(ledger, "include/cgpui/ui/window_context.hpp") ||
      !contains(ledger, "include/cgpui/prelude.hpp") ||
      !contains(ledger,
                "tests/api_parity/context_capabilities_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/view_handle_spelling_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/view_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/window_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/element_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/async_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/test_context_capability_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_authoring_surface_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_api_compatibility_examples_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_api_example_expansion_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_context_capability_example_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_async_test_workflow_example_test.cpp") ||
      !contains(
          ledger,
          "tests/api_parity/public_phase_b_surface_closure_example_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_authoring_vocabulary_freeze_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/public_phase_b_completion_audit_test.cpp") ||
      !contains(ledger,
                "examples/api_parity/public_api_compatibility/main.cpp") ||
      !contains(ledger,
                "examples/api_parity/public_authoring_workflow/main.cpp") ||
      !contains(ledger,
                "examples/api_parity/public_context_capabilities/main.cpp") ||
      !contains(ledger,
                "examples/api_parity/public_async_test_workflow/main.cpp") ||
      !contains(
          ledger,
          "examples/api_parity/public_phase_b_surface_closure/main.cpp") ||
      !contains(ledger, "docs/gpui-public-authoring-vocabulary.md") ||
      !contains(ledger,
                "tests/api_parity/entity_lifecycle_creation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_weak_handle_semantics_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_observation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_to_entity_observation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/window_view_observation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/subscription_lifetime_test.cpp") ||
      !contains(ledger,
                "tests/ui/window_runtime_observation_closure_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_transaction_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_invalidation_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/entity_deletion_test.cpp")) {
    return 16;
  }
  if (!contains(ledger, "| gpui_platform x11 feature | Deferred |") ||
      !contains(ledger, "| gpui_platform wayland feature | Required |") ||
      !contains(ledger, "| gpui Windows backend | Required |")) {
    return 6;
  }
  if (!contains(ledger, "| gpui::TestAppContext |") ||
      !contains(ledger, "`TestContextCapability` groups runtime id/view id") ||
      !contains(ledger, "live input/event/action observability") ||
      !contains(ledger, "direct pointer move/button/scroll simulation") ||
      !contains(ledger, "direct window activation/focus dispatch") ||
      !contains(ledger, "element focus/release helpers") ||
      !contains(ledger, "direct clipboard read/write helpers") ||
      !contains(ledger, "copy/cut/paste clipboard forwarding") ||
      !contains(ledger, "run-until-parked timer/async advancement helpers") ||
      !contains(ledger, "redraw/frame pump helpers") ||
      !contains(ledger, "`try_draw_frame() -> Result<void>`") ||
      !contains(ledger, "src/ui/test_context_pointer.cpp") ||
      !contains(ledger, "src/ui/test_context_focus.cpp") ||
      !contains(ledger, "src/ui/test_context_clipboard.cpp") ||
      !contains(ledger, "src/ui/test_context_scheduling.cpp") ||
      !contains(ledger, "src/ui/test_context_rendering.cpp") ||
      !contains(ledger, "src/ui/runtime_clipboard.cpp") ||
      !contains(ledger, "tests/ui/test_context_pointer_simulation_test.cpp") ||
      !contains(ledger, "tests/ui/test_context_focus_activation_test.cpp") ||
      !contains(ledger, "tests/ui/test_context_clipboard_test.cpp") ||
      !contains(ledger, "tests/ui/test_context_time_async_test.cpp") ||
      !contains(ledger, "tests/ui/test_context_frame_pump_test.cpp") ||
      !contains(ledger,
                "Phase G fuller simulated input/test macro depth")) {
    return 18;
  }
  if (!contains(ledger, "| gpui::actions! / action macro |") ||
      !contains(ledger, "`Action<T>` typed action concept") ||
      !contains(ledger, "`action_name<T>()`") ||
      !contains(ledger, "typed register/dispatch overloads") ||
      !contains(ledger, "action registration metadata") ||
      !contains(ledger, "action enablement metadata") ||
      !contains(ledger, "action bubbling through focused routes") ||
      !contains(ledger, "key binding grammar parsing") ||
      !contains(ledger, "platform modifier semantics") ||
      !contains(ledger, "keymap context filtering") ||
      !contains(ledger, "partial key sequence matching") ||
      !contains(ledger, "disabled key scope filtering") ||
      !contains(ledger, "command palette key integration") ||
      !contains(ledger, "include/cgpui/ui/action.hpp") ||
      !contains(ledger, "include/cgpui/ui/key_binding.hpp") ||
      !contains(ledger, "include/cgpui/ui/runtime_action_templates.hpp") ||
      !contains(ledger,
                "include/cgpui/ui/runtime_action_enablement_templates.hpp") ||
      !contains(ledger, "src/ui/runtime_command_palette_keys.cpp") ||
      !contains(ledger, "src/ui/runtime_action_dispatch.cpp") ||
      !contains(ledger, "src/ui/runtime_action_metadata.cpp") ||
      !contains(ledger, "src/ui/runtime_action_registration.cpp") ||
      !contains(ledger, "src/ui/runtime_key_binding_grammar.cpp") ||
      !contains(ledger, "src/ui/runtime_key_binding_modifiers.cpp") ||
      !contains(ledger, "src/ui/runtime_key_binding_contexts.cpp") ||
      !contains(ledger, "src/ui/runtime_key_binding_sequences.cpp") ||
      !contains(ledger,
                "tests/api_parity/typed_action_surface_test.cpp") ||
      !contains(ledger, "tests/ui/typed_action_dispatch_test.cpp") ||
      !contains(ledger, "tests/ui/action_scope_metadata_test.cpp") ||
      !contains(ledger, "tests/ui/action_enablement_metadata_test.cpp") ||
      !contains(ledger, "tests/ui/action_bubbling_test.cpp") ||
      !contains(ledger, "tests/ui/key_binding_grammar_test.cpp") ||
      !contains(ledger, "tests/ui/key_binding_platform_modifier_test.cpp") ||
      !contains(ledger, "tests/ui/keymap_context_test.cpp") ||
      !contains(ledger, "tests/ui/key_binding_partial_match_test.cpp") ||
      !contains(ledger, "tests/ui/key_binding_disabled_scope_test.cpp") ||
      !contains(ledger,
                "tests/ui/command_palette_key_integration_test.cpp") ||
      !contains(ledger,
                "tests/ui/test_context_keystroke_simulation_test.cpp") ||
      !contains(ledger, "Phase B action metadata/key dispatch depth")) {
    return 62;
  }

  const std::string status_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  if (status_json.empty()) {
    return 7;
  }
  if (const int result = require_contains_all(
          status_json,
          {
              "\"upstream_revision\"",
              "\"5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0\"",
              "\"required\"",
              "\"adapted\"",
              "\"deferred\"",
              "\"non_goal\"",
              "\"examples\"",
              "\"hello_world\"",
              "\"uniform_list\"",
              "\"window_shadow\"",
              "\"gpui::prelude\"",
              "\"gpui::actions! / action macro\"",
              "Action<T> typed action concept",
              "action_name<T>",
              "action enablement metadata",
              "action bubbling through focused routes",
              "key binding grammar parsing",
              "platform modifier semantics",
              "keymap context filtering",
              "partial key sequence matching",
              "disabled key scope filtering",
              "command palette key integration",
              "typed_action_surface_test",
              "action_enablement_metadata_test",
              "action_bubbling_test",
              "key_binding_grammar_test",
              "key_binding_platform_modifier_test",
              "keymap_context_test",
              "key_binding_partial_match_test",
              "key_binding_disabled_scope_test",
              "command_palette_key_integration_test",
              "test_context_keystroke_simulation_test",
              "test_context_pointer_simulation_test",
              "test_context_focus_activation_test",
              "key_binding.hpp",
              "runtime_key_binding_grammar.cpp",
              "runtime_key_binding_modifiers.cpp",
              "runtime_key_binding_contexts.cpp",
              "runtime_key_binding_sequences.cpp",
              "runtime_command_palette_keys.cpp",
              "test_context_keystrokes.cpp",
              "test_context_pointer.cpp",
              "test_context_focus.cpp",
              "test_context_clipboard.cpp",
              "test_context_scheduling.cpp",
              "test_context_rendering.cpp",
              "ElementBuilder::text_size",
              "ElementBuilder::font_family",
              "BoxShadow",
              "Style::box_shadow",
              "StyleOverlay::box_shadow",
              "ElementBuilder::shadow",
              "ElementBuilder::shadow_sm",
              "PaintCommandKind::box_shadow",
              "Style::min_size",
              "Style::max_size",
              "StyleOverlay::min_size",
              "StyleOverlay::max_size",
              "ElementBuilder::min_size",
              "ElementBuilder::max_size",
              "ElementBuilder::min_w",
              "ElementBuilder::min_h",
              "ElementBuilder::max_w",
              "ElementBuilder::max_h",
              "StyledElement::layout",
              "PercentageSize",
              "Style::percentage_size",
              "StyleOverlay::percentage_size",
              "ElementBuilder::size_pct",
              "ElementBuilder::w_pct",
              "ElementBuilder::h_pct",
              "finite external max constraints",
              "ElementBuilder::p",
              "ElementBuilder::px",
              "ElementBuilder::py",
              "ElementBuilder::pt",
              "ElementBuilder::pr",
              "ElementBuilder::pb",
              "ElementBuilder::pl",
              "ElementBuilder::m",
              "ElementBuilder::mx",
              "ElementBuilder::my",
              "ElementBuilder::mt",
              "ElementBuilder::mr",
              "ElementBuilder::mb",
              "ElementBuilder::ml",
              "margin/padding shorthand",
              "Position::fixed",
              "ElementBuilder::fixed",
              "out-of-flow absolute/fixed child layout",
              "FlexElement::layout",
              "direct overlay child ordering",
              "z_order()",
              "paint_ordered_children",
              "hit_test_ordered_children",
              "event_ordered_children",
              "element_layer_ordering.hpp",
              "element_layer_ordering.cpp",
              "element_style_paint.cpp",
              "element_flex_node.cpp",
              "element_vertical_stack_node.cpp",
              "paint_shadow.cpp",
              "effective clip intersection",
              "hidden-overflow plus scrollable-list clip propagation",
              "paint_clip.hpp",
              "paint_clip.cpp",
              "StyleState::active",
              "StyleStateFlags::active",
              "ElementBuilder::active_style",
              "ButtonBuilder::active_style",
              "hover/focus/active/disabled",
              "StyleClassRule",
              "StyleCascade::set_class_rule",
              "StyleCascade::class_rule",
              "src/ui/style_cascade.cpp",
              "depth-first reused class resolution",
              "StyleThemeTokens",
              "Style::with_background_color_token",
              "StyleOverlay::with_background_color_token",
              "theme-aware resolved_style",
              "StyledElement::resolved_style",
              "style_theme_tokens.cpp",
              "style_theme_cascade.cpp",
              "missing-token fallback",
              "StyleAuthoredTextFields",
              "text_style_inheritance.cpp",
              "effective text style",
              "inherited text style",
              "explicit child text-style precedence",
              "Phase C Step 335 dynamic style invalidation",
              "WindowRuntime::request_style_state_invalidation",
              "runtime_style_invalidation.cpp",
              "window_runtime_input_test.cpp",
              "window_runtime_focus_test.cpp",
              "Phase C Step 336 style cascade depth closeout",
              "phase_c_style_cascade_depth_audit_test.cpp",
              "Phase C Step 337 pointer-active input semantics",
              "ViewInputState::active_element_id",
              "WindowRuntime::update_active_state_for_event",
              "runtime_active_state.cpp",
              "Phase C Step 338 tab-order/focus-ring metadata",
              "FocusMetadata",
              "FocusRingVisibility",
              "ElementBuilder::tab_index",
              "ElementBuilder::focus_ring",
              "AccessibilityNode::tab_index",
              "WindowRuntime::ordered_focusable_element_ids",
              "runtime_focus_order.cpp",
              "Phase C Step 339 click/drag gesture synthesis",
              "ElementGestureKind::click",
              "ElementEventContext::gesture",
              "ViewInputState::pointer_down_element_id",
              "ViewInputState::clicked_element_id",
              "ViewInputState::dragging_element_id",
              "ViewInputState::dragging",
              "runtime_gesture_synthesis.cpp",
              "dispatch_synthesized_click_event",
              "test_runtime_synthesizes_click_on_release_over_pressed_element",
              "test_runtime_suppresses_click_after_drag_gesture",
              "Phase C Step 340 keyboard activation semantics",
              "should_dispatch_synthesized_keyboard_activation_event",
              "KeyElement synthesized-click delegation",
              "test_runtime_enter_space_activate_focused_button",
              "test_runtime_raw_key_handler_controls_keyboard_activation",
              "Phase C Step 341 disabled interaction semantics",
              "WindowRuntime::refresh_disabled_interaction_state",
              "runtime_disabled_interaction.cpp",
              "test_runtime_clears_disabled_interaction_state",
              "Phase C Step 342 focusable/interactable band closeout",
              "phase_c_focusable_interactable_audit_test.cpp",
              "Phase C Step 343 built-in widget expansion",
              "ButtonBuilder::label",
              "src/ui/widgets/button_builder.cpp",
              "src/ui/widgets/label_builder.cpp",
              "src/ui/widgets/text_input_builder.cpp",
              "builtin_widget_test.cpp",
              "widget_source_structure_test.cpp",
              "Phase C Step 344 checkbox/radio/switch widgets",
              "ToggleBuilder",
              "checkbox(...)",
              "radio(...)",
              "toggle_switch(...)",
              "ToggleControlElement",
              "ToggleControlKind",
              "AccessibilityNode::value",
              "PlatformAccessibilityRole::checkbox",
              "src/ui/widgets/toggle_builder.cpp",
              "src/ui/element_choice_nodes.cpp",
              "src/ui/element_choice_layout.cpp",
              "src/ui/element_choice_paint.cpp",
              "Phase C Step 345 slider widget",
              "SliderBuilder",
              "slider(...)",
              "SliderElement",
              "AccessibilityRole::slider",
              "PlatformAccessibilityRole::slider",
              "src/ui/widgets/slider_builder.cpp",
              "src/ui/element_slider_nodes.cpp",
              "src/ui/element_slider_layout.cpp",
              "src/ui/element_slider_paint.cpp",
              "Phase C Step 346 list/menu widget follow-up",
              "ItemBuilder",
              "list_item(...)",
              "menu_item(...)",
              "ItemElement",
              "AccessibilityRole::list_item",
              "AccessibilityRole::menu_item",
              "PlatformAccessibilityRole::list_item",
              "PlatformAccessibilityRole::menu_item",
              "src/ui/widgets/item_builder.cpp",
              "src/ui/element_item_nodes.cpp",
              "src/ui/element_item_layout.cpp",
              "src/ui/element_item_paint.cpp",
              "Phase C Step 347 icon/image widget follow-up",
              "ImageBuilder",
              "image(...)",
              "icon(...)",
              "svg(...)",
              "ImageSource",
              "ImageSourceKind",
              "image_source(...)",
              "svg_image_source(...)",
              "ImageElement",
              "ImageElementKind",
              "AccessibilityRole::image",
              "PlatformAccessibilityRole::image",
              "ImagePaint",
              "ImageDraw",
              "src/ui/widgets/image_builder.cpp",
              "src/ui/element_image_nodes.cpp",
              "src/ui/element_image_layout.cpp",
              "src/ui/element_image_paint.cpp",
              "Phase C Step 348 container primitive follow-up",
              "container_builder.hpp",
              "src/ui/widgets/container_builder.cpp",
              "div()",
              "h_flex()",
              "v_flex()",
              "h_stack()",
              "v_stack()",
              "Phase C Step 349 uniform list parity",
              "UniformListVisibleRange",
              "UniformListItemIdentity",
              "UniformListItemMeasurement",
              "UniformListItemMeasurementResult",
              "UniformListRecyclingWindow",
              "UniformListSelectionSource",
              "UniformListSelectionDirection",
              "UniformListSelection",
              "UniformListSelectionState",
              "UniformListLayoutSnapshot",
              "UniformListScrollAnchor",
              "UniformListItemMeasurementCache",
              "calculate_uniform_list_visible_range",
              "measure_uniform_list_items",
              "calculate_uniform_list_recycling_window",
              "select_uniform_list_item_at_point",
              "move_uniform_list_selection",
              "capture_uniform_list_scroll_anchor",
              "apply_uniform_list_scroll_anchor",
              "ScrollableListElement::layout_snapshot",
              "ScrollableListElement::measurement_cache",
              "ScrollableListElement::selection",
              "src/ui/uniform_list.cpp",
              "src/ui/uniform_list_measurement.cpp",
              "src/ui/uniform_list_recycling.cpp",
              "src/ui/uniform_list_selection.cpp",
              "src/ui/element_scroll_layout.cpp",
              "src/ui/element_scroll_events.cpp",
              "Phase C Step 350 scroll anchoring",
              "Phase C Step 351 item measurement cache",
              "Phase C Step 352 large-list recycling",
              "Phase C Step 353 keyboard/pointer selection",
              "Phase C Step 354 uniform list closeout",
              "Phase C Step 355 window/examples widgets",
              "Phase C Step 356 window/examples workflow",
              "Phase C Step 357 window/examples widget catalog",
              "Phase C Step 358 window/examples interaction states",
              "Phase C Step 359 window/examples service matrix",
              "phase_c_uniform_list_audit_test.cpp",
              "phase_c_window_examples_public_api_test.cpp",
              "phase_c_window_examples_workflow_test.cpp",
              "phase_c_window_examples_widget_catalog_test.cpp",
              "phase_c_window_examples_interaction_states_test.cpp",
              "phase_c_window_examples_service_matrix_test.cpp",
              "phase_c_window_examples_closeout_test.cpp",
              "public_window_examples_workflow/main.cpp",
              "public_window_examples_widget_catalog/main.cpp",
              "public_window_examples_interaction_states/main.cpp",
              "public_window_examples_service_matrix/main.cpp",
              "Phase C Step 360 window/examples closeout",
              "Phase C Step 361 SVG/image element front-end APIs",
              "Phase C Step 361 SVG/image front-end source APIs",
              "Phase C Step 362 SVG/image asset registration",
              "Phase C Step 363 SVG/image public example coverage",
              "ImageAssetRegistry",
              "RegisteredImageAsset",
              "api_parity_public_window_examples_workflow",
              "api_parity_public_window_examples_widget_catalog",
              "api_parity_public_window_examples_interaction_states",
              "api_parity_public_window_examples_service_matrix",
              "runtime_clipboard.cpp",
              "dispatch_pointer_* helpers",
              "dispatch_window_* helpers",
              "write_to_clipboard",
              "read_from_clipboard",
              "copy/cut/paste helpers",
              "test_context_clipboard_test",
              "run_until_parked",
              "advance_time_until_parked",
              "request_redraw",
              "try_draw_frame",
              "draw_frame",
              "test_context_time_async_test",
              "test_context_frame_pump_test",
              "runtime_action_enablement_templates.hpp",
              "runtime_action_dispatch.cpp",
              "runtime_action_registration.cpp",
              "WindowContextCapability",
              "window_context",
              "AsyncContextCapability",
              "async_context",
              "try_open_window",
              "Result<AppOpenedWindow>",
              "try_create_renderer",
              "Result<Renderer*>",
              "runtime_renderer_results.cpp",
              "renderer_result_conventions_test",
              "try_resize_surface",
              "Result<void>",
              "runtime_renderer_resize_results.cpp",
              "renderer_resize_result_conventions_test",
              "runtime_renderer_frame_results.cpp",
              "renderer_frame_result_conventions_test",
              "public_api_compatibility_examples_test",
              "examples/api_parity/public_api_compatibility/main.cpp",
              "public_api_example_expansion_test",
              "examples/api_parity/public_authoring_workflow/main.cpp",
              "public_context_capability_example_test",
              "examples/api_parity/public_context_capabilities/main.cpp",
              "public_async_test_workflow_example_test",
              "examples/api_parity/public_async_test_workflow/main.cpp",
              "public_phase_b_surface_closure_example_test",
              "examples/api_parity/public_phase_b_surface_closure/main.cpp",
              "public_authoring_vocabulary_freeze_test",
              "docs/gpui-public-authoring-vocabulary.md",
              "public_phase_b_completion_audit_test",
              "final Phase B public vocabulary completion audit",
              "try_install_native_menu",
              "Result<NativeMenuInstallation>",
              "try_show_native_file_dialog",
              "Result<NativeFileDialogResult>",
              "runtime_platform_service_results.cpp",
              "platform_service_result_conventions_test",
              "try_spawn_task",
              "Result<TaskHandle>",
              "try_spawn_background_task",
              "runtime_task_results.cpp",
              "async_spawn_result_conventions_test",
              "TestContextCapability",
              "test_context",
              "gpui::TestAppContext",
              "deterministic subscription lifetime/unsubscribe behavior",
              "deterministic Subscription lifetime/unsubscribe behavior",
              "entity-to-entity observation helpers",
              "window/view observation helpers",
              "invalidate_entity helpers",
              "entity deletion helpers",
              "\"x11\"",
          },
          8);
      result != 0) {
    return result;
  }

  const std::string tool_readme = read_source("tools/gpui_parity/README.md");
  const std::string extractor =
      read_source("tools/gpui_parity/extract_upstream_symbols.py");
  if (tool_readme.empty() || extractor.empty()) {
    return 9;
  }
  if (!contains(tool_readme, "extract_upstream_symbols.py") ||
      !contains(tool_readme,
                "docs/gpui-complete-parity-ledger.json") ||
      !contains(extractor, "GPUI_UPSTREAM_REVISION") ||
      !contains(extractor, "extract_public_reexports") ||
      !contains(extractor, "extract_examples")) {
    return 10;
  }

  const std::string plan = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-upstream-parity-ledger-plan.md");
  if (plan.empty() ||
      !contains(plan, "Steps 219-258") ||
      !contains(plan, "Run RED") ||
      !contains(plan, "Run GREEN") ||
      !contains(plan, "Phase A complete")) {
    return 11;
  }

  const std::string example =
      read_source("examples/api_parity/hello_world/main.cpp");
  if (example.empty()) {
    return 12;
  }
  if (!contains(example, "class HelloWorldView") ||
      !contains(example, "cgpui::Context<HelloWorldView>&") ||
      !contains(example, "cgpui::IntoElement render") ||
      !contains(example, "cgpui::Render<HelloWorldView>") ||
      !contains(example, "cgpui::div()") ||
      !contains(example, "cgpui::Application::create") ||
      !contains(example, "app->run") ||
      !contains(example, "GPUI upstream hello_world.rs parity")) {
    return 13;
  }

  const std::string public_api_example =
      read_source("examples/api_parity/public_api_compatibility/main.cpp");
  if (public_api_example.empty()) {
    return 80;
  }
  if (!contains(public_api_example, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_api_example, "class PublicApiCompatibilityView") ||
      !contains(public_api_example,
                "cgpui::Context<PublicApiCompatibilityView>&") ||
      !contains(public_api_example, "cgpui::IntoElement render") ||
      !contains(public_api_example,
                "static_assert(cgpui::Render<PublicApiCompatibilityView>)") ||
      !contains(public_api_example, "cgpui::Application::create()") ||
      !contains(public_api_example, "try_open_window(") ||
      !contains(public_api_example, "try_install_native_menu(") ||
      !contains(public_api_example, "try_show_native_file_dialog(") ||
      !contains(public_api_example, "try_spawn_task(") ||
      !contains(public_api_example, "try_draw_frame(") ||
      !contains(public_api_example, "register_action(") ||
      !contains(public_api_example, "bind_key(")) {
    return 81;
  }
  if (contains(public_api_example, "WindowRuntime") ||
      contains(public_api_example, "#include \"cgpui/ui/") ||
      contains(public_api_example, "#include \"cgpui/platform/") ||
      contains(public_api_example, "#include \"cgpui/renderer/") ||
      contains(public_api_example, "#include \"src/") ||
      contains(public_api_example, ".runtime") ||
      contains(public_api_example, "runtime.")) {
    return 82;
  }

  const std::string public_workflow_example =
      read_source("examples/api_parity/public_authoring_workflow/main.cpp");
  if (public_workflow_example.empty()) {
    return 85;
  }
  if (!contains(public_workflow_example, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_workflow_example,
                "class PublicAuthoringWorkflowView") ||
      !contains(public_workflow_example,
                "cgpui::Context<PublicAuthoringWorkflowView>&") ||
      !contains(public_workflow_example, "cgpui::IntoElement render") ||
      !contains(public_workflow_example,
                "static_assert(cgpui::Render<PublicAuthoringWorkflowView>)") ||
      !contains(public_workflow_example, "cgpui::Application::create()") ||
      !contains(public_workflow_example, "cgpui::EntityHandle<") ||
      !contains(public_workflow_example, "cgpui::WeakEntity<") ||
      !contains(public_workflow_example, "cgpui::ViewHandle<") ||
      !contains(public_workflow_example, "cgpui::WeakViewHandle<") ||
      !contains(public_workflow_example, "cgpui::CommandPaletteEntry") ||
      !contains(public_workflow_example, "cgpui::KeyBindingContext") ||
      !contains(public_workflow_example, "cgpui::AsyncContextCapability") ||
      !contains(public_workflow_example, "cgpui::TestContextCapability") ||
      !contains(public_workflow_example, "cgpui::NativeMenuModel") ||
      !contains(public_workflow_example, "cgpui::NativeFileDialogOptions") ||
      !contains(public_workflow_example, "try_install_native_menu(") ||
      !contains(public_workflow_example, "try_show_native_file_dialog(") ||
      !contains(public_workflow_example, "try_spawn_task(") ||
      !contains(public_workflow_example, "try_draw_frame(") ||
      !contains(public_workflow_example, "bind_key(")) {
    return 86;
  }
  if (contains(public_workflow_example, "WindowRuntime") ||
      contains(public_workflow_example, "#include \"cgpui/ui/") ||
      contains(public_workflow_example, "#include \"cgpui/platform/") ||
      contains(public_workflow_example, "#include \"cgpui/renderer/") ||
      contains(public_workflow_example, "#include \"src/") ||
      contains(public_workflow_example, "ClipboardItem") ||
      contains(public_workflow_example, "gpui::test") ||
      contains(public_workflow_example, "TaskPriority") ||
      contains(public_workflow_example, "StructuredTaskGroup") ||
      contains(public_workflow_example, ".runtime") ||
      contains(public_workflow_example, "runtime.") ||
      contains(public_workflow_example, "window_runtime")) {
    return 87;
  }

  const std::string public_context_example =
      read_source("examples/api_parity/public_context_capabilities/main.cpp");
  if (public_context_example.empty()) {
    return 88;
  }
  if (!contains(public_context_example, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_context_example,
                "class PublicContextCapabilitiesView") ||
      !contains(public_context_example,
                "cgpui::Context<PublicContextCapabilitiesView>&") ||
      !contains(public_context_example, "cgpui::IntoElement render") ||
      !contains(public_context_example,
                "static_assert(cgpui::Render<PublicContextCapabilitiesView>)") ||
      !contains(public_context_example, "cgpui::AppContext") ||
      !contains(public_context_example,
                "cgpui::ViewContextCapability<PublicContextCapabilitiesView>") ||
      !contains(public_context_example,
                "cgpui::WindowContextCapability") ||
      !contains(public_context_example,
                "cgpui::ElementContextCapability") ||
      !contains(public_context_example, "app_context()") ||
      !contains(public_context_example,
                "view_context<PublicContextCapabilitiesView>()") ||
      !contains(public_context_example, "window_context()") ||
      !contains(public_context_example, "element_context(") ||
      !contains(public_context_example, "set_global(") ||
      !contains(public_context_example, "update_global<") ||
      !contains(public_context_example, "set_app_theme(") ||
      !contains(public_context_example, "view_context.view()") ||
      !contains(public_context_example, "view_context.weak_view()") ||
      !contains(public_context_example, "window_context.window()") ||
      !contains(public_context_example, "window_context.request_render()") ||
      !contains(public_context_example, "element_context.focus()") ||
      !contains(public_context_example, "element_context.capture_pointer()") ||
      !contains(public_context_example, "element_context.set_cursor(") ||
      !contains(public_context_example, "element_context.state_or_init<") ||
      !contains(public_context_example, "cgpui::Application::create()")) {
    return 89;
  }
  if (contains(public_context_example, "WindowRuntime") ||
      contains(public_context_example, "#include \"cgpui/ui/") ||
      contains(public_context_example, "#include \"cgpui/platform/") ||
      contains(public_context_example, "#include \"cgpui/renderer/") ||
      contains(public_context_example, "#include \"src/") ||
      contains(public_context_example, "ClipboardItem") ||
      contains(public_context_example, "gpui::test") ||
      contains(public_context_example, "TaskPriority") ||
      contains(public_context_example, "StructuredTaskGroup") ||
      contains(public_context_example, ".runtime") ||
      contains(public_context_example, "runtime.") ||
      contains(public_context_example, "window_runtime")) {
    return 90;
  }

  const std::string public_async_test_example =
      read_source("examples/api_parity/public_async_test_workflow/main.cpp");
  if (public_async_test_example.empty()) {
    return 91;
  }
  if (!contains(public_async_test_example, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_async_test_example,
                "class PublicAsyncTestWorkflowView") ||
      !contains(public_async_test_example,
                "cgpui::Context<PublicAsyncTestWorkflowView>&") ||
      !contains(public_async_test_example, "cgpui::IntoElement render") ||
      !contains(public_async_test_example,
                "static_assert(cgpui::Render<PublicAsyncTestWorkflowView>)") ||
      !contains(public_async_test_example,
                "const cgpui::AsyncContextCapability async = "
                "context.async_context()") ||
      !contains(public_async_test_example,
                "const cgpui::TestContextCapability test = "
                "context.test_context()") ||
      !contains(public_async_test_example, "async.defer(") ||
      !contains(public_async_test_example, "async.schedule_timer(") ||
      !contains(public_async_test_example,
                "async.schedule_repeating_timer(") ||
      !contains(public_async_test_example, "async.try_spawn_task(") ||
      !contains(public_async_test_example,
                "async.try_spawn_background_task(") ||
      !contains(public_async_test_example, "async.batch_updates(") ||
      !contains(public_async_test_example, "test.run_until_parked()") ||
      !contains(public_async_test_example,
                "test.advance_time_until_parked(") ||
      !contains(public_async_test_example, "test.request_redraw()") ||
      !contains(public_async_test_example, "test.try_draw_frame()") ||
      !contains(public_async_test_example, "test.dispatch_keystroke(") ||
      !contains(public_async_test_example, "test.simulate_keystrokes(") ||
      !contains(public_async_test_example, "test.dispatch_pointer_move(") ||
      !contains(public_async_test_example, "test.dispatch_pointer_button(") ||
      !contains(public_async_test_example, "test.dispatch_window_focus(") ||
      !contains(public_async_test_example, "test.write_to_clipboard(") ||
      !contains(public_async_test_example, "test.read_from_clipboard()") ||
      !contains(public_async_test_example, "cgpui::Application::create()")) {
    return 92;
  }
  if (contains(public_async_test_example, "WindowRuntime") ||
      contains(public_async_test_example, "#include \"cgpui/ui/") ||
      contains(public_async_test_example, "#include \"cgpui/platform/") ||
      contains(public_async_test_example, "#include \"cgpui/renderer/") ||
      contains(public_async_test_example, "#include \"src/") ||
      contains(public_async_test_example, "ClipboardItem") ||
      contains(public_async_test_example, "gpui::test") ||
      contains(public_async_test_example, "TaskPriority") ||
      contains(public_async_test_example, "StructuredTaskGroup") ||
      contains(public_async_test_example, ".runtime") ||
      contains(public_async_test_example, "runtime.") ||
      contains(public_async_test_example, "window_runtime")) {
    return 93;
  }

  const std::string public_phase_b_example =
      read_source("examples/api_parity/public_phase_b_surface_closure/main.cpp");
  if (public_phase_b_example.empty()) {
    return 94;
  }
  if (!contains(public_phase_b_example, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_phase_b_example,
                "class PublicPhaseBSurfaceClosureView") ||
      !contains(public_phase_b_example,
                "cgpui::Context<PublicPhaseBSurfaceClosureView>&") ||
      !contains(public_phase_b_example, "cgpui::IntoElement render") ||
      !contains(public_phase_b_example,
                "static_assert(cgpui::Render<PublicPhaseBSurfaceClosureView>)") ||
      !contains(public_phase_b_example, "cgpui::Application::create()") ||
      !contains(public_phase_b_example, "cgpui::AppRunnerOptions") ||
      !contains(public_phase_b_example, "cgpui::WindowOptions") ||
      !contains(public_phase_b_example, "cgpui::AppContext") ||
      !contains(public_phase_b_example,
                "cgpui::ViewContextCapability<PublicPhaseBSurfaceClosureView>") ||
      !contains(public_phase_b_example, "cgpui::WindowContextCapability") ||
      !contains(public_phase_b_example, "cgpui::ElementContextCapability") ||
      !contains(public_phase_b_example, "cgpui::EntityHandle<") ||
      !contains(public_phase_b_example, "cgpui::WeakEntity<") ||
      !contains(public_phase_b_example, "cgpui::ViewHandle<") ||
      !contains(public_phase_b_example, "cgpui::WeakViewHandle<") ||
      !contains(public_phase_b_example, "cgpui::Action<") ||
      !contains(public_phase_b_example, "cgpui::CommandPaletteEntry") ||
      !contains(public_phase_b_example, "cgpui::KeyBindingContext") ||
      !contains(public_phase_b_example, "cgpui::Result<") ||
      !contains(public_phase_b_example, "cgpui::ErrorCode") ||
      !contains(public_phase_b_example, "cgpui::AsyncContextCapability") ||
      !contains(public_phase_b_example, "cgpui::TestContextCapability") ||
      !contains(public_phase_b_example, "cgpui::NativeMenuModel") ||
      !contains(public_phase_b_example, "cgpui::NativeFileDialogOptions") ||
      !contains(public_phase_b_example, "try_open_window(") ||
      !contains(public_phase_b_example, "try_install_native_menu(") ||
      !contains(public_phase_b_example, "try_show_native_file_dialog(") ||
      !contains(public_phase_b_example, "try_spawn_task(") ||
      !contains(public_phase_b_example, "try_spawn_background_task(") ||
      !contains(public_phase_b_example, "try_draw_frame()") ||
      !contains(public_phase_b_example, "bind_key(") ||
      !contains(public_phase_b_example,
                "register_command_palette_entry<") ||
      !contains(public_phase_b_example, "capture_pointer(") ||
      !contains(public_phase_b_example, "release_pointer(")) {
    return 95;
  }
  if (contains(public_phase_b_example, "WindowRuntime") ||
      contains(public_phase_b_example, "#include \"cgpui/ui/") ||
      contains(public_phase_b_example, "#include \"cgpui/platform/") ||
      contains(public_phase_b_example, "#include \"cgpui/renderer/") ||
      contains(public_phase_b_example, "#include \"src/") ||
      contains(public_phase_b_example, "ClipboardItem") ||
      contains(public_phase_b_example, "gpui::test") ||
      contains(public_phase_b_example, "TaskPriority") ||
      contains(public_phase_b_example, "StructuredTaskGroup") ||
      contains(public_phase_b_example, ".runtime") ||
      contains(public_phase_b_example, "runtime.") ||
      contains(public_phase_b_example, "window_runtime")) {
    return 96;
  }

  const std::string public_vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  if (public_vocabulary.empty()) {
    return 83;
  }
  if (!contains(public_vocabulary,
                "# Public Authoring Vocabulary Freeze") ||
      !contains(public_vocabulary, "Phase B Step 308") ||
      !contains(public_vocabulary,
                "The frozen include boundary is `#include \"cgpui/prelude.hpp\"`.") ||
      !contains(public_vocabulary, "`Context<T>`") ||
      !contains(public_vocabulary, "`Action<T>`") ||
      !contains(public_vocabulary, "`Result<T>`") ||
      !contains(public_vocabulary, "`AsyncContextCapability`") ||
      !contains(public_vocabulary, "`TestContextCapability`") ||
      !contains(public_vocabulary,
                "`try_spawn_task(...) -> Result<TaskHandle>`") ||
      !contains(public_vocabulary,
                "`try_draw_frame() -> Result<void>`") ||
      !contains(public_vocabulary, "`NativeMenuModel`") ||
      !contains(public_vocabulary, "`NativeFileDialogOptions`") ||
      !contains(public_vocabulary, "`ClipboardItem` payload parity") ||
      !contains(public_vocabulary,
                "upstream `gpui::test` macro equivalents")) {
    return 84;
  }

  const std::string public_authoring =
      read_source("tests/api_parity/public_authoring_surface_test.cpp");
  if (public_authoring.empty()) {
    return 17;
  }
  if (!contains(public_authoring, "#include \"cgpui/prelude.hpp\"") ||
      !contains(public_authoring, "cgpui::Context<PublicAuthoringSurfaceView>&") ||
      !contains(public_authoring, "cgpui::IntoElement render") ||
      !contains(public_authoring, "cgpui::Render<PublicAuthoringSurfaceView>") ||
      !contains(public_authoring, "cgpui::ViewHandle<PublicAuthoringSurfaceView>") ||
      !contains(public_authoring, "cgpui::Application")) {
    return 18;
  }
  if (contains(public_authoring, "WindowRuntimeContext") ||
      contains(public_authoring, "WindowRuntime") ||
      contains(public_authoring, "AppContext") ||
      contains(public_authoring, "ViewContext") ||
      contains(public_authoring, "PlatformWindow")) {
    return 19;
  }

  const std::string entity_lifecycle =
      read_source("tests/api_parity/entity_lifecycle_creation_test.cpp");
  if (entity_lifecycle.empty()) {
    return 20;
  }
  if (!contains(entity_lifecycle, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_lifecycle, "context.new_entity<LifecycleState>") ||
      !contains(entity_lifecycle, "cgpui::EntityHandle<LifecycleState>") ||
      !contains(entity_lifecycle, "entity.update(context") ||
      !contains(entity_lifecycle, "cgpui::Render<EntityLifecycleCreationView>")) {
    return 21;
  }
  if (contains(entity_lifecycle, "WindowRuntimeContext") ||
      contains(entity_lifecycle, "WindowRuntime") ||
      contains(entity_lifecycle, "AppContext") ||
      contains(entity_lifecycle, "ViewContext") ||
      contains(entity_lifecycle, "PlatformWindow")) {
    return 22;
  }

  const std::string entity_weak_handles =
      read_source("tests/api_parity/entity_weak_handle_semantics_test.cpp");
  if (entity_weak_handles.empty()) {
    return 23;
  }
  if (!contains(entity_weak_handles, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_weak_handles, "weak.upgrade(context)") ||
      !contains(entity_weak_handles, "weak.read(context)") ||
      !contains(entity_weak_handles,
                "std::optional<cgpui::EntityHandle<WeakLifecycleState>>") ||
      !contains(entity_weak_handles,
                "cgpui::Render<EntityWeakHandleSemanticsView>")) {
    return 24;
  }
  if (contains(entity_weak_handles, "WindowRuntimeContext") ||
      contains(entity_weak_handles, "WindowRuntime") ||
      contains(entity_weak_handles, "AppContext") ||
      contains(entity_weak_handles, "ViewContext") ||
      contains(entity_weak_handles, "PlatformWindow")) {
    return 25;
  }

  const std::string entity_observation =
      read_source("tests/api_parity/entity_observation_test.cpp");
  if (entity_observation.empty()) {
    return 26;
  }
  if (!contains(entity_observation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_observation, "entity.observe(") ||
      !contains(entity_observation, "entity.observe_subscription(") ||
      !contains(entity_observation, "context.observe_entity(") ||
      !contains(entity_observation, "context.observe_entity_subscription") ||
      !contains(entity_observation,
                "cgpui::EntityHandle<ObservedState>") ||
      !contains(entity_observation,
                "cgpui::Render<EntityObservationView>")) {
    return 27;
  }
  if (contains(entity_observation, "WindowRuntimeContext") ||
      contains(entity_observation, "WindowRuntime") ||
      contains(entity_observation, "AppContext") ||
      contains(entity_observation, "ViewContext") ||
      contains(entity_observation, "PlatformWindow")) {
    return 28;
  }

  const std::string entity_to_entity_observation =
      read_source("tests/api_parity/entity_to_entity_observation_test.cpp");
  if (entity_to_entity_observation.empty()) {
    return 53;
  }
  if (!contains(entity_to_entity_observation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_to_entity_observation, "observer.observe_entity(") ||
      !contains(entity_to_entity_observation,
                "context.observe_entity_subscription") ||
      !contains(entity_to_entity_observation, "ObserverState& owner") ||
      !contains(entity_to_entity_observation,
                "cgpui::EntityHandle<ObservedState>") ||
      !contains(entity_to_entity_observation,
                "cgpui::Render<EntityToEntityObservationView>")) {
    return 54;
  }
  if (contains(entity_to_entity_observation, "WindowRuntimeContext") ||
      contains(entity_to_entity_observation, "WindowRuntime") ||
      contains(entity_to_entity_observation, "AppContext") ||
      contains(entity_to_entity_observation, "ViewContext") ||
      contains(entity_to_entity_observation, "PlatformWindow")) {
    return 55;
  }

  const std::string window_view_observation =
      read_source("tests/api_parity/window_view_observation_test.cpp");
  if (window_view_observation.empty()) {
    return 56;
  }
  if (!contains(window_view_observation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(window_view_observation, "context.observe_window(") ||
      !contains(window_view_observation,
                "observe_window_subscription") ||
      !contains(window_view_observation, "window_context.observe") ||
      !contains(window_view_observation, "view.observe(") ||
      !contains(window_view_observation, "view_context.observe") ||
      !contains(window_view_observation,
                "cgpui::Render<WindowViewObservationView>")) {
    return 57;
  }
  if (contains(window_view_observation, "WindowRuntimeContext") ||
      contains(window_view_observation, "WindowRuntime") ||
      contains(window_view_observation, "AppContext") ||
      contains(window_view_observation, "PlatformWindow")) {
    return 58;
  }

  const std::string subscription_lifetime =
      read_source("tests/api_parity/subscription_lifetime_test.cpp");
  if (subscription_lifetime.empty()) {
    return 50;
  }
  if (!contains(subscription_lifetime, "#include \"cgpui/prelude.hpp\"") ||
      !contains(subscription_lifetime, "subscription.release()") ||
      !contains(subscription_lifetime, "context.observe_entity_subscription") ||
      !contains(subscription_lifetime, "std::move(moved_from)") ||
      !contains(subscription_lifetime,
                "cgpui::Render<SubscriptionLifetimeView>")) {
    return 51;
  }
  if (contains(subscription_lifetime, "WindowRuntimeContext") ||
      contains(subscription_lifetime, "WindowRuntime") ||
      contains(subscription_lifetime, "AppContext") ||
      contains(subscription_lifetime, "ViewContext") ||
      contains(subscription_lifetime, "PlatformWindow")) {
    return 52;
  }

  const std::string observation_closure =
      read_source("tests/ui/window_runtime_observation_closure_test.cpp");
  if (observation_closure.empty()) {
    return 59;
  }
  if (!contains(observation_closure, "runtime.remove_view(child_view_id)") ||
      !contains(observation_closure,
                "runtime.subscriptions_for_view(child_view_id).empty()") ||
      !contains(observation_closure, "RuntimeDiagnosticsSnapshot") ||
      !contains(observation_closure, "view_subscription.release()")) {
    return 60;
  }
  if (contains(observation_closure, "Action") ||
      contains(observation_closure, "KeyBinding") ||
      contains(observation_closure, "key_dispatch")) {
    return 61;
  }

  const std::string entity_transaction =
      read_source("tests/api_parity/entity_transaction_test.cpp");
  if (entity_transaction.empty()) {
    return 29;
  }
  if (!contains(entity_transaction, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_transaction, "entity.update(") ||
      !contains(entity_transaction, "context.update_entity(") ||
      !contains(entity_transaction, "std::optional<int>") ||
      !contains(entity_transaction,
                "const cgpui::Context<TransactionState>&") ||
      !contains(entity_transaction,
                "cgpui::Render<EntityUpdateTransactionView>")) {
    return 30;
  }
  if (contains(entity_transaction, "WindowRuntimeContext") ||
      contains(entity_transaction, "WindowRuntime") ||
      contains(entity_transaction, "AppContext") ||
      contains(entity_transaction, "ViewContext") ||
      contains(entity_transaction, "PlatformWindow")) {
    return 31;
  }

  const std::string entity_invalidation =
      read_source("tests/api_parity/entity_invalidation_test.cpp");
  if (entity_invalidation.empty()) {
    return 32;
  }
  if (!contains(entity_invalidation, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_invalidation, "entity.invalidate(context)") ||
      !contains(entity_invalidation, "context.invalidate_entity(entity)") ||
      !contains(entity_invalidation,
                "cgpui::EntityHandle<InvalidationStateModel>") ||
      !contains(entity_invalidation, "std::same_as") ||
      !contains(entity_invalidation,
                "cgpui::Render<EntityInvalidationView>")) {
    return 33;
  }
  if (contains(entity_invalidation, "WindowRuntimeContext") ||
      contains(entity_invalidation, "WindowRuntime") ||
      contains(entity_invalidation, "AppContext") ||
      contains(entity_invalidation, "ViewContext") ||
      contains(entity_invalidation, "PlatformWindow")) {
    return 34;
  }

  const std::string entity_deletion =
      read_source("tests/api_parity/entity_deletion_test.cpp");
  if (entity_deletion.empty()) {
    return 35;
  }
  if (!contains(entity_deletion, "#include \"cgpui/prelude.hpp\"") ||
      !contains(entity_deletion, "entity.remove(context)") ||
      !contains(entity_deletion, "context.remove_entity(context_entity)") ||
      !contains(entity_deletion, "missing.remove(context)") ||
      !contains(entity_deletion,
                "cgpui::EntityHandle<DeletionState>") ||
      !contains(entity_deletion, "std::same_as") ||
      !contains(entity_deletion,
                "cgpui::Render<EntityDeletionView>")) {
    return 36;
  }
  if (contains(entity_deletion, "WindowRuntimeContext") ||
      contains(entity_deletion, "WindowRuntime") ||
      contains(entity_deletion, "AppContext") ||
      contains(entity_deletion, "ViewContext") ||
      contains(entity_deletion, "PlatformWindow")) {
    return 37;
  }

  const std::string app_context_capability =
      read_source("tests/api_parity/app_context_capability_test.cpp");
  if (app_context_capability.empty()) {
    return 38;
  }
  if (!contains(app_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(app_context_capability, "context.app_context()") ||
      !contains(app_context_capability, "cgpui::AppContext") ||
      !contains(app_context_capability, "app_context.app()") ||
      !contains(app_context_capability,
                "app_context.global<CapabilityGlobal>()") ||
      !contains(app_context_capability, "std::same_as") ||
      !contains(app_context_capability,
               "cgpui::Render<AppContextCapabilityView>")) {
    return 39;
  }
  if (contains(app_context_capability, "WindowRuntimeContext") ||
      contains(app_context_capability, "WindowRuntime") ||
      contains(app_context_capability, "ViewContext") ||
      contains(app_context_capability, "PlatformWindow")) {
    return 40;
  }

  const std::string view_context_capability =
      read_source("tests/api_parity/view_context_capability_test.cpp");
  if (view_context_capability.empty()) {
    return 41;
  }
  if (!contains(view_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(view_context_capability, "context.view_context") ||
      !contains(view_context_capability, "cgpui::ViewContextCapability") ||
      !contains(view_context_capability, "view_context.view()") ||
      !contains(view_context_capability, "view_context.weak_view()") ||
      !contains(view_context_capability, "view_context.current()") ||
      !contains(view_context_capability, "std::same_as") ||
      !contains(view_context_capability,
                "cgpui::Render<ViewContextCapabilityView>")) {
    return 42;
  }
  if (contains(view_context_capability, "WindowRuntimeContext") ||
      contains(view_context_capability, "WindowRuntime") ||
      contains(view_context_capability, "AppContext") ||
      contains(view_context_capability, "PlatformWindow")) {
    return 43;
  }

  const std::string window_context_capability =
      read_source("tests/api_parity/window_context_capability_test.cpp");
  if (window_context_capability.empty()) {
    return 44;
  }
  if (!contains(window_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(window_context_capability, "context.window_context()") ||
      !contains(window_context_capability, "cgpui::WindowContextCapability") ||
      !contains(window_context_capability, "window_context.window()") ||
      !contains(window_context_capability,
                "window_context.current_window()") ||
      !contains(window_context_capability, "window_context.runtime_id()") ||
      !contains(window_context_capability, "window_context.request_render()") ||
      !contains(window_context_capability, "std::same_as") ||
      !contains(window_context_capability,
                "cgpui::Render<WindowContextCapabilityView>")) {
    return 45;
  }
  if (contains(window_context_capability, "WindowRuntimeContext") ||
      contains(window_context_capability, "PlatformWindow")) {
    return 46;
  }

  const std::string element_context_capability =
      read_source("tests/api_parity/element_context_capability_test.cpp");
  if (element_context_capability.empty()) {
    return 47;
  }
  if (!contains(element_context_capability, "#include \"cgpui/prelude.hpp\"") ||
      !contains(element_context_capability, "context.element_context") ||
      !contains(element_context_capability, "cgpui::ElementContextCapability") ||
      !contains(element_context_capability, "element_context.element_id()") ||
      !contains(element_context_capability,
                "element_context.request_keyboard_focus()") ||
      !contains(element_context_capability,
                "element_context.capture_pointer()") ||
      !contains(element_context_capability, "element_context.set_cursor") ||
      !contains(element_context_capability, "element_context.state") ||
      !contains(element_context_capability, "std::same_as") ||
      !contains(element_context_capability,
                "cgpui::Render<ElementContextCapabilityView>")) {
    return 48;
  }
  if (contains(element_context_capability, "WindowRuntimeContext") ||
      contains(element_context_capability, "PlatformWindow")) {
    return 49;
  }

  const std::string typed_action =
      read_source("tests/api_parity/typed_action_surface_test.cpp");
  if (typed_action.empty()) {
    return 63;
  }
  if (!contains(typed_action, "#include \"cgpui/prelude.hpp\"") ||
      !contains(typed_action, "struct SaveAction") ||
      !contains(typed_action, "static constexpr std::string_view name") ||
      !contains(typed_action, "cgpui::Action<SaveAction>") ||
      !contains(typed_action, "!cgpui::Action<MissingActionName>") ||
      !contains(typed_action, "cgpui::action_name_v<SaveAction>") ||
      !contains(typed_action, "cgpui::action_name<CloseWindowAction>()")) {
    return 64;
  }
  if (contains(typed_action, "WindowRuntimeContext") ||
      contains(typed_action, "WindowRuntime") ||
      contains(typed_action, "AppContext") ||
      contains(typed_action, "ViewContext") ||
      contains(typed_action, "PlatformWindow") ||
      contains(typed_action, "KeyBinding") ||
      contains(typed_action, "dispatch_action")) {
    return 65;
  }

  const std::string typed_dispatch =
      read_source("tests/ui/typed_action_dispatch_test.cpp");
  if (typed_dispatch.empty()) {
    return 66;
  }
  if (!contains(typed_dispatch, "RuntimeSaveAction") ||
      !contains(typed_dispatch,
                "context.runtime.register_action<RuntimeSaveAction>") ||
      !contains(typed_dispatch,
                "context.runtime.dispatch_action<RuntimeSaveAction>()") ||
      !contains(typed_dispatch,
                "context.register_action<ContextLegacyAction>") ||
      !contains(typed_dispatch,
                "context.dispatch_action<ContextLegacyAction>()") ||
      !contains(typed_dispatch, "typed.runtime.string")) {
    return 67;
  }
  const std::string action_scope_metadata =
      read_source("tests/ui/action_scope_metadata_test.cpp");
  if (action_scope_metadata.empty()) {
    return 68;
  }
  if (!contains(action_scope_metadata, "ActionRegistrationScope::general") ||
      !contains(action_scope_metadata, "ActionRegistrationScope::app") ||
      !contains(action_scope_metadata, "ActionRegistrationScope::window") ||
      !contains(action_scope_metadata, "ActionRegistrationScope::view") ||
      !contains(action_scope_metadata,
                "ActionRegistrationScope::focused_element") ||
      !contains(action_scope_metadata, "action_registrations_for_scope") ||
      contains(action_scope_metadata, "KeyBinding") ||
      contains(action_scope_metadata, "CommandPaletteEntry")) {
    return 69;
  }
  const std::string typed_command_metadata =
      read_source("tests/ui/typed_action_command_metadata_test.cpp");
  const std::string runtime_command_palette_templates =
      read_source("include/cgpui/ui/runtime_command_palette_templates.hpp");
  if (typed_command_metadata.empty() ||
      runtime_command_palette_templates.empty()) {
    return 70;
  }
  if (!contains(typed_command_metadata,
                "command_palette_entry<RuntimePaletteAction>") ||
      !contains(typed_command_metadata,
                "register_command_palette_entry<ContextPaletteAction>") ||
      !contains(typed_command_metadata,
                "register_command_palette_entry<AppContextPaletteAction>") ||
      !contains(typed_command_metadata,
                "register_command_palette_entry<ViewPaletteAction>") ||
      !contains(typed_command_metadata,
                "register_command_palette_entry<FocusedPaletteAction>") ||
      !contains(runtime_command_palette_templates,
                "CommandPaletteEntry command_palette_entry(") ||
      !contains(runtime_command_palette_templates,
                "AppContext::register_command_palette_entry(") ||
      contains(typed_command_metadata, "KeyBinding")) {
    return 71;
  }
  const std::string action_enablement =
      read_source("tests/ui/action_enablement_metadata_test.cpp");
  const std::string runtime_action_enablement_templates =
      read_source("include/cgpui/ui/runtime_action_enablement_templates.hpp");
  const std::string runtime_action_registration =
      read_source("src/ui/runtime_action_registration.cpp");
  if (action_enablement.empty() ||
      runtime_action_enablement_templates.empty() ||
      runtime_action_registration.empty()) {
    return 72;
  }
  if (!contains(action_enablement, "ActionRegistrationOptions{.enabled = true}") ||
      !contains(action_enablement, "ActionRegistrationOptions{.enabled = false}") ||
      !contains(action_enablement, "action_registrations_for_enabled(false)") ||
      !contains(action_enablement, "runtime_disabled_result.handled") ||
      !contains(runtime_action_enablement_templates,
                "ActionRegistrationOptions options") ||
      !contains(runtime_action_registration, ".enabled = options.enabled") ||
      contains(action_enablement, "KeyBinding")) {
    return 73;
  }

  const std::string action_bubbling =
      read_source("tests/ui/action_bubbling_test.cpp");
  const std::string runtime_action_dispatch =
      read_source("src/ui/runtime_action_dispatch.cpp");
  if (action_bubbling.empty() || runtime_action_dispatch.empty()) {
    return 74;
  }
  if (!contains(action_bubbling, "bubble.unhandled.to.window") ||
      !contains(action_bubbling, "bubble.disabled.to.view") ||
      !contains(action_bubbling, "bubble.cancelled.stop") ||
      !contains(action_bubbling, "EventResult::unhandled()") ||
      !contains(action_bubbling, "EventResult::cancelled_event()") ||
      !contains(action_bubbling, "ActionRegistrationOptions{.enabled = false}") ||
      !contains(runtime_action_dispatch, "action_result_stops_bubbling") ||
      !contains(runtime_action_dispatch, "!action_result_stops_bubbling(result)") ||
      contains(action_bubbling, "KeyBinding")) {
    return 75;
  }
  const std::string key_binding_grammar =
      read_source("tests/ui/key_binding_grammar_test.cpp");
  const std::string key_binding_platform_modifier =
      read_source("tests/ui/key_binding_platform_modifier_test.cpp");
  const std::string runtime_key_binding_grammar =
      read_source("src/ui/runtime_key_binding_grammar.cpp");
  const std::string runtime_key_binding_modifiers =
      read_source("src/ui/runtime_key_binding_modifiers.cpp");
  const std::string key_binding_header =
      read_source("include/cgpui/ui/key_binding.hpp");
  if (key_binding_grammar.empty() || key_binding_platform_modifier.empty() ||
      runtime_key_binding_grammar.empty() ||
      runtime_key_binding_modifiers.empty() ||
      key_binding_header.empty()) {
    return 76;
  }
  if (!contains(key_binding_grammar, "parse_key_binding(\"ctrl-shift-s\"") ||
      !contains(key_binding_grammar,
                "context.bind_key(\"ctrl-shift-s\"") ||
      !contains(key_binding_grammar, "context.bind_key(\"cmd-p\"") ||
      !contains(key_binding_grammar, "ctrl-unknown") ||
      !contains(runtime_key_binding_grammar, "parse_key_binding(") ||
      !contains(runtime_key_binding_grammar,
                "apply_key_binding_modifier_token(") ||
      !contains(key_binding_platform_modifier, "secondary-s") ||
      !contains(key_binding_platform_modifier, "platform-p") ||
      !contains(key_binding_platform_modifier,
                "DesktopPlatformTarget::windows") ||
      !contains(key_binding_platform_modifier,
                "DesktopPlatformTarget::linux_wayland") ||
      !contains(key_binding_platform_modifier,
                "DesktopPlatformTarget::macos_cocoa") ||
      !contains(runtime_key_binding_modifiers, "secondary_modifier_for(") ||
      !contains(runtime_key_binding_modifiers, "platform_modifier_for(") ||
      !contains(key_binding_header, "struct KeyBinding") ||
      !contains(key_binding_header, "DesktopPlatformTarget") ||
      !contains(key_binding_header, "std::optional<KeyBinding>") ||
      contains(runtime_key_binding_grammar, "dispatch_action(")) {
    return 77;
  }
  const std::string command_palette_key_integration =
      read_source("tests/ui/command_palette_key_integration_test.cpp");
  const std::string runtime_command_palette_keys =
      read_source("src/ui/runtime_command_palette_keys.cpp");
  if (command_palette_key_integration.empty() ||
      runtime_command_palette_keys.empty()) {
    return 78;
  }
  if (!contains(command_palette_key_integration,
                "cgpui::CommandPaletteEntry") ||
      !contains(command_palette_key_integration,
                ".key_binding = \"ctrl-p\"") ||
      !contains(command_palette_key_integration,
                ".key_context = cgpui::KeyBindingContext::window()") ||
      !contains(command_palette_key_integration,
                ".enabled = false") ||
      !contains(command_palette_key_integration, "ctrl-unknown") ||
      !contains(runtime_command_palette_keys,
                "WindowRuntime::command_palette_key_binding(") ||
      !contains(runtime_command_palette_keys,
                "parse_key_binding(entry.key_binding") ||
      !contains(runtime_command_palette_keys,
                "KeyBindingContext::focused_element(") ||
      contains(runtime_command_palette_keys, "dispatch_action(")) {
    return 79;
  }

  const std::string xmake = read_source("xmake.lua");
  if (!contains(xmake, "target(\"gpui_parity_ledger_test\")") ||
      !contains(xmake, "target(\"context_render_spelling_test\")") ||
      !contains(xmake, "target(\"context_capabilities_test\")") ||
      !contains(xmake, "target(\"app_context_capability_test\")") ||
      !contains(xmake, "target(\"view_context_capability_test\")") ||
      !contains(xmake, "target(\"window_context_capability_test\")") ||
      !contains(xmake, "target(\"element_context_capability_test\")") ||
      !contains(xmake, "target(\"view_handle_spelling_test\")") ||
      !contains(xmake, "target(\"public_authoring_surface_test\")") ||
      !contains(xmake, "target(\"entity_lifecycle_creation_test\")") ||
      !contains(xmake, "target(\"entity_weak_handle_semantics_test\")") ||
      !contains(xmake, "target(\"entity_observation_test\")") ||
      !contains(xmake,
                "target(\"window_runtime_observation_closure_test\")") ||
      !contains(xmake, "target(\"entity_transaction_test\")") ||
      !contains(xmake, "target(\"entity_invalidation_test\")") ||
      !contains(xmake, "target(\"entity_deletion_test\")") ||
      !contains(xmake, "target(\"typed_action_surface_test\")") ||
      !contains(xmake, "target(\"typed_action_dispatch_test\")") ||
      !contains(xmake, "target(\"action_scope_metadata_test\")") ||
      !contains(xmake, "target(\"action_enablement_metadata_test\")") ||
      !contains(xmake, "target(\"action_bubbling_test\")") ||
      !contains(xmake, "target(\"key_binding_grammar_test\")") ||
      !contains(xmake, "target(\"key_binding_platform_modifier_test\")") ||
      !contains(xmake, "target(\"keymap_context_test\")") ||
      !contains(xmake, "target(\"typed_action_command_metadata_test\")") ||
      !contains(xmake, "target(\"command_palette_key_integration_test\")") ||
      !contains(xmake,
                "target(\"test_context_keystroke_simulation_test\")") ||
      !contains(xmake,
                "target(\"test_context_pointer_simulation_test\")") ||
      !contains(xmake, "target(\"test_context_time_async_test\")") ||
      !contains(xmake, "target(\"test_context_frame_pump_test\")") ||
      !contains(xmake, "target(\"renderer_frame_result_conventions_test\")") ||
      !contains(xmake, "target(\"async_spawn_result_conventions_test\")") ||
      !contains(xmake, "target(\"api_parity_hello_world\")") ||
      !contains(xmake,
                "target(\"api_parity_public_api_compatibility\")") ||
      !contains(xmake,
                "target(\"api_parity_public_authoring_workflow\")") ||
      !contains(xmake,
                "target(\"api_parity_public_context_capabilities\")") ||
      !contains(xmake,
                "target(\"api_parity_public_async_test_workflow\")") ||
      !contains(xmake,
                "target(\"api_parity_public_phase_b_surface_closure\")") ||
      !contains(xmake,
                "target(\"public_api_compatibility_examples_test\")") ||
      !contains(xmake,
                "target(\"public_api_example_expansion_test\")") ||
      !contains(xmake,
                "target(\"public_context_capability_example_test\")") ||
      !contains(xmake,
                "target(\"public_async_test_workflow_example_test\")") ||
      !contains(
          xmake,
          "target(\"public_phase_b_surface_closure_example_test\")") ||
      !contains(xmake,
                "target(\"public_authoring_vocabulary_freeze_test\")") ||
      !contains(xmake,
                "target(\"public_phase_b_completion_audit_test\")") ||
      !contains(xmake,
                "target(\"phase_c_style_cascade_depth_audit_test\")") ||
      !contains(xmake,
                "target(\"phase_c_focusable_interactable_audit_test\")") ||
      !contains(xmake,
                "target(\"phase_c_uniform_list_audit_test\")") ||
      !contains(xmake,
                "target(\"phase_c_window_examples_public_api_test\")") ||
      !contains(xmake,
                "target(\"phase_c_window_examples_workflow_test\")") ||
      !contains(
          xmake,
          "target(\"phase_c_window_examples_widget_catalog_test\")") ||
      !contains(
          xmake,
          "target(\"phase_c_window_examples_interaction_states_test\")") ||
      !contains(xmake,
                "target(\"phase_c_window_examples_service_matrix_test\")") ||
      !contains(xmake,
                "target(\"phase_c_window_examples_closeout_test\")") ||
      !contains(xmake, "target(\"phase_c_svg_image_front_end_test\")") ||
      !contains(xmake,
                "target(\"phase_c_svg_image_asset_registration_test\")") ||
      !contains(xmake, "target(\"builtin_widget_test\")") ||
      !contains(xmake, "target(\"widget_source_structure_test\")") ||
      !contains(xmake, "target(\"api_parity_public_window_examples\")") ||
      !contains(xmake,
                "target(\"api_parity_public_window_examples_workflow\")") ||
      !contains(
          xmake,
          "target(\"api_parity_public_window_examples_widget_catalog\")") ||
      !contains(
          xmake,
          "target(\"api_parity_public_window_examples_interaction_states\")") ||
      !contains(
          xmake,
          "target(\"api_parity_public_window_examples_service_matrix\")") ||
      !contains(xmake, "tests/api_parity/gpui_parity_ledger_test.cpp") ||
      !contains(xmake, "tests/api_parity/context_render_spelling_test.cpp") ||
      !contains(xmake, "tests/api_parity/context_capabilities_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/app_context_capability_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/view_context_capability_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/window_context_capability_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/element_context_capability_test.cpp") ||
      !contains(xmake, "tests/api_parity/view_handle_spelling_test.cpp") ||
      !contains(xmake, "tests/api_parity/public_authoring_surface_test.cpp") ||
      !contains(xmake, "tests/api_parity/entity_lifecycle_creation_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_weak_handle_semantics_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_observation_test.cpp") ||
      !contains(xmake,
                "tests/ui/window_runtime_observation_closure_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_transaction_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_invalidation_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/entity_deletion_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/typed_action_surface_test.cpp") ||
      !contains(xmake,
                "tests/ui/typed_action_dispatch_test.cpp") ||
      !contains(xmake,
                "tests/ui/action_scope_metadata_test.cpp") ||
      !contains(xmake,
                "tests/ui/action_enablement_metadata_test.cpp") ||
      !contains(xmake,
                "tests/ui/action_bubbling_test.cpp") ||
      !contains(xmake,
                "tests/ui/key_binding_grammar_test.cpp") ||
      !contains(xmake,
                "tests/ui/key_binding_platform_modifier_test.cpp") ||
      !contains(xmake,
                "tests/ui/keymap_context_test.cpp") ||
      !contains(xmake,
                "tests/ui/key_binding_partial_match_test.cpp") ||
      !contains(xmake,
                "tests/ui/key_binding_disabled_scope_test.cpp") ||
      !contains(xmake,
                "tests/ui/typed_action_command_metadata_test.cpp") ||
      !contains(xmake,
                "tests/ui/command_palette_key_integration_test.cpp") ||
      !contains(xmake,
                "tests/ui/test_context_keystroke_simulation_test.cpp") ||
      !contains(xmake,
                "tests/ui/test_context_pointer_simulation_test.cpp") ||
      !contains(xmake,
                "tests/ui/test_context_time_async_test.cpp") ||
      !contains(xmake,
                "tests/ui/test_context_frame_pump_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/renderer_frame_result_conventions_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/async_spawn_result_conventions_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/public_api_compatibility_examples_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/public_api_example_expansion_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/public_context_capability_example_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/public_async_test_workflow_example_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/public_phase_b_surface_closure_example_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/public_authoring_vocabulary_freeze_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/public_phase_b_completion_audit_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_focusable_interactable_audit_test.cpp") ||
      !contains(xmake,
                "tests/api_parity/phase_c_uniform_list_audit_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_public_api_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_workflow_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_interaction_states_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_service_matrix_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_closeout_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_svg_image_front_end_test.cpp") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_svg_image_asset_registration_test.cpp") ||
      !contains(xmake, "tests/ui/builtin_widget_test.cpp") ||
      !contains(
          xmake,
          "tests/architecture/widget_source_structure_test.cpp") ||
      !contains(xmake, "add_files(\"src/ui/widgets/*.cpp\")") ||
      !contains(xmake, "examples/api_parity/hello_world/main.cpp") ||
      !contains(
          xmake,
          "examples/api_parity/public_api_compatibility/main.cpp") ||
      !contains(
          xmake,
          "examples/api_parity/public_authoring_workflow/main.cpp") ||
      !contains(
          xmake,
          "examples/api_parity/public_context_capabilities/main.cpp") ||
      !contains(
          xmake,
          "examples/api_parity/public_async_test_workflow/main.cpp")) {
    return 14;
  }

  if (!contains(
          xmake,
          "examples/api_parity/public_phase_b_surface_closure/main.cpp")) {
    return 14;
  }

  if (!contains(xmake,
                "examples/api_parity/public_window_examples/main.cpp")) {
    return 14;
  }

  if (!contains(
          xmake,
          "examples/api_parity/public_window_examples_workflow/main.cpp")) {
    return 14;
  }

  if (!contains(
          xmake,
          "examples/api_parity/public_window_examples_widget_catalog/main.cpp")) {
    return 14;
  }

  if (!contains(
          xmake,
          "examples/api_parity/public_window_examples_interaction_states/main.cpp")) {
    return 14;
  }

  if (!contains(
          xmake,
          "examples/api_parity/public_window_examples_service_matrix/main.cpp")) {
    return 14;
  }

  return 0;
}
