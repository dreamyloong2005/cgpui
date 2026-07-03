#include "ui_internal.hpp"

namespace cgpui {

ViewId WindowRuntimeContext::allocate_view_id() const {
  return runtime.allocate_view_id();
}

std::optional<EventRoute> WindowRuntimeContext::current_event_route() const {
  return event_route;
}

ViewInputState WindowRuntimeContext::input_state() const {
  return input;
}

bool WindowRuntimeContext::is_view_id_allocated(ViewId view_id) const {
  return runtime.is_view_id_allocated(view_id);
}

std::optional<ViewId> WindowRuntimeContext::upgrade_view(WeakView view) const {
  return runtime.upgrade_view(view);
}

void WindowRuntimeContext::capture_pointer(PointerCaptureOwner owner) const {
  runtime.capture_pointer(owner);
}

void WindowRuntimeContext::capture_pointer(ElementId element_id) const {
  runtime.capture_pointer(PointerCaptureOwner::element(element_id));
}

void WindowRuntimeContext::release_pointer(PointerCaptureOwner owner) const {
  runtime.release_pointer(owner);
}

void WindowRuntimeContext::release_pointer(ElementId element_id) const {
  runtime.release_pointer(PointerCaptureOwner::element(element_id));
}

void WindowRuntimeContext::request_keyboard_focus() const {
  runtime.request_keyboard_focus();
}

void WindowRuntimeContext::request_keyboard_focus(ViewId view_id) const {
  runtime.request_keyboard_focus(view_id);
}

void WindowRuntimeContext::request_keyboard_focus(ElementId element_id) const {
  runtime.request_keyboard_focus(element_id);
}

void WindowRuntimeContext::focus(ElementId element_id) const {
  runtime.request_keyboard_focus(element_id);
}

void WindowRuntimeContext::release_keyboard_focus() const {
  runtime.release_keyboard_focus();
}

void WindowRuntimeContext::release_keyboard_focus(ViewId view_id) const {
  runtime.release_keyboard_focus(view_id);
}

void WindowRuntimeContext::release_keyboard_focus(ElementId element_id) const {
  runtime.release_keyboard_focus(element_id);
}

void WindowRuntimeContext::blur(ElementId element_id) const {
  runtime.release_keyboard_focus(element_id);
}

FocusHandle WindowRuntimeContext::focus_handle(ElementId element_id) const {
  return runtime.focus_handle(element_id);
}

void WindowRuntimeContext::set_element_tree(
    std::unique_ptr<ElementTree> tree) const {
  runtime.set_element_tree(std::move(tree));
}

void WindowRuntimeContext::register_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_action(std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_app_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_app_action(std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_window_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_window_action(std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_view_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_view_action(view_id, std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_view_action(
    ViewId target_view_id,
    std::string name,
    ActionHandler handler) const {
  runtime.register_view_action(
      target_view_id,
      std::move(name),
      std::move(handler));
}

void WindowRuntimeContext::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler) const {
  runtime.register_focused_element_action(
      element_id,
      std::move(name),
      std::move(handler));
}

ActionDispatchResult WindowRuntimeContext::dispatch_action(
    std::string name) const {
  return runtime.dispatch_action(std::move(name));
}

std::optional<ActionDispatchResult> WindowRuntimeContext::last_action_dispatch()
    const {
  return runtime.last_action_dispatch();
}

void WindowRuntimeContext::register_command_palette_entry(
    CommandPaletteEntry entry) const {
  runtime.register_command_palette_entry(std::move(entry));
}

std::span<const CommandPaletteEntry>
WindowRuntimeContext::command_palette_entries() const {
  return runtime.command_palette_entries();
}

std::vector<CommandPaletteEntry>
WindowRuntimeContext::command_palette_entries_for_group(
    std::string_view group) const {
  return runtime.command_palette_entries_for_group(group);
}

ActionDispatchResult WindowRuntimeContext::dispatch_command_palette_entry(
    const CommandPaletteEntry& entry) const {
  return runtime.dispatch_command_palette_entry(entry);
}

ActionDispatchResult WindowRuntimeContext::dispatch_command_palette_action(
    std::string action_name) const {
  return runtime.dispatch_command_palette_action(std::move(action_name));
}

void WindowRuntimeContext::bind_key(KeyBinding binding) const {
  runtime.bind_key(std::move(binding));
}

void WindowRuntimeContext::bind_text_edit_action(
    TextEditBinding binding) const {
  runtime.bind_text_edit_action(std::move(binding));
}

void WindowRuntimeContext::bind_text_model(
    ElementId element_id,
    TextModel* model) const {
  runtime.bind_text_model(element_id, model);
}

bool WindowRuntimeContext::mutate_focused_text_model(
    FocusedTextModelMutation mutation) const {
  if (!mutation) {
    return false;
  }

  TextModel* model = runtime.focused_text_model();
  if (model == nullptr) {
    return false;
  }

  mutation(*model);
  return true;
}

std::optional<ImeCandidateRect> WindowRuntimeContext::focused_text_ime_rect()
    const {
  return runtime.focused_text_ime_rect();
}

AccessibilityTreeSnapshot WindowRuntimeContext::accessibility_snapshot()
    const {
  return runtime.accessibility_snapshot();
}

void WindowRuntimeContext::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) const {
  runtime.set_element_cursor(element_id, cursor_shape);
}

bool WindowRuntimeContext::paste_clipboard_text() const {
  return runtime.paste_clipboard_text();
}

bool WindowRuntimeContext::copy_selection_to_clipboard() const {
  return runtime.copy_selection_to_clipboard();
}

bool WindowRuntimeContext::cut_selection_to_clipboard() const {
  return runtime.cut_selection_to_clipboard();
}

void WindowRuntimeContext::request_render() const {
  runtime.request_render();
}

void WindowRuntimeContext::request_layout() const {
  runtime.request_layout();
}

void WindowRuntimeContext::request_paint() const {
  runtime.request_paint();
}

void WindowRuntimeContext::defer(DeferredCallback callback) const {
  runtime.defer(std::move(callback));
}

TimerId WindowRuntimeContext::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) const {
  return runtime.schedule_timer(delay_ms, std::move(callback));
}

TimerId WindowRuntimeContext::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) const {
  return runtime.schedule_repeating_timer(interval_ms, std::move(callback));
}

AnimationHandle WindowRuntimeContext::start_animation(
    AnimationOptions options,
    AnimationCallback callback) const {
  return runtime.start_animation(options, std::move(callback));
}

std::optional<AnimationSnapshot> WindowRuntimeContext::animation_snapshot(
    AnimationId id) const {
  return runtime.animation_snapshot(id);
}

bool WindowRuntimeContext::cancel_animation(AnimationId id) const {
  return runtime.cancel_animation(id);
}

TaskHandle WindowRuntimeContext::spawn_task(
    TaskCompletionCallback callback) const {
  return runtime.spawn_task(std::move(callback));
}

TaskHandle WindowRuntimeContext::spawn_background_task(
    BackgroundTaskCallback work,
    TaskCompletionCallback completion) const {
  return runtime.spawn_background_task(
      std::move(work),
      std::move(completion));
}

void WindowRuntimeContext::batch_updates(
    UpdateBatchCallback callback) const {
  runtime.batch_updates(std::move(callback));
}

void WindowRuntimeContext::clear_invalidation() const {
  runtime.clear_invalidation();
}

InvalidationState WindowRuntimeContext::invalidation_state() const {
  return runtime.invalidation_state();
}

RuntimeDiagnosticsSnapshot WindowRuntimeContext::diagnostics_snapshot() const {
  return runtime.diagnostics_snapshot();
}

std::span<const PlatformDiagnosticEvent>
WindowRuntimeContext::platform_diagnostics() const {
  return runtime.platform_diagnostics();
}

NativeMenuInstallation WindowRuntimeContext::install_native_menu(
    NativeMenuModel menu) const {
  return runtime.install_native_menu(std::move(menu));
}

const NativeMenuInstallation& WindowRuntimeContext::native_menu_installation()
    const {
  return runtime.native_menu_installation();
}

NativeFileDialogResult WindowRuntimeContext::show_native_file_dialog(
    NativeFileDialogOptions options) const {
  return runtime.show_native_file_dialog(std::move(options));
}

const NativeFileDialogResult& WindowRuntimeContext::native_file_dialog_result()
    const {
  return runtime.native_file_dialog_result();
}

void WindowRuntimeContext::set_window_theme(Theme theme) const {
  runtime.set_window_theme(window_runtime_id, std::move(theme));
}

bool WindowRuntimeContext::clear_window_theme() const {
  return runtime.clear_window_theme(window_runtime_id);
}

const Theme& WindowRuntimeContext::app_theme() const {
  return runtime.app_theme();
}

const Theme* WindowRuntimeContext::window_theme() const {
  return runtime.window_theme(window_runtime_id);
}

std::optional<Color> WindowRuntimeContext::theme_color(
    const ThemeTokenId& id) const {
  return runtime.theme_color(window_runtime_id, id);
}

std::optional<float> WindowRuntimeContext::theme_spacing(
    const ThemeTokenId& id) const {
  return runtime.theme_spacing(window_runtime_id, id);
}


} // namespace cgpui
