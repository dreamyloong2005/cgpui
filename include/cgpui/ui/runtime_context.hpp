#pragma once

#include "cgpui/ui/runtime_app_context.hpp"
#include "cgpui/ui/runtime_handles.hpp"
#include "cgpui/ui/runtime_input_state.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct WindowRuntimeContext {
  WindowRuntime& runtime;
  PlatformApplication& application;
  PlatformWindow& platform_window;
  Renderer& renderer;
  WindowRuntimeId window_runtime_id;
  ViewId view_id;
  Size viewport_size;
  DpiScale scale;
  ViewInputState input;
  std::optional<EventRoute> event_route;
  EventResult last_event_result;
  std::optional<EventDispatchRecord> last_event_dispatch;
  int frame_index = 0;

  [[nodiscard]] std::optional<EventRoute> current_event_route() const;
  [[nodiscard]] App app() const;
  [[nodiscard]] Window window() const;
  [[nodiscard]] Window current_window() const;
  [[nodiscard]] ViewInputState input_state() const;
  [[nodiscard]] ViewId allocate_view_id() const;
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> upgrade_view(WeakView view) const;
  template <typename T>
  [[nodiscard]] ViewHandle<T> view() const;
  template <typename T>
  [[nodiscard]] WeakViewHandle<T> weak_view() const;
  template <typename T>
  [[nodiscard]] std::optional<ViewHandle<T>> upgrade_view(
      WeakViewHandle<T> view) const;
  template <typename T>
  [[nodiscard]] const T* read_view(ViewHandle<T> view) const;
  void capture_pointer(PointerCaptureOwner owner) const;
  void capture_pointer(ElementId element_id) const;
  void release_pointer(PointerCaptureOwner owner) const;
  void release_pointer(ElementId element_id) const;
  void request_keyboard_focus() const;
  void request_keyboard_focus(ViewId view_id) const;
  void request_keyboard_focus(ElementId element_id) const;
  void focus(ElementId element_id) const;
  void release_keyboard_focus() const;
  void release_keyboard_focus(ViewId view_id) const;
  void release_keyboard_focus(ElementId element_id) const;
  void blur(ElementId element_id) const;
  [[nodiscard]] FocusHandle focus_handle(ElementId element_id) const;
  void set_element_tree(std::unique_ptr<ElementTree> tree) const;

  template <typename T>
  [[nodiscard]] T* element_state(ElementId element_id) const;
  template <typename T, typename... Args>
  [[nodiscard]] T* emplace_element_state(
      ElementId element_id,
      Args&&... args) const;
  template <typename T, typename... Args>
  [[nodiscard]] T* element_state_or_init(
      ElementId element_id,
      Args&&... args) const;

  void register_action(std::string name, ActionHandler handler) const;
  void register_app_action(std::string name, ActionHandler handler) const;
  void register_window_action(std::string name, ActionHandler handler) const;
  void register_view_action(std::string name, ActionHandler handler) const;
  void register_view_action(
      ViewId view_id,
      std::string name,
      ActionHandler handler) const;
  void register_focused_element_action(
      ElementId element_id,
      std::string name,
      ActionHandler handler) const;
  [[nodiscard]] ActionDispatchResult dispatch_action(std::string name) const;
  [[nodiscard]] std::optional<ActionDispatchResult> last_action_dispatch() const;
  void register_command_palette_entry(CommandPaletteEntry entry) const;
  [[nodiscard]] std::span<const CommandPaletteEntry> command_palette_entries()
      const;
  [[nodiscard]] std::vector<CommandPaletteEntry>
  command_palette_entries_for_group(std::string_view group) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_entry(
      const CommandPaletteEntry& entry) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_action(
      std::string action_name) const;
  void bind_key(KeyBinding binding) const;
  void bind_text_edit_action(TextEditBinding binding) const;
  void bind_text_model(ElementId element_id, TextModel* model) const;
  [[nodiscard]] bool mutate_focused_text_model(
      FocusedTextModelMutation mutation) const;
  [[nodiscard]] std::optional<ImeCandidateRect> focused_text_ime_rect() const;
  [[nodiscard]] AccessibilityTreeSnapshot accessibility_snapshot() const;
  void set_element_cursor(ElementId element_id, CursorShape cursor_shape) const;
  [[nodiscard]] bool paste_clipboard_text() const;
  [[nodiscard]] bool copy_selection_to_clipboard() const;
  [[nodiscard]] bool cut_selection_to_clipboard() const;
  void request_render() const;
  void request_layout() const;
  void request_paint() const;
  void defer(DeferredCallback callback) const;
  [[nodiscard]] TimerId schedule_timer(
      std::uint64_t delay_ms,
      TimerCallback callback) const;
  [[nodiscard]] TimerId schedule_repeating_timer(
      std::uint64_t interval_ms,
      TimerCallback callback) const;
  [[nodiscard]] AnimationHandle start_animation(
      AnimationOptions options,
      AnimationCallback callback) const;
  [[nodiscard]] std::optional<AnimationSnapshot> animation_snapshot(
      AnimationId id) const;
  [[nodiscard]] bool cancel_animation(AnimationId id) const;
  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback) const;
  [[nodiscard]] TaskHandle spawn_background_task(
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  void batch_updates(UpdateBatchCallback callback) const;
  void clear_invalidation() const;
  [[nodiscard]] InvalidationState invalidation_state() const;
  [[nodiscard]] RuntimeDiagnosticsSnapshot diagnostics_snapshot() const;
  [[nodiscard]] std::span<const PlatformDiagnosticEvent>
  platform_diagnostics() const;
  [[nodiscard]] NativeMenuInstallation install_native_menu(
      NativeMenuModel menu) const;
  [[nodiscard]] const NativeMenuInstallation& native_menu_installation() const;
  [[nodiscard]] NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) const;
  [[nodiscard]] const NativeFileDialogResult& native_file_dialog_result()
      const;
  void set_window_theme(Theme theme) const;
  [[nodiscard]] bool clear_window_theme() const;
  [[nodiscard]] const Theme& app_theme() const;
  [[nodiscard]] const Theme* window_theme() const;
  [[nodiscard]] std::optional<Color> theme_color(
      const ThemeTokenId& id) const;
  [[nodiscard]] std::optional<float> theme_spacing(
      const ThemeTokenId& id) const;

  template <typename T>
  void set_global(T global_value) const;
  template <typename T>
  [[nodiscard]] const T* global() const;
  template <typename T, typename Update>
  bool update_global(Update&& update) const;
  template <typename T, typename... Args>
  Model<T> new_model(Args&&... args) const;
  template <typename T>
  [[nodiscard]] const T* read_model(Model<T> model) const;
  template <typename T>
  [[nodiscard]] EntityHandle<T> entity(EntityId<T> id) const;
  template <typename T>
  [[nodiscard]] WeakEntity<T> weak_entity(EntityId<T> id) const;
  template <typename T>
  [[nodiscard]] std::optional<Model<T>> upgrade_entity(
      WeakEntity<T> entity) const;
  template <typename T, typename Update>
  bool update_model(Model<T> model, Update&& update) const;
  template <typename T, typename Observer>
  bool observe_model(Model<T> model, Observer&& observer) const;
  template <typename T, typename Observer>
  [[nodiscard]] Subscription observe_model_subscription(
      Model<T> model,
      Observer&& observer) const;
  template <typename T>
  bool remove_model(Model<T> model) const;
  template <typename T>
  EntityId<T> insert_entity(T entity) const;
  template <typename T, typename... Args>
  EntityId<T> emplace_entity(Args&&... args) const;
  template <typename T>
  [[nodiscard]] const T* read_entity(EntityId<T> id) const;
  template <typename T>
  [[nodiscard]] T* mutate_entity(EntityId<T> id) const;
  template <typename T>
  bool remove_entity(EntityId<T> id) const;
  template <typename T>
  void subscribe_view_to_entity(ViewId view_id, EntityId<T> entity_id) const;
};

} // namespace cgpui
