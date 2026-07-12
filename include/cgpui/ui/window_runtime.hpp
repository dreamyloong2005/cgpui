#pragma once

#include "cgpui/ui/action.hpp"
#include "cgpui/ui/async_io_hook.hpp"
#include "cgpui/ui/runtime_types.hpp"
#include "cgpui/ui/task_priority.hpp"
#include "cgpui/ui/task_group.hpp"

#include <any>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cgpui { namespace detail { class CrossThreadEntityQueueState; }

struct AppRunnerOptions {
  WindowDescriptor window;
  WindowRuntimeOptions runtime;
  AppSetupCallback setup;
  AppContextSetupCallback setup_context;
};
class WindowRuntime {
 public:
  WindowRuntime(
      PlatformApplication& application,
      View& view,
      RendererFactory renderer_factory);
  ~WindowRuntime();
  [[nodiscard]] int run(
      const WindowDescriptor& descriptor,
      WindowRuntimeOptions options = {});
  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options);
  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options, std::unique_ptr<View> root_view);
  [[nodiscard]] Result<AppOpenedWindow> try_open_window(WindowOptions options);
  [[nodiscard]] Result<AppOpenedWindow> try_open_window(WindowOptions options, std::unique_ptr<View> root_view);
  [[nodiscard]] Result<Renderer*> try_create_renderer(const RenderSurfaceDescriptor& descriptor);
  [[nodiscard]] std::span<const AppOpenedWindow> app_opened_windows() const;
  [[nodiscard]] WindowRuntimeId root_window_runtime_id() const;
  [[nodiscard]] std::span<const WindowRuntimeRecord> window_runtime_records() const;
  [[nodiscard]] const WindowRuntimeRecord* window_runtime_record(WindowRuntimeId runtime_id) const;
  [[nodiscard]] const View* app_opened_window_root_view(ViewId root_view_id) const;
  [[nodiscard]] View* root_view();
  [[nodiscard]] const View* root_view() const;
  [[nodiscard]] ViewId register_view(View& view);
  [[nodiscard]] ViewId register_view(std::unique_ptr<View> view);
  [[nodiscard]] View* find_view(ViewId view_id);
  [[nodiscard]] const View* find_view(ViewId view_id) const;
  [[nodiscard]] bool remove_view(ViewId view_id);
  void set_after_frame_callback(WindowRuntimeFrameCallback callback);
  void set_after_render_callback(WindowRuntimeRenderCallback callback);
  void set_after_event_callback(WindowRuntimeEventCallback callback);
  void set_close_requested_callback(WindowRuntimeFrameCallback callback);
  void set_error_callback(WindowRuntimeErrorCallback callback);
  void set_element_root(const Element* element);
  void set_element_tree(std::unique_ptr<ElementTree> tree);
  [[nodiscard]] const ElementTree* element_tree() const;
  [[nodiscard]] const Element* element_root() const;
  [[nodiscard]] const StaticElementTreeView* static_element_tree() const;

  template <typename T>
  [[nodiscard]] T* element_state(ElementId element_id);
  template <typename T>
  [[nodiscard]] const T* element_state(ElementId element_id) const;
  template <typename T, typename... Args>
  [[nodiscard]] T* emplace_element_state(ElementId element_id, Args&&... args);
  template <typename T, typename... Args>
  [[nodiscard]] T* element_state_or_init(ElementId element_id, Args&&... args);

  void capture_pointer(PointerCaptureOwner owner);
  void release_pointer(PointerCaptureOwner owner);
  void request_keyboard_focus();
  void request_keyboard_focus(ViewId view_id);
  void request_keyboard_focus(ElementId element_id);
  void release_keyboard_focus();
  void release_keyboard_focus(ViewId view_id);
  void release_keyboard_focus(ElementId element_id);
  [[nodiscard]] FocusHandle focus_handle(ElementId element_id) const;
  [[nodiscard]] ViewInputState input_state() const;
  void register_action(std::string name, ActionHandler handler);
  void register_action(std::string name, ActionHandler handler, ActionRegistrationOptions options);
  template <Action T> void register_action(ActionHandler handler);
  template <Action T> void register_action(ActionHandler handler, ActionRegistrationOptions options);
  void register_app_action(std::string name, ActionHandler handler);
  void register_app_action(std::string name, ActionHandler handler, ActionRegistrationOptions options);
  template <Action T> void register_app_action(ActionHandler handler);
  template <Action T> void register_app_action(ActionHandler handler, ActionRegistrationOptions options);
  void register_window_action(std::string name, ActionHandler handler);
  void register_window_action(std::string name, ActionHandler handler, ActionRegistrationOptions options);
  template <Action T> void register_window_action(ActionHandler handler);
  template <Action T> void register_window_action(ActionHandler handler, ActionRegistrationOptions options);
  void register_view_action(ViewId view_id, std::string name, ActionHandler handler);
  void register_view_action(ViewId view_id, std::string name, ActionHandler handler, ActionRegistrationOptions options);
  template <Action T> void register_view_action(ViewId view_id, ActionHandler handler);
  template <Action T> void register_view_action(ViewId view_id, ActionHandler handler, ActionRegistrationOptions options);
  void register_focused_element_action(ElementId element_id, std::string name, ActionHandler handler);
  void register_focused_element_action(ElementId element_id, std::string name, ActionHandler handler, ActionRegistrationOptions options);
  template <Action T> void register_focused_element_action(ElementId element_id, ActionHandler handler);
  template <Action T> void register_focused_element_action(ElementId element_id, ActionHandler handler, ActionRegistrationOptions options);
  [[nodiscard]] ActionDispatchResult dispatch_action(std::string name);
  template <Action T> [[nodiscard]] ActionDispatchResult dispatch_action();
  [[nodiscard]] std::optional<ActionDispatchResult> last_action_dispatch() const;
  [[nodiscard]] std::span<const ActionRegistration> action_registrations() const;
  [[nodiscard]] std::vector<ActionRegistration> action_registrations_for_scope(ActionRegistrationScope scope) const;
  [[nodiscard]] std::vector<ActionRegistration> action_registrations_for_enabled(bool enabled) const;
  void register_command_palette_entry(CommandPaletteEntry entry);
  template <Action T> void register_command_palette_entry(CommandPaletteEntry entry);
  [[nodiscard]] std::span<const CommandPaletteEntry> command_palette_entries() const;
  [[nodiscard]] std::vector<CommandPaletteEntry> command_palette_entries_for_group(std::string_view group) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_entry(
      const CommandPaletteEntry& entry);
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_action(
      std::string action_name);
  void bind_key(KeyBinding binding);
  void bind_text_edit_action(TextEditBinding binding);
  void bind_text_model(ElementId element_id, TextModel* model);
  [[nodiscard]] TextModel* focused_text_model();
  [[nodiscard]] const TextModel* focused_text_model() const;
  [[nodiscard]] std::optional<ImeCandidateRect> focused_text_ime_rect() const;
  [[nodiscard]] AccessibilityTreeSnapshot accessibility_snapshot() const;
  void set_clipboard(Clipboard* clipboard);
  [[nodiscard]] std::optional<std::string> read_clipboard_text();
  [[nodiscard]] bool write_clipboard_text(std::string_view text), paste_clipboard_text();
  [[nodiscard]] bool copy_selection_to_clipboard(), cut_selection_to_clipboard();
  void set_element_cursor(ElementId element_id, CursorShape cursor_shape);
  void request_render(), request_layout(), request_paint();
  [[nodiscard]] Result<void> try_draw_frame();
  void defer(DeferredCallback callback);
  [[nodiscard]] TimerId schedule_timer(
      std::uint64_t delay_ms,
      TimerCallback callback);
  [[nodiscard]] TimerId schedule_repeating_timer(
      std::uint64_t interval_ms,
      TimerCallback callback);
  [[nodiscard]] bool cancel_timer(TimerId id);
  void advance_time(std::uint64_t delta_ms);
  [[nodiscard]] AnimationHandle start_animation(
      AnimationOptions options,
      AnimationCallback callback);
  [[nodiscard]] std::optional<AnimationSnapshot> animation_snapshot(
      AnimationId id) const;
  [[nodiscard]] bool cancel_animation(AnimationId id);
  [[nodiscard]] TaskGroup create_task_group();
  [[nodiscard]] AsyncIoHook create_async_io_hook(AsyncIoCompletionCallback callback);
  [[nodiscard]] AsyncIoHook create_async_io_hook(TaskPriority priority, AsyncIoCompletionCallback callback);
  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback);
  [[nodiscard]] TaskHandle spawn_task(TaskPriority priority, TaskCompletionCallback callback);
  [[nodiscard]] Result<TaskHandle> try_spawn_task(TaskCompletionCallback callback);
  [[nodiscard]] Result<TaskHandle> try_spawn_task(TaskPriority priority, TaskCompletionCallback callback);
  [[nodiscard]] TaskHandle spawn_background_task(BackgroundTaskCallback work, TaskCompletionCallback completion);
  [[nodiscard]] TaskHandle spawn_background_task(TaskPriority priority, BackgroundTaskCallback work, TaskCompletionCallback completion);
  [[nodiscard]] Result<TaskHandle> try_spawn_background_task(BackgroundTaskCallback work, TaskCompletionCallback completion);
  [[nodiscard]] Result<TaskHandle> try_spawn_background_task(TaskPriority priority, BackgroundTaskCallback work, TaskCompletionCallback completion);
  [[nodiscard]] bool complete_task(TaskId id);
  void drain_task_completions();
  void drain_async_io_completions();
  void batch_updates(UpdateBatchCallback callback);
  void clear_invalidation();
  [[nodiscard]] InvalidationState invalidation_state() const;
  [[nodiscard]] NativeMenuInstallation install_native_menu(
      NativeMenuModel menu);
  [[nodiscard]] Result<NativeMenuInstallation> try_install_native_menu(NativeMenuModel menu);
  [[nodiscard]] const NativeMenuInstallation& native_menu_installation() const;
  [[nodiscard]] NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options);
  [[nodiscard]] Result<NativeFileDialogResult> try_show_native_file_dialog(NativeFileDialogOptions options);
  [[nodiscard]] const NativeFileDialogResult& native_file_dialog_result() const;
  [[nodiscard]] Result<NativeMessageDialogResult>
  try_show_native_message_dialog(NativeMessageDialogOptions options);
  [[nodiscard]] const NativeMessageDialogResult&
  native_message_dialog_result() const;
  [[nodiscard]] Result<PlatformOpenUrlResult> try_open_url(std::string url);
  [[nodiscard]] const PlatformOpenUrlResult& open_url_result() const;
  [[nodiscard]] Result<PlatformReopenResult> try_request_reopen();
  [[nodiscard]] const PlatformReopenResult& reopen_result() const;
  void set_app_theme(Theme theme);
  [[nodiscard]] const Theme& app_theme() const;
  void set_window_theme(WindowRuntimeId runtime_id, Theme theme);
  [[nodiscard]] bool clear_window_theme(WindowRuntimeId runtime_id);
  [[nodiscard]] const Theme* window_theme(WindowRuntimeId runtime_id) const;
  [[nodiscard]] std::optional<Color> theme_color(
      WindowRuntimeId runtime_id,
      const ThemeTokenId& id) const;
  [[nodiscard]] std::optional<float> theme_spacing(
      WindowRuntimeId runtime_id,
      const ThemeTokenId& id) const;
  [[nodiscard]] std::optional<RenderRecord> last_render_record() const;
  [[nodiscard]] RuntimeDiagnosticsSnapshot diagnostics_snapshot() const;
  [[nodiscard]] std::span<const PlatformDiagnosticEvent> platform_diagnostics() const;
  [[nodiscard]] std::span<const EntitySubscription> subscriptions_for_view(
      ViewId view_id) const;
  [[nodiscard]] bool subscription_connected(SubscriptionId id) const;
  [[nodiscard]] bool remove_subscription(SubscriptionId id);
  [[nodiscard]] ViewId allocate_view_id();
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> upgrade_view(WeakView view) const;
  [[nodiscard]] Result<void> try_resize_surface(Size size, DpiScale scale);
  Result<void> resize_surface(Size size, DpiScale scale);

  template <typename Observer>
  bool observe_window(Observer&& observer);
  template <typename Observer>
  [[nodiscard]] Subscription observe_window_subscription(Observer&& observer);
  template <typename Observer>
  bool observe_view(ViewId view_id, Observer&& observer);
  template <typename Observer>
  [[nodiscard]] Subscription observe_view_subscription(
      ViewId view_id,
      Observer&& observer);

  template <typename T>
  void set_global(T global_value);
  template <typename T>
  [[nodiscard]] const T* global() const;
  template <typename T>
  [[nodiscard]] T* global();
  template <typename T, typename Update>
  bool update_global(Update&& update);
  template <typename T>
  EntityId<T> insert_entity(T entity);
  template <typename T, typename... Args>
  EntityId<T> emplace_entity(Args&&... args);
  template <typename T>
  [[nodiscard]] const T* read_entity(EntityId<T> id) const;
  template <typename T>
  [[nodiscard]] std::optional<EntityId<T>> upgrade_entity(
      WeakEntity<T> entity) const;
  template <typename T>
  [[nodiscard]] T* mutate_entity(EntityId<T> id);
  template <typename T, typename Observer>
  bool observe_model(EntityId<T> entity_id, Observer&& observer);
  template <typename T, typename Observer>
  [[nodiscard]] Subscription observe_model_subscription(
      EntityId<T> entity_id,
      Observer&& observer);
  template <typename T>
  bool remove_entity(EntityId<T> id);
  template <typename T>
  void subscribe_view_to_entity(ViewId view_id, EntityId<T> entity_id);
  template <typename T>
  bool invalidate_entity(EntityId<T> entity_id);
  template <typename T>
  bool notify_entity_changed(EntityId<T> entity_id);
 private:
#include "../../../src/ui/window_runtime_internal.hpp"
};

} // namespace cgpui
