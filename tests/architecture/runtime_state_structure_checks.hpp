#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace runtime_state_structure {

inline void require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

inline bool has(const std::string& source, std::string_view token) {
  return source.find(token) != std::string::npos;
}

inline std::string read(const std::filesystem::path& root, const std::string& path) {
  std::ifstream input(root / path);
  require(static_cast<bool>(input), "missing private runtime module: " + path);
  return {std::istreambuf_iterator<char>(input), {}};
}

inline void limit(const std::string& source, std::size_t maximum,
                  const std::string& path) {
  const auto count = static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) +
      (!source.empty() && source.back() != '\n' ? 1U : 0U);
  require(count <= maximum, path + " exceeds " + std::to_string(maximum) + " lines");
}

inline void tokens(const std::string& source, std::initializer_list<const char*> required,
                   const std::string& owner) {
  for (const char* token : required) {
    require(has(source, token), owner + " must own " + token);
  }
}

inline std::string group_path(const char* name) {
  return std::string("src/ui/runtime_state_") + name + "_internal.hpp";
}

inline void check(const std::filesystem::path& root) {
  const auto aggregate = read(root, "src/ui/runtime_state_internal.hpp");
  limit(aggregate, 45, "src/ui/runtime_state_internal.hpp");
  tokens(aggregate, {"struct WindowRuntime::RuntimeState {", "~RuntimeState();"},
         "runtime state aggregate");
  require(has(read(root, "src/ui/ui_internal.hpp"), "#include \"runtime_state_internal.hpp\""),
          "runtime implementation must include the actual private state aggregate");

  struct Group {
    const char* name;
    const char* type;
    std::size_t maximum;
    std::initializer_list<const char*> fields;
  };
  const Group groups[]{
      {"core", "RuntimeCoreState", 30,
       {"PlatformApplication& application_", "View& view_", "RendererFactory renderer_factory_",
        "after_frame_callback_", "error_callback_", "should_quit_", "failed_"}},
      {"windows", "RuntimeWindowState", 30,
       {"PlatformWindow* window_", "Renderer* renderer_", "framebuffer_size_", "viewport_size_",
        "app_opened_windows_", "root_window_runtime_id_", "window_runtime_records_"}},
      {"views", "RuntimeViewState", 25,
       {"root_view_id_", "next_view_id_", "view_registry_", "removed_view_ids_"}},
      {"elements", "RuntimeElementState", 30,
       {"owned_element_tree_", "view_rendered_element_tree_", "static_element_tree_", "element_cursors_"}},
      {"input", "RuntimeInputState", 30,
       {"ViewInputState input_", "pointer_capture_owner_", "keyboard_focus_owner_",
        "text_pointer_selection_drag_", "current_event_route_", "dispatching_view_event_"}},
      {"actions", "RuntimeActionState", 35,
       {"action_handlers_", "window_action_handlers_", "view_action_handlers_",
        "focused_element_action_handlers_", "action_registrations_", "key_bindings_"}},
      {"text", "RuntimeTextState", 20,
       {"text_edit_bindings_", "text_models_", "Clipboard* clipboard_", "applied_ime_text_input_placement_"}},
      {"entities", "RuntimeEntityState", 30,
       {"globals_", "entity_stores_", "entity_count_", "entity_subscriptions_",
        "entity_observers_", "window_observers_", "view_observers_", "next_subscription_id_"}},
      {"scheduling", "RuntimeScheduleState", 30,
       {"deferred_callbacks_", "timers_", "next_timer_id_", "current_time_ms_",
        "invalidation_state_", "redraw_scheduled_", "deferred_redraw_request_"}},
      {"animations", "RuntimeAnimationState", 20, {}},
      {"tasks", "RuntimeTaskState", 35,
       {"tasks_mutex_", "cross_thread_entity_queue_", "async_io_registry_", "task_pool_",
        "task_group_store_", "tasks_", "task_completion_queue_", "next_task_id_"}},
      {"platform", "RuntimePlatformState", 25,
       {"last_platform_accessibility_update_", "Theme app_theme_", "window_themes_"}},
      {"diagnostics", "RuntimeDiagnosticState", 25,
       {"last_render_record_", "last_event_dispatch_", "last_action_dispatch_",
        "frame_index_", "platform_diagnostics_", "platform_diagnostic_sequence_"}},
  };
  const auto task_position = aggregate.find("RuntimeTaskState tasks;");
  require(task_position != std::string::npos, "runtime task state must have an owning member");
  for (const auto& group : groups) {
    const auto path = group_path(group.name);
    const auto source = read(root, path);
    limit(source, group.maximum, path);
    require(has(source, std::string("struct WindowRuntime::") + group.type + " {"),
            path + " must define its own focused state group");
    require(has(aggregate, std::string("#include \"runtime_state_") + group.name + "_internal.hpp\""),
            "runtime aggregate must include " + path);
    const auto member = std::string(group.type) + " " + group.name + ";";
    const auto position = aggregate.find(member);
    require(position != std::string::npos, "runtime aggregate must own " + member);
    if (std::string_view(group.name) != "tasks") {
      require(task_position < position, "task metadata must outlive the " + std::string(group.name) + " group");
    }
    tokens(source, group.fields, path);
  }

  struct Types {
    const char* group;
    std::size_t maximum;
    std::initializer_list<const char*> definitions;
  };
  const Types types[]{
      {"core", 12, {"struct WindowRuntime::OwnedRendererFactoryTag {}"}},
      {"views", 15, {"struct WindowRuntime::RegisteredView {"}},
      {"elements", 25, {"struct WindowRuntime::StaticRenderInstallResult {", "struct WindowRuntime::WindowElementHost {"}},
      {"input", 15, {"struct WindowRuntime::TextPointerSelectionDrag {"}},
      {"actions", 35, {"struct WindowRuntime::ActionDispatchTarget {", "class WindowRuntime::ActionWindowScope {"}},
      {"scheduling", 20, {"struct WindowRuntime::RuntimeTimer {"}},
      {"animations", 25, {"struct WindowRuntime::RuntimeAnimation {"}},
      {"tasks", 35, {"struct WindowRuntime::RuntimeTask {", "struct WindowRuntime::RuntimeTaskCompletion {",
                     "struct WindowRuntime::RuntimeTaskDiagnostics {"}},
  };
  for (const auto& type : types) {
    const auto file = std::string("runtime_state_") + type.group + "_types_internal.hpp";
    const auto path = "src/ui/" + file;
    const auto source = read(root, path);
    limit(source, type.maximum, path);
    tokens(source, type.definitions, path);
    require(has(read(root, group_path(type.group)), "#include \"" + file + "\""),
            "state group must include its actual complete types: " + file);
  }

  struct Fragment {
    const char* group;
    const char* file;
    std::size_t maximum;
    std::initializer_list<const char*> fields;
  };
  const Fragment fragments[]{
      {"windows", "runtime_window_ownership_state_internal.hpp", 5,
       {"native_additional_windows_", "retired_native_windows_", "window_lifetime_"}},
      {"elements", "runtime_window_element_host_internal.hpp", 3, {"window_element_hosts_"}},
      {"actions", "runtime_window_action_dispatch_internal.hpp", 3, {"action_window_runtime_id_"}},
      {"animations", "runtime_animation_frame_pacing_internal.hpp", 8,
       {"animation_frame_timer_id_", "animation_frame_timer_deadline_ms_", "element_animation_frame_deadline_ms_",
        "element_animation_frame_pending_", "delivering_animation_frame_", "animation_frame_pacing_diagnostics_"}},
      {"animations", "runtime_animation_state_internal.hpp", 6,
       {"animations_", "next_animation_id_", "last_animation_cancellation_", "element_animation_state_store_"}},
      {"diagnostics", "runtime_renderer_diagnostic_state_internal.hpp", 4,
       {"last_frame_statistics_", "last_renderer_frame_diagnostics_"}},
      {"platform", "runtime_platform_service_state_internal.hpp", 10,
       {"native_menu_installation_", "native_file_dialog_result_", "native_message_dialog_result_",
        "open_url_result_", "reopen_result_"}},
      {"platform", "runtime_system_appearance_internal.hpp", 5,
       {"system_appearance_callback_", "system_appearance_bridge_installed_"}},
      {"scheduling", "runtime_frame_scheduling_internal.hpp", 5,
       {"rendering_frame_", "next_frame_redraw_requested_"}},
  };
  for (const auto& fragment : fragments) {
    const auto path = std::string("src/ui/") + fragment.file;
    const auto source = read(root, path);
    limit(source, fragment.maximum, path);
    tokens(source, fragment.fields, path);
    require(!has(source, "#include") && !has(source, "(") && !has(source, "class ") &&
                !has(source, "struct "), path + " must remain a fields-only fragment");
    const auto owner = read(root, group_path(fragment.group));
    const auto begin = owner.find("struct WindowRuntime::");
    const auto end = owner.find("\n};", begin);
    const auto include = owner.find(std::string("#include \"") + fragment.file + "\"");
    require(begin != std::string::npos && end != std::string::npos &&
                include > begin && include < end,
            path + " must be included inside its real private state group");
  }

  const auto state = read(root, "src/ui/runtime_state.cpp");
  const auto core = read(root, "src/ui/runtime_state_core.cpp");
  const auto windows = read(root, "src/ui/runtime_state_windows.cpp");
  const auto tasks = read(root, "src/ui/runtime_state_tasks.cpp");
  limit(state, 20, "src/ui/runtime_state.cpp");
  limit(core, 15, "src/ui/runtime_state_core.cpp");
  limit(windows, 20, "src/ui/runtime_state_windows.cpp");
  limit(tasks, 25, "src/ui/runtime_state_tasks.cpp");
  tokens(state, {"WindowRuntime::RuntimeState::RuntimeState(", ": tasks(owner), core(",
                 "WindowRuntime::RuntimeState::~RuntimeState() = default"}, "state construction");
  tokens(core, {"WindowRuntime::RuntimeCoreState::RuntimeCoreState(", "application_(application)",
                "view_(view)", "renderer_factory_(std::move(factory))"}, "core construction");
  tokens(windows, {"#include \"runtime_window_store_internal.hpp\"",
                   "#include \"runtime_window_lifetime_internal.hpp\"",
                   "std::make_unique<RuntimeWindowStore>()", "std::make_unique<RuntimeWindowLifetime>()",
                   "WindowRuntime::RuntimeWindowState::~RuntimeWindowState() = default"}, "window ownership");
  tokens(tasks, {"#include \"runtime_async_io_internal.hpp\"", "#include \"runtime_cross_thread_entity_internal.hpp\"",
                 "#include \"runtime_task_pool_internal.hpp\"", "#include \"runtime_task_group_internal.hpp\"",
                 "std::make_shared<detail::CrossThreadEntityQueueState>(owner)",
                 "std::make_unique<RuntimeAsyncIoRegistry>()", "std::make_unique<RuntimeTaskPool>()",
                 "std::make_unique<RuntimeTaskGroupStore>()",
                 "WindowRuntime::RuntimeTaskState::~RuntimeTaskState() = default"}, "task ownership");
}

} // namespace runtime_state_structure
