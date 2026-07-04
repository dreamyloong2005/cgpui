#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
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

std::size_t line_count(const std::string& text) {
  std::size_t count = 0;
  for (const char value : text) {
    if (value == '\n') {
      count += 1;
    }
  }
  return count;
}

} // namespace

int main() {
  const std::vector<const char*> public_headers{
      "include/cgpui/ui/action.hpp",
      "include/cgpui/ui/paint.hpp",
      "include/cgpui/ui/async_context.hpp",
      "include/cgpui/ui/test_context.hpp",
      "include/cgpui/ui/render.hpp",
      "include/cgpui/ui/element_context.hpp",
      "include/cgpui/ui/view_context.hpp",
      "include/cgpui/ui/view_handle.hpp",
      "include/cgpui/ui/window_context.hpp",
      "include/cgpui/ui/view.hpp",
      "include/cgpui/ui/style_tokens.hpp",
      "include/cgpui/ui/style_values.hpp",
      "include/cgpui/ui/style_box.hpp",
      "include/cgpui/ui/style_text.hpp",
      "include/cgpui/ui/style_layout.hpp",
      "include/cgpui/ui/style_animation.hpp",
      "include/cgpui/ui/style_overlay.hpp",
      "include/cgpui/ui/style_state.hpp",
      "include/cgpui/ui/style_core.hpp",
      "include/cgpui/ui/style_cascade.hpp",
      "include/cgpui/ui/style.hpp",
      "include/cgpui/ui/text_edit_actions.hpp",
      "include/cgpui/ui/text_font.hpp",
      "include/cgpui/ui/text_shape.hpp",
      "include/cgpui/ui/text_glyphs.hpp",
      "include/cgpui/ui/text_measurement.hpp",
      "include/cgpui/ui/text_wrapping.hpp",
      "include/cgpui/ui/text_hit_testing.hpp",
      "include/cgpui/ui/text_layout.hpp",
      "include/cgpui/ui/text_model.hpp",
      "include/cgpui/ui/text.hpp",
      "include/cgpui/ui/element_core.hpp",
      "include/cgpui/ui/element_layout_nodes.hpp",
      "include/cgpui/ui/element_style_nodes.hpp",
      "include/cgpui/ui/element_containers.hpp",
      "include/cgpui/ui/element_text_nodes.hpp",
      "include/cgpui/ui/element_pointer_nodes.hpp",
      "include/cgpui/ui/element_focus_nodes.hpp",
      "include/cgpui/ui/element_button_nodes.hpp",
      "include/cgpui/ui/element_interaction_nodes.hpp",
      "include/cgpui/ui/element_scroll_nodes.hpp",
      "include/cgpui/ui/element_nodes.hpp",
      "include/cgpui/ui/element_builder_core.hpp",
      "include/cgpui/ui/element_builder.hpp",
      "include/cgpui/ui/text_input_builder.hpp",
      "include/cgpui/ui/label_builder.hpp",
      "include/cgpui/ui/button_builder.hpp",
      "include/cgpui/ui/scrollable_list_builder.hpp",
      "include/cgpui/ui/widget_builders.hpp",
      "include/cgpui/ui/element_builders.hpp",
      "include/cgpui/ui/element_tree_templates.hpp",
      "include/cgpui/ui/element_tree.hpp",
      "include/cgpui/ui/element.hpp",
      "include/cgpui/ui/runtime_callbacks.hpp",
      "include/cgpui/ui/runtime_ids.hpp",
      "include/cgpui/ui/runtime_handles.hpp",
      "include/cgpui/ui/runtime_window_options.hpp",
      "include/cgpui/ui/runtime_app_context.hpp",
      "include/cgpui/ui/runtime_actions.hpp",
      "include/cgpui/ui/runtime_events.hpp",
      "include/cgpui/ui/runtime_diagnostics.hpp",
      "include/cgpui/ui/runtime_input_state.hpp",
      "include/cgpui/ui/runtime_context.hpp",
      "include/cgpui/ui/runtime_types.hpp",
      "include/cgpui/ui/window_runtime.hpp",
      "include/cgpui/ui/runtime_rendering.hpp",
      "include/cgpui/ui/runtime_templates.hpp",
      "include/cgpui/ui/runtime_action_enablement_templates.hpp",
      "include/cgpui/ui/runtime_command_palette_templates.hpp",
      "include/cgpui/ui/runtime.hpp",
  };
  for (const char* header : public_headers) {
    if (read_source(header).empty()) {
      return 1;
    }
  }

  const std::vector<const char*> text_implementation_files{
      "src/ui/text_shape.cpp",
      "src/ui/text_glyph_raster.cpp",
      "src/ui/text_measurement.cpp",
      "src/ui/text_wrapping.cpp",
      "src/ui/text_hit_testing.cpp",
  };
  for (const char* source : text_implementation_files) {
    if (read_source(source).empty()) {
      return 113;
    }
  }

  const std::vector<const char*> runtime_test_files{
      "tests/ui/window_runtime_test_support.hpp",
      "tests/ui/window_runtime_input_test.cpp",
      "tests/ui/window_runtime_focus_test.cpp",
      "tests/ui/window_runtime_actions_test.cpp",
      "tests/ui/window_runtime_text_test.cpp",
      "tests/ui/window_runtime_rendering_test.cpp",
      "tests/ui/window_runtime_scheduling_test.cpp",
      "tests/ui/window_runtime_multiwindow_test.cpp",
      "tests/ui/window_runtime_theme_test.cpp",
  };
  for (const char* test_file : runtime_test_files) {
    if (read_source(test_file).empty()) {
      return 110;
    }
  }
  if (line_count(read_source("tests/ui/window_runtime_test.cpp")) > 260) {
    return 111;
  }
  if (line_count(read_source("tests/ui/window_runtime_test_support.hpp")) >
          1800 ||
      line_count(read_source("tests/ui/window_runtime_input_test.cpp")) >
          2600 ||
      line_count(read_source("tests/ui/window_runtime_focus_test.cpp")) >
          1200 ||
      line_count(read_source("tests/ui/window_runtime_actions_test.cpp")) >
          1800 ||
      line_count(read_source("tests/ui/window_runtime_text_test.cpp")) > 1800 ||
      line_count(read_source("tests/ui/window_runtime_rendering_test.cpp")) >
          1200 ||
      line_count(read_source("tests/ui/window_runtime_scheduling_test.cpp")) >
          1800 ||
      line_count(read_source("tests/ui/window_runtime_multiwindow_test.cpp")) >
          900 ||
      line_count(read_source("tests/ui/window_runtime_theme_test.cpp")) > 500) {
    return 112;
  }

  const std::string ui_header = read_source("include/cgpui/ui/ui.hpp");
  if (ui_header.empty()) {
    return 2;
  }
  if (!contains(ui_header, "#include \"cgpui/ui/paint.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/action.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/async_context.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/test_context.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/render.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/element_context.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/view_context.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/window_context.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/view.hpp\"") ||
      !contains(ui_header, "#include \"cgpui/ui/runtime.hpp\"")) {
    return 3;
  }
  if (contains(ui_header, "class PaintList") ||
      contains(ui_header, "class WindowRuntime") ||
      contains(ui_header, "struct WindowRuntimeContext")) {
    return 4;
  }

  const std::string paint_header = read_source("include/cgpui/ui/paint.hpp");
  if (!contains(paint_header, "class PaintList") ||
      !contains(paint_header, "struct PaintCommand") ||
      !contains(paint_header, "struct ImagePaint")) {
    return 5;
  }

  const std::string render_header = read_source("include/cgpui/ui/render.hpp");
  if (!contains(render_header, "using ViewContext = WindowRuntimeContext") ||
      !contains(render_header, "using Context = ViewContext") ||
      !contains(render_header, "using IntoElement = AnyElement") ||
      !contains(render_header, "concept Render")) {
    return 6;
  }
  if (line_count(render_header) > 80 ||
      contains(render_header, "class View")) {
    return 115;
  }

  const std::string view_header = read_source("include/cgpui/ui/view.hpp");
  if (!contains(view_header, "#include \"cgpui/ui/render.hpp\"") ||
      !contains(view_header, "class View")) {
    return 116;
  }
  if (contains(view_header, "using IntoElement") ||
      contains(view_header, "concept Render")) {
    return 117;
  }

  const std::string style_header = read_source("include/cgpui/ui/style.hpp");
  if (!contains(style_header, "#include \"cgpui/ui/style_tokens.hpp\"") ||
      !contains(style_header, "#include \"cgpui/ui/style_values.hpp\"") ||
      !contains(style_header, "#include \"cgpui/ui/style_core.hpp\"") ||
      !contains(style_header, "#include \"cgpui/ui/style_cascade.hpp\"")) {
    return 26;
  }
  if (line_count(style_header) > 40 ||
      contains(style_header, "struct Style") ||
      contains(style_header, "class Theme") ||
      contains(style_header, "class StyleCascade")) {
    return 27;
  }

  const std::string style_tokens_header =
      read_source("include/cgpui/ui/style_tokens.hpp");
  const std::string style_values_header =
      read_source("include/cgpui/ui/style_values.hpp");
  const std::string style_core_header =
      read_source("include/cgpui/ui/style_core.hpp");
  const std::string style_box_header =
      read_source("include/cgpui/ui/style_box.hpp");
  const std::string style_text_header =
      read_source("include/cgpui/ui/style_text.hpp");
  const std::string style_layout_header =
      read_source("include/cgpui/ui/style_layout.hpp");
  const std::string style_animation_header =
      read_source("include/cgpui/ui/style_animation.hpp");
  const std::string style_overlay_header =
      read_source("include/cgpui/ui/style_overlay.hpp");
  const std::string style_state_header =
      read_source("include/cgpui/ui/style_state.hpp");
  const std::string style_cascade_header =
      read_source("include/cgpui/ui/style_cascade.hpp");
  if (!contains(style_core_header, "#include \"cgpui/ui/style_box.hpp\"") ||
      !contains(style_core_header, "#include \"cgpui/ui/style_text.hpp\"") ||
      !contains(style_core_header, "#include \"cgpui/ui/style_layout.hpp\"") ||
      !contains(style_core_header,
                "#include \"cgpui/ui/style_animation.hpp\"") ||
      !contains(style_core_header, "#include \"cgpui/ui/style_overlay.hpp\"") ||
      !contains(style_core_header, "#include \"cgpui/ui/style_state.hpp\"")) {
    return 101;
  }
  if (!contains(style_tokens_header, "class Theme") ||
      !contains(style_values_header, "struct FontDescriptor") ||
      !contains(style_box_header, "struct Style") ||
      !contains(style_text_header, "#include \"cgpui/ui/style_box.hpp\"") ||
      !contains(style_layout_header, "#include \"cgpui/ui/style_box.hpp\"") ||
      !contains(style_animation_header, "struct StyleTween") ||
      !contains(style_overlay_header, "struct StyleOverlay") ||
      !contains(style_state_header, "struct StyleState") ||
      !contains(style_cascade_header, "class StyleCascade")) {
    return 28;
  }
  if (line_count(style_core_header) > 220 ||
      contains(style_core_header, "struct Style {") ||
      contains(style_core_header, "struct StyleOverlay {") ||
      contains(style_core_header, "struct StyleState {")) {
    return 102;
  }

  const std::string text_header = read_source("include/cgpui/ui/text.hpp");
  if (!contains(text_header, "#include \"cgpui/ui/text_font.hpp\"") ||
      !contains(text_header, "#include \"cgpui/ui/text_edit_actions.hpp\"") ||
      !contains(text_header, "#include \"cgpui/ui/text_layout.hpp\"") ||
      !contains(text_header, "#include \"cgpui/ui/text_model.hpp\"")) {
    return 21;
  }
  if (line_count(text_header) > 40 ||
      contains(text_header, "class TextModel") ||
      contains(text_header, "class FontDatabase") ||
      contains(text_header, "struct TextMeasurement")) {
    return 22;
  }

  const std::string text_font_header =
      read_source("include/cgpui/ui/text_font.hpp");
  const std::string text_edit_actions_header =
      read_source("include/cgpui/ui/text_edit_actions.hpp");
  const std::string text_shape_header =
      read_source("include/cgpui/ui/text_shape.hpp");
  const std::string text_glyphs_header =
      read_source("include/cgpui/ui/text_glyphs.hpp");
  const std::string text_measurement_header =
      read_source("include/cgpui/ui/text_measurement.hpp");
  const std::string text_wrapping_header =
      read_source("include/cgpui/ui/text_wrapping.hpp");
  const std::string text_hit_testing_header =
      read_source("include/cgpui/ui/text_hit_testing.hpp");
  const std::string text_layout_header =
      read_source("include/cgpui/ui/text_layout.hpp");
  const std::string text_model_header =
      read_source("include/cgpui/ui/text_model.hpp");
  if (!contains(text_font_header, "class FontDatabase") ||
      !contains(text_edit_actions_header, "enum class TextEditAction") ||
      !contains(text_shape_header, "struct TextShapeRun") ||
      !contains(text_glyphs_header, "struct RasterizedGlyph") ||
      !contains(text_measurement_header, "class TextMeasurementCache") ||
      !contains(text_wrapping_header, "struct TextWrapLayout") ||
      !contains(text_hit_testing_header, "struct TextHitTestResult") ||
      !contains(text_layout_header, "#include \"cgpui/ui/text_shape.hpp\"") ||
      !contains(text_layout_header, "#include \"cgpui/ui/text_glyphs.hpp\"") ||
      !contains(text_layout_header,
                "#include \"cgpui/ui/text_measurement.hpp\"") ||
      !contains(text_layout_header, "#include \"cgpui/ui/text_wrapping.hpp\"") ||
      !contains(text_layout_header,
                "#include \"cgpui/ui/text_hit_testing.hpp\"") ||
      !contains(text_model_header, "class TextModel")) {
    return 23;
  }
  if (line_count(text_layout_header) > 220 ||
      line_count(text_model_header) > 220) {
    return 99;
  }
  if (contains(text_shape_header, " inline ") ||
      contains(text_glyphs_header, " inline ") ||
      contains(text_measurement_header, " inline ") ||
      contains(text_wrapping_header, " inline ") ||
      contains(text_hit_testing_header, " inline ")) {
    return 114;
  }

  const std::string runtime_header =
      read_source("include/cgpui/ui/runtime.hpp");
  if (!contains(runtime_header, "#include \"cgpui/ui/action.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_callbacks.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_ids.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_handles.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_window_options.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_app_context.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_actions.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_events.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_diagnostics.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_input_state.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_context.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_types.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/window_runtime.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_rendering.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_action_templates.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_action_enablement_templates.hpp\"") ||
      !contains(runtime_header,
                "#include \"cgpui/ui/runtime_command_palette_templates.hpp\"") ||
      !contains(runtime_header, "#include \"cgpui/ui/runtime_templates.hpp\"")) {
    return 7;
  }
  if (line_count(runtime_header) > 40 ||
      contains(runtime_header, "class WindowRuntime") ||
      contains(runtime_header, "struct WindowRuntimeContext") ||
      contains(runtime_header, "struct AppContext") ||
      contains(runtime_header, "template <typename T>")) {
    return 19;
  }

  const std::string action_header = read_source("include/cgpui/ui/action.hpp");
  if (!contains(action_header, "concept Action") ||
      !contains(action_header, "action_name_v") ||
      !contains(action_header, "action_name()") ||
      !contains(action_header, "std::string_view")) {
    return 124;
  }
  if (line_count(action_header) > 80 ||
      contains(action_header, "struct CommandPaletteEntry") ||
      contains(action_header, "KeyBinding") ||
      contains(action_header, "class WindowRuntime") ||
      contains(action_header, "struct WindowRuntimeContext")) {
    return 125;
  }

  const std::string runtime_types_header =
      read_source("include/cgpui/ui/runtime_types.hpp");
  const std::string runtime_callbacks_header =
      read_source("include/cgpui/ui/runtime_callbacks.hpp");
  const std::string runtime_ids_header =
      read_source("include/cgpui/ui/runtime_ids.hpp");
  const std::string runtime_handles_header =
      read_source("include/cgpui/ui/runtime_handles.hpp");
  const std::string runtime_window_options_header =
      read_source("include/cgpui/ui/runtime_window_options.hpp");
  const std::string runtime_app_context_header =
      read_source("include/cgpui/ui/runtime_app_context.hpp");
  const std::string runtime_actions_header =
      read_source("include/cgpui/ui/runtime_actions.hpp");
  const std::string runtime_events_header =
      read_source("include/cgpui/ui/runtime_events.hpp");
  const std::string runtime_diagnostics_header =
      read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string runtime_input_state_header =
      read_source("include/cgpui/ui/runtime_input_state.hpp");
  const std::string async_context_header =
      read_source("include/cgpui/ui/async_context.hpp");
  const std::string test_context_header =
      read_source("include/cgpui/ui/test_context.hpp");
  const std::string element_context_header =
      read_source("include/cgpui/ui/element_context.hpp");
  const std::string view_context_header =
      read_source("include/cgpui/ui/view_context.hpp");
  const std::string view_handle_header =
      read_source("include/cgpui/ui/view_handle.hpp");
  const std::string window_context_header =
      read_source("include/cgpui/ui/window_context.hpp");
  const std::string runtime_context_header =
      read_source("include/cgpui/ui/runtime_context.hpp");
  const std::string window_runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string window_runtime_internal_header =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string runtime_rendering_header =
      read_source("include/cgpui/ui/runtime_rendering.hpp");
  const std::string runtime_action_templates_header =
      read_source("include/cgpui/ui/runtime_action_templates.hpp");
  const std::string runtime_action_enablement_templates_header =
      read_source("include/cgpui/ui/runtime_action_enablement_templates.hpp");
  const std::string runtime_command_palette_templates_header =
      read_source("include/cgpui/ui/runtime_command_palette_templates.hpp");
  const std::string runtime_templates_header =
      read_source("include/cgpui/ui/runtime_templates.hpp");
  if (!contains(runtime_callbacks_header, "using ActionHandler") ||
      !contains(runtime_ids_header, "struct WindowRuntimeId") ||
      !contains(runtime_handles_header, "class Subscription") ||
      !contains(runtime_window_options_header, "struct WindowOptions") ||
      !contains(runtime_app_context_header, "struct AppContext") ||
      !contains(runtime_actions_header, "enum class ActionRegistrationScope") ||
      !contains(runtime_actions_header, "struct ActionRegistrationOptions") ||
      !contains(runtime_actions_header, "struct ActionRegistration") ||
      !contains(runtime_actions_header, "bool enabled = true") ||
      !contains(runtime_actions_header, "struct CommandPaletteEntry") ||
      !contains(runtime_events_header, "struct EventRoute") ||
      !contains(runtime_diagnostics_header,
                "struct RuntimeDiagnosticsSnapshot") ||
      !contains(async_context_header, "class AsyncContextCapability") ||
      !contains(async_context_header, "void defer(") ||
      !contains(async_context_header, "TimerId schedule_timer(") ||
      !contains(async_context_header, "AnimationHandle start_animation(") ||
      !contains(async_context_header, "TaskHandle spawn_background_task(") ||
      !contains(async_context_header, "void batch_updates(") ||
      !contains(test_context_header, "class TestContextCapability") ||
      !contains(test_context_header, "WindowRuntimeId runtime_id() const") ||
      !contains(test_context_header, "ViewInputState input_state() const") ||
      !contains(test_context_header, "void advance_time(") ||
      !contains(test_context_header, "bool complete_task(") ||
      !contains(test_context_header, "void drain_task_completions(") ||
      !contains(element_context_header, "class ElementContextCapability") ||
      !contains(element_context_header, "ElementId element_id() const") ||
      !contains(element_context_header, "void capture_pointer() const") ||
      !contains(element_context_header, "void set_cursor(") ||
      !contains(element_context_header, "T* state() const") ||
      !contains(view_context_header, "class ViewContextCapability") ||
      !contains(view_context_header, "ViewId view_id() const") ||
      !contains(view_context_header, "ViewHandle<T> view() const") ||
      !contains(view_context_header, "WeakViewHandle<T> weak_view() const") ||
      !contains(view_context_header, "bool observe(") ||
      !contains(view_context_header, "Subscription observe_subscription(") ||
      !contains(view_context_header, "const T* current() const") ||
      !contains(view_handle_header, "class WeakView") ||
      !contains(view_handle_header, "class ViewHandle") ||
      !contains(view_handle_header, "class WeakViewHandle") ||
      !contains(view_handle_header, "bool observe(") ||
      !contains(view_handle_header, "Subscription observe_subscription(") ||
      !contains(window_context_header, "class WindowContextCapability") ||
      !contains(window_context_header, "Window window() const") ||
      !contains(window_context_header, "Window current_window() const") ||
      !contains(window_context_header, "WindowRuntimeId runtime_id() const") ||
      !contains(window_context_header, "bool observe(") ||
      !contains(window_context_header,
                "Subscription observe_subscription(") ||
      !contains(window_context_header, "void request_render() const") ||
      !contains(runtime_input_state_header, "struct ViewInputState") ||
      !contains(runtime_context_header, "struct WindowRuntimeContext") ||
      !contains(runtime_context_header, "AppContext app_context() const") ||
      !contains(runtime_context_header,
                "AsyncContextCapability async_context() const") ||
      !contains(runtime_context_header,
                "TestContextCapability test_context() const") ||
      !contains(runtime_context_header,
                "WindowContextCapability window_context() const") ||
      !contains(runtime_context_header,
                "ElementContextCapability element_context(") ||
      !contains(runtime_context_header,
                "ViewContextCapability<T> view_context() const") ||
      !contains(runtime_context_header, "ViewHandle<T> view() const") ||
      !contains(runtime_context_header, "bool observe_window(") ||
      !contains(runtime_context_header,
                "Subscription observe_window_subscription(") ||
      !contains(runtime_context_header, "bool observe_view(") ||
      !contains(runtime_context_header,
                "Subscription observe_view_subscription(") ||
      !contains(runtime_templates_header,
                "ViewContextCapability<T> "
                "WindowRuntimeContext::view_context() const") ||
      !contains(runtime_templates_header,
                "WindowRuntimeContext::observe_window(") ||
      !contains(runtime_templates_header,
                "WindowRuntimeContext::observe_view(") ||
      !contains(runtime_templates_header,
                "ViewContextCapability<T>::observe(") ||
      !contains(runtime_templates_header,
                "const T* ViewContextCapability<T>::current() const") ||
      !contains(runtime_templates_header,
                "ViewHandle<T> WindowRuntimeContext::view() const") ||
      !contains(runtime_templates_header,
                "WeakViewHandle<T> WindowRuntimeContext::weak_view() const") ||
      !contains(runtime_templates_header,
                "T* ElementContextCapability::state() const") ||
      !contains(runtime_templates_header,
                "T* ElementContextCapability::emplace_state(") ||
      !contains(runtime_templates_header,
                "T* ElementContextCapability::state_or_init(") ||
      !contains(runtime_context_header, "PlatformWindow& platform_window") ||
      !contains(runtime_context_header, "Window window() const") ||
      !contains(runtime_context_header, "EntityHandle<T> new_entity(") ||
      !contains(runtime_templates_header,
                "EntityHandle<T> WindowRuntimeContext::new_entity(") ||
      !contains(runtime_templates_header,
                "EntityHandle<T> WindowRuntimeContext::insert_entity_handle(") ||
      !contains(runtime_templates_header,
                "EntityHandle<T> WindowRuntimeContext::entity(") ||
      !contains(runtime_templates_header,
                "WeakEntity<T> WindowRuntimeContext::weak_entity(") ||
      !contains(runtime_context_header,
                "const T* read_entity(EntityHandle<T> entity) const") ||
      !contains(runtime_templates_header,
                "const T* WindowRuntimeContext::read_entity("
                "EntityHandle<T> entity) const") ||
      !contains(runtime_context_header, "auto update_entity(") ||
      !contains(runtime_templates_header,
                "auto WindowRuntimeContext::update_entity(") ||
      !contains(runtime_context_header,
                "bool invalidate_entity(EntityHandle<T> entity) const") ||
      !contains(runtime_templates_header,
                "bool WindowRuntimeContext::invalidate_entity(") ||
      !contains(runtime_context_header,
                "bool remove_entity(EntityHandle<T> entity) const") ||
      !contains(runtime_templates_header,
                "bool WindowRuntimeContext::remove_entity(") ||
      !contains(runtime_context_header, "bool observe_entity(") ||
      !contains(runtime_context_header,
                "EntityHandle<ObserverT> observer_entity") ||
      !contains(runtime_context_header,
                "Subscription observe_entity_subscription(") ||
      !contains(runtime_templates_header,
                "WindowRuntimeContext::observe_entity(") ||
      !contains(runtime_templates_header,
                "EntityHandle<ObserverT> observer_entity") ||
      !contains(runtime_templates_header,
                "invoke_entity_to_entity_observer(") ||
      !contains(runtime_templates_header, "entity_context_token(") ||
      !contains(runtime_templates_header,
                "matches_context(detail::entity_context_token(*this))") ||
      !contains(window_runtime_header,
                "bool invalidate_entity(EntityId<T> entity_id)") ||
      !contains(runtime_types_header, "#include \"cgpui/ui/runtime_context.hpp\"") ||
      !contains(window_runtime_header, "class WindowRuntime") ||
      !contains(window_runtime_header, "struct AppRunnerOptions") ||
      !contains(window_runtime_internal_header, "struct RuntimeTask") ||
      !contains(window_runtime_internal_header, "PlatformApplication& application_") ||
      !contains(runtime_rendering_header, "Result<void> render_view(") ||
      !contains(runtime_rendering_header, "int run_app(") ||
      !contains(runtime_action_templates_header,
                "WindowRuntime::register_action(ActionHandler handler)") ||
      !contains(runtime_action_templates_header,
                "WindowRuntime::dispatch_action()") ||
      !contains(runtime_action_templates_header,
                "WindowRuntimeContext::register_action(ActionHandler handler) const") ||
      !contains(runtime_action_templates_header,
                "WindowRuntimeContext::dispatch_action() const") ||
      !contains(runtime_action_enablement_templates_header,
                "WindowRuntime::register_action(") ||
      !contains(runtime_action_enablement_templates_header,
                "ActionRegistrationOptions options") ||
      !contains(runtime_action_enablement_templates_header,
                "WindowRuntimeContext::register_action(") ||
      !contains(runtime_command_palette_templates_header,
                "CommandPaletteEntry command_palette_entry(") ||
      !contains(runtime_command_palette_templates_header,
                "WindowRuntime::register_command_palette_entry(") ||
      !contains(runtime_command_palette_templates_header,
                "WindowRuntimeContext::register_command_palette_entry(") ||
      !contains(runtime_command_palette_templates_header,
                "AppContext::register_command_palette_entry(") ||
      !contains(window_runtime_header, "action_registrations() const") ||
      !contains(window_runtime_header, "action_registrations_for_enabled(") ||
      !contains(runtime_context_header, "action_registrations()") ||
      !contains(runtime_context_header, "action_registrations_for_enabled(") ||
      !contains(runtime_templates_header,
                "void WindowRuntimeContext::set_global") ||
      !contains(runtime_templates_header,
                "EntityStore<T>& WindowRuntime::entity_store")) {
    return 20;
  }
  if (line_count(runtime_types_header) > 220 ||
      line_count(async_context_header) > 120 ||
      line_count(test_context_header) > 120 ||
      line_count(element_context_header) > 120 ||
      line_count(view_handle_header) > 180 ||
      line_count(window_context_header) > 120 ||
      line_count(window_runtime_header) > 240 ||
      line_count(runtime_action_templates_header) > 120 ||
      line_count(runtime_action_enablement_templates_header) > 100 ||
      line_count(runtime_command_palette_templates_header) > 80 ||
      line_count(window_runtime_internal_header) > 260 ||
      contains(runtime_types_header, "struct WindowRuntimeContext") ||
      contains(runtime_types_header, "struct AppContext") ||
      contains(runtime_input_state_header, "class WeakView") ||
      contains(runtime_context_header, "PlatformWindow& window") ||
      contains(runtime_action_templates_header, "CommandPaletteEntry") ||
      contains(runtime_action_enablement_templates_header,
               "CommandPaletteEntry") ||
      contains(runtime_command_palette_templates_header, "KeyBinding") ||
      contains(runtime_action_templates_header, "KeyBinding") ||
      contains(runtime_action_enablement_templates_header, "KeyBinding") ||
      contains(window_runtime_header, "PlatformApplication& application_") ||
      contains(window_runtime_header, "std::vector<RuntimeTask> tasks_")) {
    return 100;
  }

  const std::string element_header = read_source("include/cgpui/ui/element.hpp");
  if (!contains(element_header, "#include \"cgpui/ui/element_core.hpp\"") ||
      !contains(element_header, "#include \"cgpui/ui/element_nodes.hpp\"") ||
      !contains(element_header, "#include \"cgpui/ui/element_builders.hpp\"") ||
      !contains(element_header, "#include \"cgpui/ui/element_tree.hpp\"")) {
    return 16;
  }
  if (line_count(element_header) > 40 ||
      contains(element_header, "class ElementTree") ||
      contains(element_header, "class ElementBuilder") ||
      contains(element_header, "class TextInputElement")) {
    return 17;
  }

  const std::string element_core_header =
      read_source("include/cgpui/ui/element_core.hpp");
  const std::string element_nodes_header =
      read_source("include/cgpui/ui/element_nodes.hpp");
  const std::string element_layout_nodes_header =
      read_source("include/cgpui/ui/element_layout_nodes.hpp");
  const std::string element_style_nodes_header =
      read_source("include/cgpui/ui/element_style_nodes.hpp");
  const std::string element_containers_header =
      read_source("include/cgpui/ui/element_containers.hpp");
  const std::string element_text_nodes_header =
      read_source("include/cgpui/ui/element_text_nodes.hpp");
  const std::string element_pointer_nodes_header =
      read_source("include/cgpui/ui/element_pointer_nodes.hpp");
  const std::string element_focus_nodes_header =
      read_source("include/cgpui/ui/element_focus_nodes.hpp");
  const std::string element_button_nodes_header =
      read_source("include/cgpui/ui/element_button_nodes.hpp");
  const std::string element_interaction_nodes_header =
      read_source("include/cgpui/ui/element_interaction_nodes.hpp");
  const std::string element_scroll_nodes_header =
      read_source("include/cgpui/ui/element_scroll_nodes.hpp");
  const std::string element_builders_header =
      read_source("include/cgpui/ui/element_builders.hpp");
  const std::string element_builder_core_header =
      read_source("include/cgpui/ui/element_builder_core.hpp");
  const std::string element_builder_header =
      read_source("include/cgpui/ui/element_builder.hpp");
  const std::string text_input_builder_header =
      read_source("include/cgpui/ui/text_input_builder.hpp");
  const std::string label_builder_header =
      read_source("include/cgpui/ui/label_builder.hpp");
  const std::string button_builder_header =
      read_source("include/cgpui/ui/button_builder.hpp");
  const std::string scrollable_list_builder_header =
      read_source("include/cgpui/ui/scrollable_list_builder.hpp");
  const std::string widget_builders_header =
      read_source("include/cgpui/ui/widget_builders.hpp");
  const std::string element_tree_templates_header =
      read_source("include/cgpui/ui/element_tree_templates.hpp");
  const std::string element_tree_header =
      read_source("include/cgpui/ui/element_tree.hpp");
  if (!contains(element_nodes_header,
                "#include \"cgpui/ui/element_layout_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_style_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_containers.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_text_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_pointer_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_focus_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_button_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_interaction_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_scroll_nodes.hpp\"")) {
    return 24;
  }
  if (line_count(element_nodes_header) > 40 ||
      contains(element_nodes_header, "class TextInputElement") ||
      contains(element_nodes_header, "class ButtonElement") ||
      contains(element_nodes_header, "class ScrollableListElement")) {
    return 25;
  }
  if (!contains(element_builders_header,
                "#include \"cgpui/ui/element_builder_core.hpp\"") ||
      !contains(element_builders_header,
                "#include \"cgpui/ui/element_builder.hpp\"") ||
      !contains(element_builders_header,
                "#include \"cgpui/ui/text_input_builder.hpp\"") ||
      !contains(element_builders_header,
                "#include \"cgpui/ui/label_builder.hpp\"") ||
      !contains(element_builders_header,
                "#include \"cgpui/ui/button_builder.hpp\"") ||
      !contains(element_builders_header,
                "#include \"cgpui/ui/scrollable_list_builder.hpp\"") ||
      !contains(element_builders_header,
                "#include \"cgpui/ui/widget_builders.hpp\"")) {
    return 29;
  }
  if (line_count(element_builders_header) > 40 ||
      contains(element_builders_header, "class ElementBuilder") ||
      contains(element_builders_header, "class ButtonBuilder") ||
      contains(element_builders_header, "class ScrollableListBuilder")) {
    return 30;
  }
  if (!contains(element_core_header, "class Element") ||
      !contains(element_layout_nodes_header, "class FlexElement") ||
      !contains(element_style_nodes_header, "class StyledElement") ||
      !contains(element_containers_header,
                "#include \"cgpui/ui/element_layout_nodes.hpp\"") ||
      !contains(element_containers_header,
                "#include \"cgpui/ui/element_style_nodes.hpp\"") ||
      !contains(element_text_nodes_header, "class TextInputElement") ||
      !contains(element_pointer_nodes_header, "class PointerElement") ||
      !contains(element_focus_nodes_header, "class FocusableElement") ||
      !contains(element_button_nodes_header, "class ButtonElement") ||
      !contains(element_interaction_nodes_header,
                "#include \"cgpui/ui/element_pointer_nodes.hpp\"") ||
      !contains(element_interaction_nodes_header,
                "#include \"cgpui/ui/element_focus_nodes.hpp\"") ||
      !contains(element_interaction_nodes_header,
                "#include \"cgpui/ui/element_button_nodes.hpp\"") ||
      !contains(element_scroll_nodes_header, "class ScrollableListElement") ||
      !contains(element_builder_core_header, "class ElementBuilder") ||
      !contains(element_builder_header,
                "#include \"cgpui/ui/element_builder_core.hpp\"") ||
      !contains(text_input_builder_header, "class TextInputBuilder") ||
      !contains(label_builder_header, "class LabelBuilder") ||
      !contains(button_builder_header, "class ButtonBuilder") ||
      !contains(scrollable_list_builder_header,
                "class ScrollableListBuilder") ||
      !contains(widget_builders_header,
                "#include \"cgpui/ui/text_input_builder.hpp\"") ||
      !contains(widget_builders_header,
                "#include \"cgpui/ui/label_builder.hpp\"") ||
      !contains(widget_builders_header,
                "#include \"cgpui/ui/button_builder.hpp\"") ||
      !contains(widget_builders_header,
                "#include \"cgpui/ui/scrollable_list_builder.hpp\"") ||
      !contains(element_tree_templates_header,
                "T* ElementTree::state(ElementId id)") ||
      !contains(element_tree_header,
                "#include \"cgpui/ui/element_tree_templates.hpp\"") ||
      !contains(element_tree_header, "class ElementTree")) {
    return 18;
  }
  if (line_count(element_tree_header) > 220 ||
      line_count(element_containers_header) > 220 ||
      line_count(element_interaction_nodes_header) > 220 ||
      line_count(element_builder_header) > 220 ||
      line_count(widget_builders_header) > 220 ||
      contains(element_tree_header, "ElementTree::state(ElementId id)") ||
      contains(element_tree_header, "void remove_subtree(ElementId id) {") ||
      contains(element_containers_header, "class FlexElement : public Element") ||
      contains(element_interaction_nodes_header,
               "class ButtonElement : public Element") ||
      contains(element_builder_header, "class ElementBuilder {") ||
      contains(widget_builders_header, "class ButtonBuilder {")) {
    return 103;
  }
  if (read_source("src/ui/element_tree.cpp").empty() ||
      read_source("src/ui/style_tween.cpp").empty()) {
    return 104;
  }
  const std::vector<const char*> element_style_sources{
      "src/ui/element_tree_root.cpp",
      "src/ui/element_tree_reconcile.cpp",
      "src/ui/element_tree_storage.cpp",
      "src/ui/element_tree_traversal.cpp",
      "src/ui/element_tree_accessibility.cpp",
      "src/ui/element_tree_paint.cpp",
      "src/ui/element_builder_factories.cpp",
      "src/ui/element_builder_style.cpp",
      "src/ui/element_builder_layout.cpp",
      "src/ui/element_builder_interaction.cpp",
      "src/ui/element_builder_build.cpp",
      "src/ui/element_builder_finish.cpp",
      "src/ui/element_fixed_size_node.cpp",
      "src/ui/element_vertical_stack_node.cpp",
      "src/ui/element_flex_node.cpp",
      "src/ui/element_flex_layout.cpp",
      "src/ui/element_style_paint.cpp",
      "src/ui/element_button_paint.cpp",
      "src/ui/element_scroll_paint.cpp",
      "src/ui/element_text_paint.cpp",
  };
  for (const char* source : element_style_sources) {
    if (read_source(source).empty()) {
      return 105;
    }
  }
  if (line_count(read_source("src/ui/element_tree.cpp")) > 20 ||
      line_count(read_source("src/ui/element_builder.cpp")) > 20 ||
      line_count(read_source("src/ui/element_layout_nodes.cpp")) > 20 ||
      line_count(read_source("src/ui/element_paint.cpp")) > 20 ||
      line_count(read_source("src/ui/element_tree_reconcile.cpp")) > 140 ||
      line_count(read_source("src/ui/element_builder_build.cpp")) > 140 ||
      line_count(read_source("src/ui/element_flex_node.cpp")) > 120 ||
      line_count(read_source("src/ui/element_flex_layout.cpp")) > 180) {
    return 106;
  }

  const std::vector<const char*> private_headers{
      "src/ui/ui_internal.hpp",
      "src/ui/ui_accessibility_internal.hpp",
      "src/ui/ui_event_focus_internal.hpp",
      "src/ui/ui_event_internal.hpp",
      "src/ui/ui_event_kind_internal.hpp",
      "src/ui/ui_event_pointer_internal.hpp",
      "src/ui/ui_paint_internal.hpp",
  };
  for (const char* header : private_headers) {
    if (read_source(header).empty()) {
      return 52;
    }
  }

  const std::string ui_internal_header =
      read_source("src/ui/ui_internal.hpp");
  if (!contains(ui_internal_header, "#include \"ui_accessibility_internal.hpp\"") ||
      !contains(ui_internal_header, "#include \"ui_event_internal.hpp\"") ||
      !contains(ui_internal_header, "#include \"ui_paint_internal.hpp\"") ||
      line_count(ui_internal_header) > 80 ||
      contains(ui_internal_header, "event_kind_for(") ||
      contains(ui_internal_header, "platform_accessibility_update_from(") ||
      contains(ui_internal_header, "paint_styled_box_base(")) {
    return 53;
  }

  const std::string ui_event_internal_header =
      read_source("src/ui/ui_event_internal.hpp");
  if (!contains(ui_event_internal_header,
                "#include \"ui_event_focus_internal.hpp\"") ||
      !contains(ui_event_internal_header,
                "#include \"ui_event_kind_internal.hpp\"") ||
      !contains(ui_event_internal_header,
                "#include \"ui_event_pointer_internal.hpp\"") ||
      line_count(ui_event_internal_header) > 30 ||
      contains(ui_event_internal_header, "event_kind_for(") ||
      contains(ui_event_internal_header, "pointer_position_for(") ||
      contains(ui_event_internal_header, "is_keyboard_routed_event(") ||
      contains(ui_event_internal_header, "apply_pointer_capture_owner_to_route(") ||
      contains(ui_event_internal_header, "paint_styled_box_base(") ||
      contains(ui_event_internal_header,
               "platform_accessibility_update_from(")) {
    return 54;
  }

  const std::string ui_event_kind_internal_header =
      read_source("src/ui/ui_event_kind_internal.hpp");
  if (line_count(ui_event_kind_internal_header) > 120 ||
      !contains(ui_event_kind_internal_header, "event_kind_for(") ||
      !contains(ui_event_kind_internal_header,
                "drag_drop_payload_value_count(") ||
      !contains(ui_event_kind_internal_header, "drag_drop_operation_for(") ||
      contains(ui_event_kind_internal_header, "pointer_position_for(") ||
      contains(ui_event_kind_internal_header,
               "is_keyboard_routed_event(")) {
    return 77;
  }

  const std::string ui_event_pointer_internal_header =
      read_source("src/ui/ui_event_pointer_internal.hpp");
  if (line_count(ui_event_pointer_internal_header) > 90 ||
      !contains(ui_event_pointer_internal_header, "pointer_position_for(") ||
      !contains(ui_event_pointer_internal_header,
                "is_valid_pointer_capture_owner(") ||
      !contains(ui_event_pointer_internal_header,
                "apply_pointer_capture_owner_to_route(") ||
      !contains(ui_event_pointer_internal_header,
                "hit_test_runtime_element_root(") ||
      contains(ui_event_pointer_internal_header, "event_kind_for(") ||
      contains(ui_event_pointer_internal_header,
               "is_keyboard_routed_event(")) {
    return 78;
  }

  const std::string ui_event_focus_internal_header =
      read_source("src/ui/ui_event_focus_internal.hpp");
  if (line_count(ui_event_focus_internal_header) > 90 ||
      !contains(ui_event_focus_internal_header,
                "is_keyboard_routed_event(") ||
      !contains(ui_event_focus_internal_header,
                "is_focus_activation_event(") ||
      !contains(ui_event_focus_internal_header,
                "is_focus_traversal_key(") ||
      !contains(ui_event_focus_internal_header, "modifiers_equal(") ||
      !contains(ui_event_focus_internal_header,
                "ime_text_input_placement_equal(") ||
      contains(ui_event_focus_internal_header, "event_kind_for(") ||
      contains(ui_event_focus_internal_header,
               "apply_pointer_capture_owner_to_route(")) {
    return 79;
  }

  const std::string ui_accessibility_internal_header =
      read_source("src/ui/ui_accessibility_internal.hpp");
  if (!contains(ui_accessibility_internal_header,
                "platform_accessibility_role(") ||
      !contains(ui_accessibility_internal_header,
                "platform_accessibility_update_from(") ||
      !contains(ui_accessibility_internal_header,
                "append_platform_accessibility_live_updates(") ||
      contains(ui_accessibility_internal_header, "event_kind_for(") ||
      contains(ui_accessibility_internal_header, "paint_styled_box_base(")) {
    return 55;
  }

  const std::string ui_paint_internal_header =
      read_source("src/ui/ui_paint_internal.hpp");
  if (!contains(ui_paint_internal_header, "compose_paint_metadata(") ||
      !contains(ui_paint_internal_header, "record_clip_stack_statistics(") ||
      !contains(ui_paint_internal_header,
                "record_composition_stack_statistics(") ||
      !contains(ui_paint_internal_header, "paint_styled_box_base(") ||
      contains(ui_paint_internal_header, "event_kind_for(") ||
      contains(ui_paint_internal_header,
               "platform_accessibility_update_from(")) {
    return 56;
  }

  const std::vector<const char*> source_files{
      "src/ui/paint.cpp",
      "src/ui/paint_text.cpp",
      "src/ui/paint_image.cpp",
      "src/ui/element_paint.cpp",
      "src/ui/render_view.cpp",
      "src/ui/render_view_commands.cpp",
      "src/ui/view.cpp",
      "src/ui/runtime_core.cpp",
      "src/ui/runtime_run.cpp",
      "src/ui/runtime_shutdown.cpp",
      "src/ui/runtime_rendering.cpp",
      "src/ui/runtime_views.cpp",
      "src/ui/runtime_callbacks.cpp",
      "src/ui/runtime_element_tree.cpp",
      "src/ui/runtime_entities.cpp",
      "src/ui/runtime_accessibility.cpp",
      "src/ui/runtime_events.cpp",
      "src/ui/runtime_event_control.cpp",
      "src/ui/runtime_event_dispatch.cpp",
      "src/ui/runtime_event_input.cpp",
      "src/ui/runtime_event_keyboard.cpp",
      "src/ui/runtime_event_lifecycle.cpp",
      "src/ui/runtime_event_focus_routes.cpp",
      "src/ui/runtime_event_route_dispatch.cpp",
      "src/ui/runtime_event_text.cpp",
      "src/ui/runtime_event_windows.cpp",
      "src/ui/runtime_event_routes.cpp",
      "src/ui/runtime_actions.cpp",
      "src/ui/runtime_action_dispatch.cpp",
      "src/ui/runtime_action_registration.cpp",
      "src/ui/runtime_action_metadata.cpp",
      "src/ui/runtime_command_palette.cpp",
      "src/ui/runtime_focus.cpp",
      "src/ui/runtime_key_bindings.cpp",
      "src/ui/runtime_windows.cpp",
      "src/ui/runtime_window_activation.cpp",
      "src/ui/runtime_window_records.cpp",
      "src/ui/runtime_window_rendering.cpp",
      "src/ui/window.cpp",
      "src/ui/window_context.cpp",
      "src/ui/runtime_theme.cpp",
      "src/ui/runtime_text.cpp",
      "src/ui/runtime_text_focus.cpp",
      "src/ui/runtime_text_pointer_selection.cpp",
      "src/ui/runtime_clipboard.cpp",
      "src/ui/runtime_platform_services.cpp",
      "src/ui/runtime_scheduling.cpp",
      "src/ui/runtime_timers.cpp",
      "src/ui/runtime_animation_start.cpp",
      "src/ui/runtime_animation_state.cpp",
      "src/ui/runtime_animation_tick.cpp",
      "src/ui/runtime_animations.cpp",
      "src/ui/runtime_tasks.cpp",
      "src/ui/runtime_task_state.cpp",
      "src/ui/runtime_diagnostic_snapshot.cpp",
      "src/ui/runtime_diagnostics.cpp",
      "src/ui/runtime_context.cpp",
      "src/ui/runtime_context_actions.cpp",
      "src/ui/runtime_context_input.cpp",
      "src/ui/runtime_context_platform.cpp",
      "src/ui/runtime_context_scheduling.cpp",
      "src/ui/runtime_context_text.cpp",
      "src/ui/subscription.cpp",
      "src/ui/runtime_subscriptions.cpp",
      "src/ui/app_context_window_options.cpp",
      "src/ui/app_context_services.cpp",
      "src/ui/app_context_commands.cpp",
      "src/ui/app_context_theme.cpp",
      "src/ui/app_runner.cpp",
      "src/ui/app_context.cpp",
  };
  for (const char* source : source_files) {
    if (read_source(source).empty()) {
      return 8;
    }
  }

  const std::string ui_source = read_source("src/ui/ui.cpp");
  if (ui_source.empty()) {
    return 9;
  }
  if (line_count(ui_source) > 250) {
    return 10;
  }
  if (contains(ui_source, "PaintList::") ||
      contains(ui_source, "Result<RenderRecord> render_view") ||
      contains(ui_source, "Subscription::") ||
      contains(ui_source, "WindowRuntime::run") ||
      contains(ui_source, "WindowRuntimeContext::") ||
      contains(ui_source, "AppContext::")) {
    return 11;
  }

  const std::string subscription_source =
      read_source("src/ui/subscription.cpp");
  if (!contains(subscription_source, "Subscription::~Subscription()") ||
      !contains(subscription_source,
                "Subscription::Subscription(Subscription&& other)") ||
      !contains(subscription_source,
                "Subscription& Subscription::operator=(") ||
      !contains(subscription_source, "bool Subscription::connected() const") ||
      !contains(subscription_source, "bool Subscription::release()") ||
      line_count(subscription_source) > 80 ||
      contains(subscription_source, "TaskHandle::") ||
      contains(subscription_source, "AnimationHandle::")) {
    return 122;
  }

  const std::string paint_source = read_source("src/ui/paint.cpp");
  const std::string paint_text_source = read_source("src/ui/paint_text.cpp");
  const std::string paint_image_source = read_source("src/ui/paint_image.cpp");
  if (line_count(paint_source) > 90 ||
      !contains(paint_source, "PaintList::fill_rect(") ||
      !contains(paint_source, "PaintList::fill_rounded_rect(") ||
      contains(paint_source, "PaintList::fill_text(") ||
      contains(paint_source, "PaintList::draw_image(")) {
    return 81;
  }
  if (line_count(paint_text_source) > 100 ||
      !contains(paint_text_source, "PaintList::fill_text(") ||
      !contains(paint_text_source, "PaintList::fill_text_selection(") ||
      !contains(paint_text_source, "PaintList::fill_text_caret(") ||
      contains(paint_text_source, "PaintList::draw_image(") ||
      contains(paint_text_source, "PaintList::fill_rect(")) {
    return 82;
  }
  if (line_count(paint_image_source) > 50 ||
      !contains(paint_image_source, "PaintList::draw_image(") ||
      contains(paint_image_source, "PaintList::fill_text(") ||
      contains(paint_image_source, "PaintList::fill_rect(")) {
    return 83;
  }

  const std::string runtime_core_source =
      read_source("src/ui/runtime_core.cpp");
  const std::string runtime_run_source =
      read_source("src/ui/runtime_run.cpp");
  const std::string runtime_shutdown_source =
      read_source("src/ui/runtime_shutdown.cpp");
  if (line_count(runtime_core_source) > 80 ||
      !contains(runtime_core_source, "WindowRuntime::WindowRuntime(") ||
      contains(runtime_core_source, "WindowRuntime::~WindowRuntime(") ||
      contains(runtime_core_source, "WindowRuntime::run(") ||
      contains(runtime_core_source, "WindowRuntime::fail_and_quit(") ||
      contains(runtime_core_source, "AppOpenedWindow WindowRuntime::open_window(") ||
      contains(runtime_core_source,
               "void WindowRuntime::activate_native_window_for_record(") ||
      contains(runtime_core_source,
               "void WindowRuntime::handle_redraw_for_record(") ||
      contains(runtime_core_source,
               "WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(") ||
      contains(runtime_core_source, "void WindowRuntime::handle_redraw(") ||
      contains(runtime_core_source,
               "ViewId WindowRuntime::register_view(") ||
      contains(runtime_core_source,
               "bool WindowRuntime::notify_entity_changed(") ||
      contains(runtime_core_source,
               "void WindowRuntime::set_element_tree(")) {
    return 32;
  }
  if (line_count(runtime_run_source) > 140 ||
      !contains(runtime_run_source, "int WindowRuntime::run(") ||
      !contains(runtime_run_source, "application_.create_window(") ||
      !contains(runtime_run_source, "renderer_factory_(") ||
      !contains(runtime_run_source, "void WindowRuntime::fail_and_quit(") ||
      contains(runtime_run_source, "WindowRuntime::~WindowRuntime(")) {
    return 84;
  }
  if (line_count(runtime_shutdown_source) > 70 ||
      !contains(runtime_shutdown_source, "WindowRuntime::~WindowRuntime(") ||
      !contains(runtime_shutdown_source, "task.worker.request_stop(") ||
      contains(runtime_shutdown_source, "WindowRuntime::run(")) {
    return 85;
  }

  const std::string render_view_source =
      read_source("src/ui/render_view.cpp");
  if (line_count(render_view_source) > 180 ||
      contains(render_view_source, "StyledElement::paint(") ||
      contains(render_view_source, "ButtonElement::paint(") ||
      contains(render_view_source, "ScrollableListElement::paint(") ||
      contains(render_view_source, "LabelElement::paint(") ||
      contains(render_view_source, "TextElement::paint(") ||
      contains(render_view_source, "draw_image(") ||
      contains(render_view_source, "draw_text_selection(") ||
      contains(render_view_source, "draw_text_caret(") ||
      contains(render_view_source, "draw_text(") ||
      contains(render_view_source, "draw_rounded_rect(")) {
    return 57;
  }

  const std::string element_paint_source =
      read_source("src/ui/element_paint.cpp");
  const std::string element_style_paint_source =
      read_source("src/ui/element_style_paint.cpp");
  const std::string element_button_paint_source =
      read_source("src/ui/element_button_paint.cpp");
  const std::string element_scroll_paint_source =
      read_source("src/ui/element_scroll_paint.cpp");
  const std::string element_text_paint_source =
      read_source("src/ui/element_text_paint.cpp");
  if (line_count(element_paint_source) > 20 ||
      contains(element_paint_source, "StyledElement::paint(") ||
      contains(element_paint_source, "ButtonElement::paint(") ||
      contains(element_paint_source, "ScrollableListElement::paint(") ||
      contains(element_paint_source, "LabelElement::paint(") ||
      contains(element_paint_source, "TextElement::paint(") ||
      line_count(element_text_paint_source) > 100 ||
      !contains(element_style_paint_source, "StyledElement::paint(") ||
      !contains(element_button_paint_source, "ButtonElement::paint(") ||
      !contains(element_scroll_paint_source,
                "ScrollableListElement::paint(") ||
      !contains(element_text_paint_source, "LabelElement::paint(") ||
      !contains(element_text_paint_source, "TextElement::paint(") ||
      contains(element_paint_source, "submit_paint_command_to_frame(")) {
    return 58;
  }

  const std::string render_view_commands_source =
      read_source("src/ui/render_view_commands.cpp");
  if (line_count(render_view_commands_source) > 180 ||
      !contains(render_view_commands_source, "submit_paint_command_to_frame(") ||
      !contains(render_view_commands_source, "draw_image(") ||
      !contains(render_view_commands_source, "draw_text_selection(") ||
      !contains(render_view_commands_source, "draw_text_caret(") ||
      !contains(render_view_commands_source, "draw_text(") ||
      !contains(render_view_commands_source, "draw_rounded_rect(") ||
      contains(render_view_commands_source, "StyledElement::paint(")) {
    return 59;
  }

  const std::string runtime_rendering_source =
      read_source("src/ui/runtime_rendering.cpp");
  if (!contains(runtime_rendering_source,
                "void WindowRuntime::handle_redraw(") ||
      !contains(runtime_rendering_source,
                "Result<void> WindowRuntime::resize_surface(")) {
    return 42;
  }

  const std::string runtime_views_source =
      read_source("src/ui/runtime_views.cpp");
  if (!contains(runtime_views_source,
                "ViewId WindowRuntime::register_view(") ||
      !contains(runtime_views_source,
                "View* WindowRuntime::find_view(") ||
      !contains(runtime_views_source,
                "std::optional<ViewId> WindowRuntime::upgrade_view(")) {
    return 43;
  }

  const std::string runtime_callbacks_source =
      read_source("src/ui/runtime_callbacks.cpp");
  if (!contains(runtime_callbacks_source,
                "void WindowRuntime::set_after_frame_callback(") ||
      !contains(runtime_callbacks_source,
                "void WindowRuntime::set_error_callback(")) {
    return 44;
  }

  const std::string runtime_element_tree_source =
      read_source("src/ui/runtime_element_tree.cpp");
  if (!contains(runtime_element_tree_source,
                "void WindowRuntime::set_element_tree(") ||
      !contains(runtime_element_tree_source,
                "const Element* WindowRuntime::element_root(") ||
      !contains(runtime_element_tree_source,
                "void WindowRuntime::capture_pointer(")) {
    return 45;
  }

  const std::string runtime_entities_source =
      read_source("src/ui/runtime_entities.cpp");
  if (!contains(runtime_entities_source,
                "bool WindowRuntime::notify_entity_changed(")) {
    return 46;
  }

  const std::string runtime_windows_source =
      read_source("src/ui/runtime_windows.cpp");
  if (line_count(runtime_windows_source) > 130 ||
      !contains(runtime_windows_source,
                "AppOpenedWindow WindowRuntime::open_window(") ||
      !contains(runtime_windows_source,
                "std::span<const AppOpenedWindow> "
                "WindowRuntime::app_opened_windows(") ||
      contains(runtime_windows_source,
               "void WindowRuntime::activate_native_window_for_record(") ||
      contains(runtime_windows_source,
               "void WindowRuntime::handle_redraw_for_record(") ||
      contains(runtime_windows_source,
               "WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(") ||
      contains(runtime_windows_source,
               "void WindowRuntime::cleanup_closed_additional_window(")) {
    return 33;
  }

  const std::string runtime_window_activation_source =
      read_source("src/ui/runtime_window_activation.cpp");
  if (line_count(runtime_window_activation_source) > 100 ||
      !contains(runtime_window_activation_source,
                "void WindowRuntime::activate_native_window_for_record(") ||
      !contains(runtime_window_activation_source,
                "application_.create_window(") ||
      !contains(runtime_window_activation_source, "renderer_factory_(") ||
      contains(runtime_window_activation_source,
               "void WindowRuntime::handle_redraw_for_record(")) {
    return 65;
  }

  const std::string runtime_window_rendering_source =
      read_source("src/ui/runtime_window_rendering.cpp");
  if (line_count(runtime_window_rendering_source) > 120 ||
      !contains(runtime_window_rendering_source,
                "void WindowRuntime::handle_redraw_for_record(") ||
      !contains(runtime_window_rendering_source,
                "ViewContext render_context = context_for_record(record)") ||
      !contains(runtime_window_rendering_source,
                "auto result = render_view(") ||
      contains(runtime_window_rendering_source,
               "void WindowRuntime::activate_native_window_for_record(")) {
    return 66;
  }

  const std::string runtime_window_records_source =
      read_source("src/ui/runtime_window_records.cpp");
  if (line_count(runtime_window_records_source) > 150 ||
      !contains(runtime_window_records_source,
                "WindowRuntimeContext WindowRuntime::context_for_record(") ||
      !contains(runtime_window_records_source,
                "void WindowRuntime::cleanup_closed_additional_window(") ||
      !contains(runtime_window_records_source,
                "WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(") ||
      !contains(runtime_window_records_source,
                "WindowRuntimeId WindowRuntime::allocate_window_runtime_id(") ||
      contains(runtime_window_records_source,
               "AppOpenedWindow WindowRuntime::open_window(")) {
    return 67;
  }

  const std::string window_source = read_source("src/ui/window.cpp");
  const std::string window_context_source =
      read_source("src/ui/window_context.cpp");
  if (!contains(window_source, "Window::descriptor()") ||
      !contains(window_source, "Window::viewport_size()") ||
      !contains(window_context_source, "WindowContextCapability::window()") ||
      !contains(window_context_source,
                "WindowContextCapability::current_window()") ||
      !contains(window_context_source,
                "WindowContextCapability::request_render()") ||
      !contains(window_context_source,
                "WindowRuntimeContext::window()") ||
      !contains(window_context_source,
                "WindowRuntimeContext::current_window()") ||
      !contains(window_context_source,
                "WindowRuntimeContext::window_context()") ||
      line_count(window_source) > 120 ||
      line_count(window_context_source) > 120) {
    return 68;
  }

  const std::string runtime_events_source =
      read_source("src/ui/runtime_events.cpp");
  if (line_count(runtime_events_source) > 90) {
    return 12;
  }
  if (contains(runtime_events_source,
               "bool WindowRuntime::paste_clipboard_text(") ||
      contains(runtime_events_source,
               "bool WindowRuntime::copy_selection_to_clipboard(") ||
      contains(runtime_events_source,
               "bool WindowRuntime::cut_selection_to_clipboard(") ||
      contains(runtime_events_source, "void WindowRuntime::set_clipboard(") ||
      contains(runtime_events_source,
               "bool WindowRuntime::apply_text_pointer_selection(") ||
      contains(runtime_events_source,
               "TextModel* WindowRuntime::focused_text_model(") ||
      contains(runtime_events_source, "void WindowRuntime::bind_text_model(") ||
      contains(runtime_events_source, "void WindowRuntime::register_action(") ||
      contains(runtime_events_source,
               "ActionDispatchResult WindowRuntime::dispatch_action(") ||
      contains(runtime_events_source,
               "void WindowRuntime::register_command_palette_entry(") ||
      contains(runtime_events_source, "void WindowRuntime::bind_key(") ||
      contains(runtime_events_source,
               "void WindowRuntime::record_lifecycle_event(") ||
      contains(runtime_events_source,
               "void WindowRuntime::handle_window_control_event(") ||
      contains(runtime_events_source,
               "void WindowRuntime::dispatch_view_event_for_record(") ||
      contains(runtime_events_source,
               "void WindowRuntime::handle_native_additional_window_event(") ||
      contains(runtime_events_source,
               "void WindowRuntime::refresh_route_ancestry(") ||
      contains(runtime_events_source,
               "EventResult WindowRuntime::dispatch_routed_element_event(") ||
      contains(runtime_events_source, "hit_test_runtime_element_root(") ||
      contains(runtime_events_source, "for (const KeyBinding&") ||
      contains(runtime_events_source, "ImeComposition") ||
      contains(runtime_events_source, "view_.handle_event(") ||
      contains(runtime_events_source,
               "WindowRuntimeContext WindowRuntime::context()")) {
    return 13;
  }

  const std::string runtime_event_control_source =
      read_source("src/ui/runtime_event_control.cpp");
  if (line_count(runtime_event_control_source) > 90 ||
      !contains(runtime_event_control_source,
                "bool WindowRuntime::handle_window_control_event(") ||
      !contains(runtime_event_control_source, "WindowCloseRequested") ||
      !contains(runtime_event_control_source, "WindowActivated") ||
      !contains(runtime_event_control_source, "handle_resize(") ||
      !contains(runtime_event_control_source, "handle_redraw()") ||
      !contains(runtime_event_control_source, "handle_wakeup()") ||
      contains(runtime_event_control_source, "hit_test_runtime_element_root(") ||
      contains(runtime_event_control_source, "ImeComposition")) {
    return 68;
  }

  const std::string runtime_event_input_source =
      read_source("src/ui/runtime_event_input.cpp");
  if (line_count(runtime_event_input_source) > 160 ||
      !contains(runtime_event_input_source,
                "void WindowRuntime::update_input_state_for_event(") ||
      !contains(runtime_event_input_source,
                "std::optional<ElementId> WindowRuntime::hit_test_target_for_event(") ||
      !contains(runtime_event_input_source,
                "void WindowRuntime::update_hover_cursor_for_event(") ||
      !contains(runtime_event_input_source,
                "void WindowRuntime::resolve_event_route_target(") ||
      !contains(runtime_event_input_source,
                "void WindowRuntime::apply_focus_activation_for_event(") ||
      !contains(runtime_event_input_source, "hit_test_runtime_element_root(") ||
      !contains(runtime_event_input_source,
                "apply_pointer_capture_owner_to_route(") ||
      contains(runtime_event_input_source, "for (const KeyBinding&") ||
      contains(runtime_event_input_source, "ImeComposition")) {
    return 69;
  }

  const std::string runtime_event_keyboard_source =
      read_source("src/ui/runtime_event_keyboard.cpp");
  if (line_count(runtime_event_keyboard_source) > 90 ||
      !contains(runtime_event_keyboard_source,
                "void WindowRuntime::apply_keyboard_bindings_for_event(") ||
      !contains(runtime_event_keyboard_source, "focus_next_element(") ||
      !contains(runtime_event_keyboard_source, "dispatch_action(") ||
      !contains(runtime_event_keyboard_source, "TextEditBinding") ||
      contains(runtime_event_keyboard_source, "ImeComposition") ||
      contains(runtime_event_keyboard_source, "view_.handle_event(")) {
    return 70;
  }

  const std::string runtime_event_text_source =
      read_source("src/ui/runtime_event_text.cpp");
  if (line_count(runtime_event_text_source) > 100 ||
      !contains(runtime_event_text_source,
                "void WindowRuntime::apply_text_input_for_event(") ||
      !contains(runtime_event_text_source, "TextInput") ||
      !contains(runtime_event_text_source, "ImeComposition") ||
      !contains(runtime_event_text_source, "ImeDeleteSurroundingText") ||
      !contains(runtime_event_text_source, "focused_text_model(") ||
      contains(runtime_event_text_source, "for (const KeyBinding&") ||
      contains(runtime_event_text_source, "view_.handle_event(")) {
    return 71;
  }

  const std::string runtime_event_dispatch_source =
      read_source("src/ui/runtime_event_dispatch.cpp");
  if (line_count(runtime_event_dispatch_source) > 120 ||
      !contains(runtime_event_dispatch_source,
                "EventResult WindowRuntime::dispatch_current_event_route(") ||
      !contains(runtime_event_dispatch_source,
                "void WindowRuntime::finish_event_dispatch(") ||
      !contains(runtime_event_dispatch_source,
                "apply_text_pointer_selection(") ||
      !contains(runtime_event_dispatch_source,
                "dispatch_routed_element_event(") ||
      !contains(runtime_event_dispatch_source, "view_.handle_event(") ||
      !contains(runtime_event_dispatch_source, "last_event_dispatch_") ||
      contains(runtime_event_dispatch_source, "ImeComposition")) {
    return 72;
  }

  const std::string runtime_event_lifecycle_source =
      read_source("src/ui/runtime_event_lifecycle.cpp");
  if (!contains(runtime_event_lifecycle_source,
                "void WindowRuntime::record_lifecycle_event(") ||
      !contains(runtime_event_lifecycle_source,
                "void WindowRuntime::record_lifecycle_event_for_record(")) {
    return 34;
  }

  const std::string runtime_event_windows_source =
      read_source("src/ui/runtime_event_windows.cpp");
  if (!contains(runtime_event_windows_source,
                "void WindowRuntime::dispatch_view_event_for_record(") ||
      !contains(runtime_event_windows_source,
                "void WindowRuntime::handle_native_additional_window_event(")) {
    return 35;
  }

  const std::string runtime_event_routes_source =
      read_source("src/ui/runtime_event_routes.cpp");
  if (line_count(runtime_event_routes_source) > 120 ||
      !contains(runtime_event_routes_source,
                "void WindowRuntime::refresh_route_ancestry(") ||
      !contains(runtime_event_routes_source,
                "std::vector<ElementId> WindowRuntime::element_ancestry_for(") ||
      !contains(runtime_event_routes_source,
                "Element* WindowRuntime::routed_element(") ||
      contains(runtime_event_routes_source,
               "bool WindowRuntime::focus_next_element(") ||
      contains(runtime_event_routes_source,
               "EventResult WindowRuntime::dispatch_routed_element_event(")) {
    return 36;
  }

  const std::string runtime_event_focus_routes_source =
      read_source("src/ui/runtime_event_focus_routes.cpp");
  if (line_count(runtime_event_focus_routes_source) > 80 ||
      !contains(runtime_event_focus_routes_source,
                "bool WindowRuntime::focus_next_element(") ||
      !contains(runtime_event_focus_routes_source, "enabled_preorder_ids") ||
      contains(runtime_event_focus_routes_source,
               "EventResult WindowRuntime::dispatch_routed_element_event(")) {
    return 81;
  }

  const std::string runtime_event_route_dispatch_source =
      read_source("src/ui/runtime_event_route_dispatch.cpp");
  if (line_count(runtime_event_route_dispatch_source) > 90 ||
      !contains(runtime_event_route_dispatch_source,
                "WindowRuntime::action_dispatch_view_id(") ||
      !contains(runtime_event_route_dispatch_source,
                "WindowRuntime::scroll_state_for_route(") ||
      !contains(runtime_event_route_dispatch_source,
                "EventResult WindowRuntime::dispatch_routed_element_event(") ||
      contains(runtime_event_route_dispatch_source,
               "std::vector<ElementId> WindowRuntime::element_ancestry_for(")) {
    return 82;
  }

  const std::string runtime_context_source =
      read_source("src/ui/runtime_context.cpp");
  if (!contains(runtime_context_source,
                "WindowRuntimeContext WindowRuntime::context()") ||
      line_count(runtime_context_source) > 80 ||
      contains(runtime_context_source, "WindowRuntimeContext::capture_pointer(") ||
      contains(runtime_context_source, "WindowRuntimeContext::register_action(") ||
      contains(runtime_context_source, "WindowRuntimeContext::bind_text_model(") ||
      contains(runtime_context_source, "WindowRuntimeContext::schedule_timer(") ||
      contains(runtime_context_source, "WindowRuntimeContext::install_native_menu(")) {
    return 37;
  }

  const std::string runtime_context_input_source =
      read_source("src/ui/runtime_context_input.cpp");
  if (!contains(runtime_context_input_source,
                "ViewId WindowRuntimeContext::allocate_view_id() const") ||
      !contains(runtime_context_input_source,
                "void WindowRuntimeContext::capture_pointer(") ||
      !contains(runtime_context_input_source,
                "void WindowRuntimeContext::request_keyboard_focus(") ||
      !contains(runtime_context_input_source,
                "FocusHandle WindowRuntimeContext::focus_handle(")) {
    return 47;
  }

  const std::string async_context_source =
      read_source("src/ui/async_context.cpp");
  if (!contains(async_context_source,
                "void AsyncContextCapability::defer(") ||
      !contains(async_context_source,
                "TimerId AsyncContextCapability::schedule_timer(") ||
      !contains(async_context_source,
                "AnimationHandle AsyncContextCapability::start_animation(") ||
      !contains(async_context_source,
                "TaskHandle AsyncContextCapability::spawn_background_task(") ||
      !contains(async_context_source,
                "AsyncContextCapability WindowRuntimeContext::async_context() const") ||
      line_count(async_context_source) > 110 ||
      contains(async_context_source, "ElementContextCapability")) {
    return 119;
  }

  const std::string test_context_source =
      read_source("src/ui/test_context.cpp");
  if (!contains(test_context_source,
                "WindowRuntimeId TestContextCapability::runtime_id() const") ||
      !contains(test_context_source,
                "ViewInputState TestContextCapability::input_state() const") ||
      !contains(test_context_source,
                "void TestContextCapability::advance_time(") ||
      !contains(test_context_source,
                "bool TestContextCapability::complete_task(") ||
      !contains(test_context_source,
                "TestContextCapability WindowRuntimeContext::test_context() const") ||
      line_count(test_context_source) > 110 ||
      contains(test_context_source, "AsyncContextCapability") ||
      contains(test_context_source, "ElementContextCapability")) {
    return 120;
  }

  const std::string element_context_source =
      read_source("src/ui/element_context.cpp");
  if (!contains(element_context_source,
                "ElementId ElementContextCapability::element_id() const") ||
      !contains(element_context_source,
                "void ElementContextCapability::capture_pointer() const") ||
      !contains(element_context_source,
                "void ElementContextCapability::set_cursor(") ||
      !contains(element_context_source,
                "ElementContextCapability WindowRuntimeContext::element_context(") ||
      line_count(element_context_source) > 80 ||
      contains(element_context_source, "WindowContextCapability")) {
    return 118;
  }

  const std::string runtime_context_actions_source =
      read_source("src/ui/runtime_context_actions.cpp");
  if (!contains(runtime_context_actions_source,
                "void WindowRuntimeContext::register_action(") ||
      !contains(runtime_context_actions_source,
                "ActionDispatchResult WindowRuntimeContext::dispatch_action(") ||
      !contains(runtime_context_actions_source,
                "void WindowRuntimeContext::register_command_palette_entry(") ||
      !contains(runtime_context_actions_source,
                "void WindowRuntimeContext::bind_key(")) {
    return 48;
  }

  const std::string runtime_context_text_source =
      read_source("src/ui/runtime_context_text.cpp");
  if (!contains(runtime_context_text_source,
                "void WindowRuntimeContext::bind_text_model(") ||
      !contains(runtime_context_text_source,
                "bool WindowRuntimeContext::mutate_focused_text_model(") ||
      !contains(runtime_context_text_source,
                "std::optional<ImeCandidateRect> "
                "WindowRuntimeContext::focused_text_ime_rect()") ||
      !contains(runtime_context_text_source,
                "bool WindowRuntimeContext::paste_clipboard_text() const")) {
    return 49;
  }

  const std::string runtime_context_scheduling_source =
      read_source("src/ui/runtime_context_scheduling.cpp");
  if (!contains(runtime_context_scheduling_source,
                "void WindowRuntimeContext::request_render() const") ||
      !contains(runtime_context_scheduling_source,
                "TimerId WindowRuntimeContext::schedule_timer(") ||
      !contains(runtime_context_scheduling_source,
                "AnimationHandle WindowRuntimeContext::start_animation(") ||
      !contains(runtime_context_scheduling_source,
                "TaskHandle WindowRuntimeContext::spawn_background_task(") ||
      !contains(runtime_context_scheduling_source,
                "RuntimeDiagnosticsSnapshot "
                "WindowRuntimeContext::diagnostics_snapshot() const")) {
    return 50;
  }

  const std::string runtime_context_platform_source =
      read_source("src/ui/runtime_context_platform.cpp");
  if (!contains(runtime_context_platform_source,
                "NativeMenuInstallation "
                "WindowRuntimeContext::install_native_menu(") ||
      !contains(runtime_context_platform_source,
                "NativeFileDialogResult "
                "WindowRuntimeContext::show_native_file_dialog(") ||
      !contains(runtime_context_platform_source,
                "void WindowRuntimeContext::set_window_theme(") ||
      !contains(runtime_context_platform_source,
                "std::optional<Color> WindowRuntimeContext::theme_color(")) {
    return 51;
  }

  const std::string runtime_actions_source =
      read_source("src/ui/runtime_actions.cpp");
  if (line_count(runtime_actions_source) > 40 ||
      contains(runtime_actions_source,
               "void WindowRuntime::request_keyboard_focus(") ||
      contains(runtime_actions_source,
               "ActionDispatchResult WindowRuntime::dispatch_action(") ||
      contains(runtime_actions_source,
               "void WindowRuntime::register_command_palette_entry(") ||
      contains(runtime_actions_source, "void WindowRuntime::bind_key(") ||
      contains(runtime_actions_source,
               "AccessibilityTreeSnapshot WindowRuntime::accessibility_snapshot(") ||
      contains(runtime_actions_source,
               "void WindowRuntime::set_element_cursor(")) {
    return 31;
  }

  const std::string runtime_focus_source =
      read_source("src/ui/runtime_focus.cpp");
  if (line_count(runtime_focus_source) > 110 ||
      !contains(runtime_focus_source,
                "void WindowRuntime::request_keyboard_focus(") ||
      !contains(runtime_focus_source,
                "void WindowRuntime::release_keyboard_focus(") ||
      !contains(runtime_focus_source,
                "ViewInputState WindowRuntime::input_state(") ||
      !contains(runtime_focus_source,
                "void WindowRuntime::set_element_cursor(") ||
      contains(runtime_focus_source, "dispatch_action(") ||
      contains(runtime_focus_source, "register_command_palette_entry(")) {
    return 73;
  }

  const std::string runtime_action_dispatch_source =
      read_source("src/ui/runtime_action_dispatch.cpp");
  const std::string runtime_action_registration_source =
      read_source("src/ui/runtime_action_registration.cpp");
  const std::string runtime_action_metadata_source =
      read_source("src/ui/runtime_action_metadata.cpp");
  if (line_count(runtime_action_dispatch_source) > 100 ||
      !contains(runtime_action_dispatch_source,
                "ActionDispatchResult WindowRuntime::dispatch_action(") ||
      !contains(runtime_action_dispatch_source,
                "WindowRuntime::last_action_dispatch(") ||
      !contains(runtime_action_dispatch_source,
                "action_registration_enabled(") ||
      !contains(runtime_action_dispatch_source,
                "action_result_stops_bubbling") ||
      !contains(runtime_action_dispatch_source,
                "!action_result_stops_bubbling(result)") ||
      contains(runtime_action_dispatch_source,
               "void WindowRuntime::register_action(") ||
      contains(runtime_action_dispatch_source,
               "void WindowRuntime::register_view_action(") ||
      contains(runtime_action_dispatch_source,
               "void WindowRuntime::register_focused_element_action(") ||
      contains(runtime_action_dispatch_source,
               "register_command_palette_entry(") ||
      contains(runtime_action_dispatch_source,
               "action_registrations_for_scope(") ||
      contains(runtime_action_dispatch_source, "KeyBinding") ||
      contains(runtime_action_dispatch_source, "bind_text_edit_action(")) {
    return 74;
  }
  if (line_count(runtime_action_registration_source) > 120 ||
      !contains(runtime_action_registration_source,
                "void WindowRuntime::register_action(") ||
      !contains(runtime_action_registration_source,
                "void WindowRuntime::register_view_action(") ||
      !contains(runtime_action_registration_source,
                "void WindowRuntime::register_focused_element_action(") ||
      !contains(runtime_action_registration_source,
                "ActionRegistrationOptions options") ||
      !contains(runtime_action_registration_source, ".enabled = options.enabled") ||
      contains(runtime_action_registration_source,
               "ActionDispatchResult WindowRuntime::dispatch_action(") ||
      contains(runtime_action_registration_source,
               "register_command_palette_entry(") ||
      contains(runtime_action_registration_source,
               "action_registrations_for_scope(") ||
      contains(runtime_action_registration_source, "bind_text_edit_action(")) {
    return 77;
  }
  if (line_count(runtime_action_metadata_source) > 90 ||
      !contains(runtime_action_metadata_source,
                "WindowRuntime::upsert_action_registration(") ||
      !contains(runtime_action_metadata_source,
                "WindowRuntime::action_registrations()") ||
      !contains(runtime_action_metadata_source,
                "WindowRuntime::action_registrations_for_scope(") ||
      !contains(runtime_action_metadata_source,
                "WindowRuntime::action_registrations_for_enabled(") ||
      !contains(runtime_action_metadata_source,
                "WindowRuntime::action_registration_enabled(") ||
      contains(runtime_action_metadata_source,
               "ActionDispatchResult WindowRuntime::dispatch_action(") ||
      contains(runtime_action_metadata_source,
               "register_command_palette_entry(") ||
      contains(runtime_action_metadata_source, "KeyBinding")) {
    return 76;
  }

  const std::string runtime_command_palette_source =
      read_source("src/ui/runtime_command_palette.cpp");
  if (line_count(runtime_command_palette_source) > 100 ||
      !contains(runtime_command_palette_source,
                "void WindowRuntime::register_command_palette_entry(") ||
      !contains(runtime_command_palette_source,
                "WindowRuntime::command_palette_entries(") ||
      !contains(runtime_command_palette_source,
                "WindowRuntime::dispatch_command_palette_entry(") ||
      !contains(runtime_command_palette_source,
                "WindowRuntime::dispatch_command_palette_action(") ||
      contains(runtime_command_palette_source, "request_keyboard_focus(") ||
      contains(runtime_command_palette_source, "bind_text_edit_action(")) {
    return 75;
  }

  const std::string runtime_key_bindings_source =
      read_source("src/ui/runtime_key_bindings.cpp");
  if (line_count(runtime_key_bindings_source) > 60 ||
      !contains(runtime_key_bindings_source,
                "void WindowRuntime::bind_key(") ||
      !contains(runtime_key_bindings_source,
                "void WindowRuntime::bind_text_edit_action(") ||
      contains(runtime_key_bindings_source, "dispatch_action(") ||
      contains(runtime_key_bindings_source,
               "register_command_palette_entry(")) {
    return 76;
  }

  const std::string runtime_text_source = read_source("src/ui/runtime_text.cpp");
  const std::string runtime_text_focus_source =
      read_source("src/ui/runtime_text_focus.cpp");
  const std::string runtime_text_pointer_selection_source =
      read_source("src/ui/runtime_text_pointer_selection.cpp");
  if (line_count(runtime_text_source) > 70 ||
      !contains(runtime_text_source, "void WindowRuntime::bind_text_model(") ||
      !contains(runtime_text_source, "WindowRuntime::routed_text_input(") ||
      contains(runtime_text_source,
               "bool WindowRuntime::apply_text_pointer_selection(") ||
      contains(runtime_text_source,
               "TextModel* WindowRuntime::focused_text_model(") ||
      contains(runtime_text_source,
               "WindowRuntime::focused_text_ime_rect()")) {
    return 14;
  }
  if (line_count(runtime_text_focus_source) > 90 ||
      !contains(runtime_text_focus_source,
                "TextModel* WindowRuntime::focused_text_model(") ||
      !contains(runtime_text_focus_source,
                "std::optional<ImeCandidateRect> "
                "WindowRuntime::focused_text_ime_rect()") ||
      contains(runtime_text_focus_source,
               "bool WindowRuntime::apply_text_pointer_selection(") ||
      contains(runtime_text_focus_source, "void WindowRuntime::bind_text_model(")) {
    return 86;
  }
  if (line_count(runtime_text_pointer_selection_source) > 100 ||
      !contains(runtime_text_pointer_selection_source,
                "bool WindowRuntime::apply_text_pointer_selection(") ||
      !contains(runtime_text_pointer_selection_source,
                "WindowRuntime::text_offset_for_point(") ||
      contains(runtime_text_pointer_selection_source,
               "TextModel* WindowRuntime::focused_text_model(") ||
      contains(runtime_text_pointer_selection_source,
               "WindowRuntime::focused_text_ime_rect()")) {
    return 87;
  }

  const std::string runtime_clipboard_source =
      read_source("src/ui/runtime_clipboard.cpp");
  if (!contains(runtime_clipboard_source,
                "void WindowRuntime::set_clipboard(") ||
      !contains(runtime_clipboard_source,
                "bool WindowRuntime::paste_clipboard_text(") ||
      !contains(runtime_clipboard_source,
                "bool WindowRuntime::copy_selection_to_clipboard(") ||
      !contains(runtime_clipboard_source,
                "bool WindowRuntime::cut_selection_to_clipboard(")) {
    return 15;
  }

  const std::string runtime_scheduling_source =
      read_source("src/ui/runtime_scheduling.cpp");
  if (line_count(runtime_scheduling_source) > 220 ||
      contains(runtime_scheduling_source,
               "TimerId WindowRuntime::schedule_timer(") ||
      contains(runtime_scheduling_source,
               "AnimationHandle WindowRuntime::start_animation(") ||
      contains(runtime_scheduling_source,
               "TaskHandle WindowRuntime::spawn_task(") ||
      contains(runtime_scheduling_source,
               "TaskHandle WindowRuntime::spawn_background_task(")) {
    return 38;
  }

  const std::string runtime_timers_source =
      read_source("src/ui/runtime_timers.cpp");
  if (!contains(runtime_timers_source,
                "TimerId WindowRuntime::schedule_timer(") ||
      !contains(runtime_timers_source,
                "TimerId WindowRuntime::schedule_repeating_timer(") ||
      !contains(runtime_timers_source,
                "bool WindowRuntime::cancel_timer(") ||
      !contains(runtime_timers_source,
                "void WindowRuntime::fire_due_timers(")) {
    return 39;
  }

  const std::string runtime_animation_start_source =
      read_source("src/ui/runtime_animation_start.cpp");
  const std::string runtime_animation_state_source =
      read_source("src/ui/runtime_animation_state.cpp");
  const std::string runtime_animation_tick_source =
      read_source("src/ui/runtime_animation_tick.cpp");
  const std::string runtime_animations_source =
      read_source("src/ui/runtime_animations.cpp");
  if (runtime_animation_start_source.empty() ||
      runtime_animation_state_source.empty() ||
      runtime_animation_tick_source.empty() ||
      runtime_animations_source.empty()) {
    return 40;
  }
  if (line_count(runtime_animation_start_source) > 80 ||
      !contains(runtime_animation_start_source,
                "AnimationHandle WindowRuntime::start_animation(") ||
      contains(runtime_animation_start_source,
               "WindowRuntime::animation_snapshot(") ||
      contains(runtime_animation_start_source,
               "WindowRuntime::tick_animation(")) {
    return 88;
  }
  if (line_count(runtime_animation_state_source) > 100 ||
      !contains(runtime_animation_state_source,
                "std::optional<AnimationSnapshot> "
                "WindowRuntime::animation_snapshot(") ||
      !contains(runtime_animation_state_source,
                "bool WindowRuntime::cancel_animation(") ||
      !contains(runtime_animation_state_source,
                "bool WindowRuntime::animation_active(") ||
      !contains(runtime_animation_state_source,
                "bool WindowRuntime::animation_complete(") ||
      contains(runtime_animation_state_source,
               "AnimationHandle WindowRuntime::start_animation(") ||
      contains(runtime_animation_state_source,
               "void WindowRuntime::tick_animation(")) {
    return 89;
  }
  if (line_count(runtime_animation_tick_source) > 70 ||
      !contains(runtime_animation_tick_source,
                "void WindowRuntime::tick_animation(") ||
      !contains(runtime_animation_tick_source, "callback(context(), *snapshot)") ||
      contains(runtime_animation_tick_source,
               "AnimationHandle WindowRuntime::start_animation(") ||
      contains(runtime_animation_tick_source,
               "bool WindowRuntime::cancel_animation(")) {
    return 90;
  }
  if (line_count(runtime_animations_source) > 20 ||
      contains(runtime_animations_source,
               "AnimationHandle WindowRuntime::start_animation(") ||
      contains(runtime_animations_source,
               "WindowRuntime::animation_snapshot(") ||
      contains(runtime_animations_source,
               "WindowRuntime::cancel_animation(") ||
      contains(runtime_animations_source,
               "void WindowRuntime::tick_animation(")) {
    return 91;
  }

  const std::string runtime_tasks_source =
      read_source("src/ui/runtime_tasks.cpp");
  if (line_count(runtime_tasks_source) > 150 ||
      !contains(runtime_tasks_source,
                "TaskHandle WindowRuntime::spawn_task(") ||
      !contains(runtime_tasks_source,
                "TaskHandle WindowRuntime::spawn_background_task(") ||
      !contains(runtime_tasks_source,
                "bool WindowRuntime::complete_task(") ||
      !contains(runtime_tasks_source,
                "void WindowRuntime::drain_task_completions(") ||
      contains(runtime_tasks_source,
               "bool WindowRuntime::cancel_task(") ||
      contains(runtime_tasks_source,
               "bool WindowRuntime::task_active(")) {
    return 41;
  }

  const std::string runtime_task_state_source =
      read_source("src/ui/runtime_task_state.cpp");
  if (line_count(runtime_task_state_source) > 110 ||
      !contains(runtime_task_state_source,
                "bool WindowRuntime::task_active(") ||
      !contains(runtime_task_state_source,
                "bool WindowRuntime::task_complete(") ||
      !contains(runtime_task_state_source,
                "bool WindowRuntime::task_cancelled(") ||
      !contains(runtime_task_state_source,
                "bool WindowRuntime::cancel_task(") ||
      contains(runtime_task_state_source,
               "TaskHandle WindowRuntime::spawn_task(") ||
      contains(runtime_task_state_source,
               "void WindowRuntime::drain_task_completions(")) {
    return 80;
  }

  const std::string runtime_diagnostics_source =
      read_source("src/ui/runtime_diagnostics.cpp");
  if (line_count(runtime_diagnostics_source) > 80 ||
      contains(runtime_diagnostics_source,
               "RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot(") ||
      contains(runtime_diagnostics_source,
               "NativeMenuInstallation WindowRuntime::install_native_menu(") ||
      contains(runtime_diagnostics_source,
               "void WindowRuntime::set_app_theme(") ||
      contains(runtime_diagnostics_source,
               "void WindowRuntime::update_platform_accessibility_tree(") ||
      contains(runtime_diagnostics_source,
               "void WindowRuntime::apply_focused_text_ime_placement(")) {
    return 60;
  }

  const std::string runtime_diagnostic_snapshot_source =
      read_source("src/ui/runtime_diagnostic_snapshot.cpp");
  if (line_count(runtime_diagnostic_snapshot_source) > 150 ||
      !contains(runtime_diagnostic_snapshot_source,
                "RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot(") ||
      !contains(runtime_diagnostic_snapshot_source,
                "WindowRuntime::RuntimeTaskDiagnostics "
                "WindowRuntime::task_diagnostics(") ||
      contains(runtime_diagnostic_snapshot_source,
               "WindowRuntime::subscriptions_for_view(") ||
      contains(runtime_diagnostic_snapshot_source,
               "WindowRuntime::subscription_connected(") ||
      contains(runtime_diagnostic_snapshot_source,
               "WindowRuntime::remove_subscription(") ||
      contains(runtime_diagnostic_snapshot_source,
               "WindowRuntime::install_native_menu(") ||
      contains(runtime_diagnostic_snapshot_source,
               "WindowRuntime::set_app_theme(")) {
    return 61;
  }

  const std::string runtime_subscriptions_source =
      read_source("src/ui/runtime_subscriptions.cpp");
  if (line_count(runtime_subscriptions_source) > 110 ||
      !contains(runtime_subscriptions_source,
                "std::span<const EntitySubscription> "
                "WindowRuntime::subscriptions_for_view(") ||
      !contains(runtime_subscriptions_source,
                "bool WindowRuntime::subscription_connected(") ||
      !contains(runtime_subscriptions_source,
                "bool WindowRuntime::remove_subscription(") ||
      contains(runtime_subscriptions_source,
               "RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot(") ||
      contains(runtime_subscriptions_source,
               "WindowRuntime::task_diagnostics(")) {
    return 123;
  }

  const std::string runtime_platform_services_source =
      read_source("src/ui/runtime_platform_services.cpp");
  if (line_count(runtime_platform_services_source) > 160 ||
      !contains(runtime_platform_services_source,
                "NativeMenuInstallation WindowRuntime::install_native_menu(") ||
      !contains(runtime_platform_services_source,
                "NativeFileDialogResult WindowRuntime::show_native_file_dialog(") ||
      !contains(runtime_platform_services_source,
                "void WindowRuntime::apply_cursor_shape(") ||
      !contains(runtime_platform_services_source,
                "void WindowRuntime::apply_focused_text_ime_placement(") ||
      !contains(runtime_platform_services_source,
                "void WindowRuntime::record_platform_diagnostic(") ||
      contains(runtime_platform_services_source,
               "RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot(")) {
    return 62;
  }

  const std::string runtime_theme_source =
      read_source("src/ui/runtime_theme.cpp");
  if (line_count(runtime_theme_source) > 120 ||
      !contains(runtime_theme_source, "void WindowRuntime::set_app_theme(") ||
      !contains(runtime_theme_source, "const Theme& WindowRuntime::app_theme(") ||
      !contains(runtime_theme_source,
                "void WindowRuntime::set_window_theme(") ||
      !contains(runtime_theme_source,
                "std::optional<Color> WindowRuntime::theme_color(") ||
      contains(runtime_theme_source, "install_native_menu(")) {
    return 63;
  }

  const std::string runtime_accessibility_source =
      read_source("src/ui/runtime_accessibility.cpp");
  if (line_count(runtime_accessibility_source) > 100 ||
      !contains(runtime_accessibility_source,
                "void WindowRuntime::update_platform_accessibility_tree(") ||
      !contains(runtime_accessibility_source,
                "WindowRuntime::build_platform_accessibility_update(") ||
      !contains(runtime_accessibility_source,
                "AccessibilityTreeSnapshot WindowRuntime::accessibility_snapshot(") ||
      contains(runtime_accessibility_source, "install_native_menu(") ||
      contains(runtime_accessibility_source,
               "RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot(")) {
    return 64;
  }

  const std::string app_context_source =
      read_source("src/ui/app_context.cpp");
  const std::string app_context_window_options_source =
      read_source("src/ui/app_context_window_options.cpp");
  const std::string app_context_services_source =
      read_source("src/ui/app_context_services.cpp");
  const std::string app_context_commands_source =
      read_source("src/ui/app_context_commands.cpp");
  const std::string app_context_theme_source =
      read_source("src/ui/app_context_theme.cpp");
  const std::string app_runner_source =
      read_source("src/ui/app_runner.cpp");
  if (app_context_source.empty() ||
      app_context_window_options_source.empty() ||
      app_context_services_source.empty() ||
      app_context_commands_source.empty() ||
      app_context_theme_source.empty() ||
      app_runner_source.empty()) {
    return 92;
  }
  if (line_count(app_context_source) > 20 ||
      contains(app_context_source, "WindowOptions::title(") ||
      contains(app_context_source, "AppContext::open_window(") ||
      contains(app_context_source,
               "AppContext::register_command_palette_entry(") ||
      contains(app_context_source, "AppContext::set_app_theme(") ||
      contains(app_context_source, "int run_app(")) {
    return 93;
  }
  if (line_count(app_context_window_options_source) > 80 ||
      !contains(app_context_window_options_source, "WindowOptions::title(") ||
      !contains(app_context_window_options_source, "WindowOptions::size(") ||
      !contains(app_context_window_options_source,
                "WindowOptions::transparent(") ||
      !contains(app_context_window_options_source,
                "WindowOptions::to_descriptor()") ||
      contains(app_context_window_options_source, "AppContext::open_window(") ||
      contains(app_context_window_options_source, "int run_app(")) {
    return 94;
  }
  if (line_count(app_context_services_source) > 70 ||
      !contains(app_context_services_source, "AppContext::open_window(") ||
      !contains(app_context_services_source,
                "AppContext::install_native_menu(") ||
      !contains(app_context_services_source,
                "AppContext::show_native_file_dialog(") ||
      contains(app_context_services_source,
               "register_command_palette_entry(") ||
      contains(app_context_services_source, "set_app_theme(")) {
    return 95;
  }
  if (line_count(app_context_commands_source) > 80 ||
      !contains(app_context_commands_source,
                "AppContext::register_command_palette_entry(") ||
      !contains(app_context_commands_source,
                "AppContext::command_palette_entries()") ||
      !contains(app_context_commands_source,
                "AppContext::dispatch_command_palette_action(") ||
      contains(app_context_commands_source, "AppContext::open_window(") ||
      contains(app_context_commands_source, "set_app_theme(")) {
    return 96;
  }
  if (line_count(app_context_theme_source) > 70 ||
      !contains(app_context_theme_source, "AppContext::set_app_theme(") ||
      !contains(app_context_theme_source, "AppContext::set_window_theme(") ||
      !contains(app_context_theme_source, "AppContext::theme_color(") ||
      !contains(app_context_theme_source, "AppContext::theme_spacing(") ||
      contains(app_context_theme_source, "AppContext::open_window(") ||
      contains(app_context_theme_source,
               "register_command_palette_entry(")) {
    return 97;
  }
  if (line_count(app_runner_source) > 80 ||
      !contains(app_runner_source, "int run_app(") ||
      !contains(app_runner_source, "WindowRuntime runtime(") ||
      !contains(app_runner_source, "AppContext context{.runtime = runtime}") ||
      contains(app_runner_source, "WindowOptions::title(") ||
      contains(app_runner_source, "AppContext::open_window(")) {
    return 98;
  }

  return 0;
}
