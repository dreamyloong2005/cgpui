#pragma once

#include "cgpui/core/entity.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/text.hpp"

#include <any>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace cgpui {

struct WindowRuntimeContext;
using ViewContext = WindowRuntimeContext;

template <typename T>
using Context = ViewContext;

using ActionHandler =
    std::function<EventResult(const WindowRuntimeContext&)>;
using FocusedTextModelMutation = std::function<void(TextModel&)>;
using DeferredCallback = std::function<void(const WindowRuntimeContext&)>;
using TimerCallback = std::function<void(const WindowRuntimeContext&)>;
using TaskCompletionCallback =
    std::function<void(const WindowRuntimeContext&)>;
using UpdateBatchCallback =
    std::function<void(const WindowRuntimeContext&)>;

template <typename T>
using ModelObserver =
    std::function<void(const WindowRuntimeContext&, Model<T>)>;

enum class PaintCommandKind {
  solid_rect,
  rounded_rect,
  text,
  text_selection,
  text_caret,
};

struct RoundedRect {
  Rect rect;
  Color color;
  BorderRadii radius;
};

struct TextPaint {
  Rect bounds;
  Color color;
  FontDescriptor font;
  std::string content;
  std::size_t byte_length = 0;
  float font_size = 16.0F;
  DpiScale scale;
  float device_font_size = 16.0F;
  std::vector<TextGlyphPaint> glyphs;
  std::vector<TextWrapLine> lines;
};

struct TextSelectionPaint {
  Rect rect;
  Color color;
  TextSelectionRange range;
  float font_size = 16.0F;
};

struct TextCaretPaint {
  Rect rect;
  Color color;
  std::size_t byte_offset = 0;
  float font_size = 16.0F;
};

struct ImeCandidateRect {
  ElementId element_id;
  Rect rect;
  std::size_t byte_offset = 0;
};

struct PaintCommand {
  PaintCommandKind kind = PaintCommandKind::solid_rect;
  SolidRect solid_rect;
  RoundedRect rounded_rect;
  TextPaint text;
  TextSelectionPaint text_selection;
  TextCaretPaint text_caret;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
  RendererCompositionStackRecord composition_stack;
  PaintMetadata metadata;
};

class PaintList {
 public:
  void clear();
  void set_scale(DpiScale scale);
  [[nodiscard]] DpiScale scale() const;
  void push_clip(Rect rect);
  void pop_clip();
  void push_metadata(PaintMetadata metadata);
  void pop_metadata();
  void fill_rect(Rect rect, Color color);
  void fill_rounded_rect(Rect rect, Color color, BorderRadii radius);
  void fill_text(
      Rect bounds,
      Color color,
      std::string_view text,
      FontDescriptor font = {},
      float font_size = 16.0F);
  void fill_text_selection(
      Rect rect,
      Color color,
      TextSelectionRange range,
      float font_size = 16.0F);
  void fill_text_caret(
      Rect rect,
      Color color,
      std::size_t byte_offset,
      float font_size = 16.0F);
  void set_text_measurement_cache(TextMeasurementCache* cache);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
  std::vector<Rect> clip_stack_;
  std::vector<PaintMetadata> metadata_stack_;
  DpiScale scale_;
  TextMeasurementCache* text_measurement_cache_ = nullptr;
};

class View {
 public:
  virtual ~View() = default;
  virtual AnyElement render(ViewContext& context);
  virtual void paint(PaintList& paint_list, Size viewport_size) = 0;
  virtual EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
};

struct WindowRuntimeOptions {
  bool request_initial_redraw = true;
};

class WindowRuntime;

struct SubscriptionId {
  std::uint64_t value = 0;

  friend bool operator==(
      const SubscriptionId&,
      const SubscriptionId&) = default;
};

struct TimerId {
  std::uint64_t value = 0;

  friend bool operator==(const TimerId&, const TimerId&) = default;
};

struct TaskId {
  std::uint64_t value = 0;

  friend bool operator==(const TaskId&, const TaskId&) = default;
};

class TaskHandle {
 public:
  TaskHandle() = default;

  [[nodiscard]] TaskId id() const {
    return id_;
  }

  [[nodiscard]] bool active() const;
  [[nodiscard]] bool complete() const;

 private:
  friend class WindowRuntime;

  TaskHandle(WindowRuntime& runtime, TaskId id) : runtime_(&runtime), id_(id) {}

  WindowRuntime* runtime_ = nullptr;
  TaskId id_{};
};

class Subscription {
 public:
  Subscription() = default;
  ~Subscription();

  Subscription(const Subscription&) = delete;
  Subscription& operator=(const Subscription&) = delete;

  Subscription(Subscription&& other) noexcept;
  Subscription& operator=(Subscription&& other) noexcept;

  [[nodiscard]] SubscriptionId id() const {
    return id_;
  }

  [[nodiscard]] bool connected() const;
  [[nodiscard]] bool release();

 private:
  friend class WindowRuntime;

  Subscription(WindowRuntime& runtime, SubscriptionId id)
      : runtime_(&runtime), id_(id) {}

  WindowRuntime* runtime_ = nullptr;
  SubscriptionId id_{};
};

struct WindowOptions {
  WindowDescriptor descriptor;

  WindowOptions& title(std::string title);
  WindowOptions& size(Size size);
  WindowOptions& size(float width, float height);
  WindowOptions& titlebar_visible(bool visible);
  WindowOptions& decorations(bool enabled);
  WindowOptions& resizable(bool enabled);
  WindowOptions& transparent(bool enabled);
  [[nodiscard]] WindowDescriptor to_descriptor() const;
};

struct WindowRuntimeId {
  std::uint64_t value = 0;

  friend bool operator==(
      const WindowRuntimeId&,
      const WindowRuntimeId&) = default;
};

struct AppOpenedWindow {
  WindowRuntimeId runtime_id;
  WindowDescriptor descriptor;
  ViewId root_view_id;
};

struct WindowRuntimeRecord {
  WindowRuntimeId runtime_id;
  WindowDescriptor descriptor;
  ViewId root_view_id;
  PlatformWindow* window = nullptr;
  Renderer* renderer = nullptr;
  bool owns_window = false;
  bool owns_renderer = false;
  bool owns_root_view = false;
  bool active = false;
  std::optional<Error> native_window_error;
};

enum class ActionScope {
  app,
  window,
  view,
  focused_element,
};

struct ActionDispatchResult {
  std::string name;
  bool handled = false;
  EventResult result;
  std::optional<ActionScope> scope;
  std::optional<ViewId> view_id;
  std::optional<ElementId> element_id;
};

struct CommandPaletteEntry {
  std::string action_name;
  std::string title;
  std::string group;
  ActionScope scope = ActionScope::app;
  bool enabled = true;
  std::optional<ViewId> view_id;
  std::optional<ElementId> element_id;
};

struct AppContext {
  WindowRuntime& runtime;

  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options) const;
  [[nodiscard]] AppOpenedWindow open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view) const;
  [[nodiscard]] NativeMenuInstallation install_native_menu(
      NativeMenuModel menu) const;
  [[nodiscard]] NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options) const;
  void register_command_palette_entry(CommandPaletteEntry entry) const;
  [[nodiscard]] std::span<const CommandPaletteEntry> command_palette_entries()
      const;
  [[nodiscard]] std::vector<CommandPaletteEntry>
  command_palette_entries_for_group(std::string_view group) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_entry(
      const CommandPaletteEntry& entry) const;
  [[nodiscard]] ActionDispatchResult dispatch_command_palette_action(
      std::string action_name) const;

  template <typename T>
  void set_global(T global_value) const;

  template <typename T>
  [[nodiscard]] const T* global() const;

  template <typename T, typename Update>
  bool update_global(Update&& update) const;
};

class WeakView {
 public:
  constexpr WeakView() = default;
  constexpr explicit WeakView(ViewId id) : id_(id) {}

  [[nodiscard]] constexpr ViewId id() const {
    return id_;
  }

  [[nodiscard]] constexpr bool empty() const {
    return id_.value == 0;
  }

  friend bool operator==(const WeakView&, const WeakView&) = default;

 private:
  ViewId id_;
};

class PointerCaptureOwner {
 public:
  [[nodiscard]] static constexpr PointerCaptureOwner view(ViewId view_id) {
    return PointerCaptureOwner(view_id);
  }

  [[nodiscard]] static constexpr PointerCaptureOwner element(
      ElementId element_id) {
    return PointerCaptureOwner(element_id);
  }

  [[nodiscard]] constexpr bool is_view() const {
    return std::holds_alternative<ViewId>(owner_);
  }

  [[nodiscard]] constexpr bool is_element() const {
    return std::holds_alternative<ElementId>(owner_);
  }

  [[nodiscard]] constexpr const ViewId* view_id() const {
    return std::get_if<ViewId>(&owner_);
  }

  [[nodiscard]] constexpr const ElementId* element_id() const {
    return std::get_if<ElementId>(&owner_);
  }

  friend bool operator==(
      const PointerCaptureOwner&,
      const PointerCaptureOwner&) = default;

 private:
  using Owner = std::variant<ViewId, ElementId>;

  constexpr explicit PointerCaptureOwner(ViewId view_id) : owner_(view_id) {}
  constexpr explicit PointerCaptureOwner(ElementId element_id)
      : owner_(element_id) {}

  Owner owner_;
};

struct EventRoute {
  ViewId target_view_id;
  std::optional<ElementId> target_element_id;
  std::vector<ElementId> element_ancestry;
  std::vector<ViewId> view_ancestry;
  EventKind event_kind = EventKind::unknown;
};

class EventRouter {
 public:
  [[nodiscard]] static EventRoute route_to_root(
      const PlatformEvent& event,
      ViewId root_view_id);
};

struct EventDispatchRecord {
  int sequence = 0;
  ViewId view_id;
  EventKind event_kind = EventKind::unknown;
  EventRoute route;
  EventResult result;
};

struct KeyBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  std::string action_name;
};

struct TextEditBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  TextEditAction edit_action = TextEditAction::move_previous;
};

struct InvalidationState {
  bool render = false;
  bool layout = false;
  bool paint = false;
};

struct FrameStatistics {
  int frame_index = 0;
  std::size_t render_pass_count = 0;
  std::size_t layout_pass_count = 0;
  std::size_t paint_pass_count = 0;
  std::size_t paint_command_count = 0;
  std::size_t submitted_command_count = 0;
  std::size_t skipped_command_count = 0;
  std::size_t solid_rect_command_count = 0;
  std::size_t rounded_rect_command_count = 0;
  std::size_t text_command_count = 0;
  std::size_t text_selection_command_count = 0;
  std::size_t text_caret_command_count = 0;
  std::size_t clip_stack_command_count = 0;
  std::size_t max_clip_stack_depth = 0;
  std::size_t composition_stack_command_count = 0;
  std::size_t max_composition_stack_depth = 0;
  std::size_t begin_frame_count = 0;
  std::size_t clear_count = 0;
  std::size_t present_count = 0;
  double frame_time_ms = 0.0;
  double render_time_ms = 0.0;
  double layout_time_ms = 0.0;
  double paint_time_ms = 0.0;
};

struct RenderRecord {
  int sequence = 0;
  ViewId view_id;
  Size viewport_size;
  std::optional<ElementId> root_element_id;
  std::optional<FrameStatistics> statistics;
};

struct RuntimeDiagnosticsSnapshot {
  std::size_t entity_store_count = 0;
  std::size_t entity_count = 0;
  std::size_t view_entity_subscription_count = 0;
  std::size_t entity_observer_count = 0;
  std::size_t connected_subscription_count = 0;
  InvalidationState invalidation;
  int frame_index = 0;
  std::optional<RenderRecord> last_render_record;
  std::optional<FrameStatistics> last_frame_statistics;
  std::vector<PlatformDiagnosticEvent> platform_diagnostics;
};

struct EntitySubscription {
  ViewId view_id;
  std::type_index entity_type{typeid(void)};
  std::uint64_t entity_id_value = 0;
};

struct EntityObserver {
  SubscriptionId subscription_id;
  std::type_index entity_type{typeid(void)};
  std::uint64_t entity_id_value = 0;
  std::function<void(const WindowRuntimeContext&, std::uint64_t)> callback;
};

struct ViewInputState {
  bool focused = false;
  bool pointer_captured = false;
  std::optional<PointerCaptureOwner> pointer_capture_owner;
  bool keyboard_focused = false;
  std::optional<ViewId> keyboard_focus_owner;
  std::optional<ElementId> keyboard_focus_element_owner;
  std::optional<ElementId> hovered_element_id;
  CursorShape cursor_shape = CursorShape::default_arrow;
  Point pointer_position{};
};

class FocusHandle {
 public:
  constexpr FocusHandle() = default;
  explicit constexpr FocusHandle(ElementId id) : id_(id) {}

  [[nodiscard]] constexpr ElementId id() const { return id_; }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }

  void request(WindowRuntime& runtime) const;
  void request(const WindowRuntimeContext& context) const;
  void release(WindowRuntime& runtime) const;
  void release(const WindowRuntimeContext& context) const;

  [[nodiscard]] bool contains(const ViewInputState& input) const;
  [[nodiscard]] bool contains(const WindowRuntime& runtime) const;
  [[nodiscard]] bool contains(const WindowRuntimeContext& context) const;
  [[nodiscard]] bool focused(const ViewInputState& input) const;
  [[nodiscard]] bool focused(const WindowRuntime& runtime) const;
  [[nodiscard]] bool focused(const WindowRuntimeContext& context) const;

 private:
  ElementId id_{};
};

struct WindowRuntimeContext {
  WindowRuntime& runtime;
  PlatformApplication& application;
  PlatformWindow& window;
  Renderer& renderer;
  ViewId view_id;
  Size viewport_size;
  DpiScale scale;
  ViewInputState input;
  std::optional<EventRoute> event_route;
  EventResult last_event_result;
  std::optional<EventDispatchRecord> last_event_dispatch;
  int frame_index = 0;

  [[nodiscard]] std::optional<EventRoute> current_event_route() const;
  [[nodiscard]] ViewInputState input_state() const;
  [[nodiscard]] ViewId allocate_view_id() const;
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> upgrade_view(WeakView view) const;
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
  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback) const;
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

using RendererFactory =
    std::function<Result<Renderer*>(const RenderSurfaceDescriptor&)>;
using AppRendererFactory = std::function<Result<std::unique_ptr<Renderer>>(
    const RenderSurfaceDescriptor&)>;
using WindowRuntimeFrameCallback =
    std::function<void(const WindowRuntimeContext&)>;
using WindowRuntimeRenderCallback =
    std::function<void(const WindowRuntimeContext&, const RenderRecord&)>;
using WindowRuntimeEventCallback =
    std::function<void(const WindowRuntimeContext&, const EventDispatchRecord&)>;
using WindowRuntimeErrorCallback =
    std::function<void(const Error&)>;
using AppSetupCallback = std::function<void(WindowRuntime&)>;
using AppContextSetupCallback = std::function<void(AppContext&)>;

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

  [[nodiscard]] int run(
      const WindowDescriptor& descriptor,
      WindowRuntimeOptions options = {});
  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options);
  [[nodiscard]] AppOpenedWindow open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view);
  [[nodiscard]] std::span<const AppOpenedWindow> app_opened_windows() const;
  [[nodiscard]] WindowRuntimeId root_window_runtime_id() const;
  [[nodiscard]] std::span<const WindowRuntimeRecord>
  window_runtime_records() const;
  [[nodiscard]] const WindowRuntimeRecord* window_runtime_record(
      WindowRuntimeId runtime_id) const;
  [[nodiscard]] const View* app_opened_window_root_view(
      ViewId root_view_id) const;
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
  void register_app_action(std::string name, ActionHandler handler);
  void register_window_action(std::string name, ActionHandler handler);
  void register_view_action(
      ViewId view_id,
      std::string name,
      ActionHandler handler);
  void register_focused_element_action(
      ElementId element_id,
      std::string name,
      ActionHandler handler);
  [[nodiscard]] ActionDispatchResult dispatch_action(std::string name);
  [[nodiscard]] std::optional<ActionDispatchResult> last_action_dispatch() const;
  void register_command_palette_entry(CommandPaletteEntry entry);
  [[nodiscard]] std::span<const CommandPaletteEntry> command_palette_entries()
      const;
  [[nodiscard]] std::vector<CommandPaletteEntry>
  command_palette_entries_for_group(std::string_view group) const;
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
  [[nodiscard]] bool paste_clipboard_text();
  [[nodiscard]] bool copy_selection_to_clipboard();
  [[nodiscard]] bool cut_selection_to_clipboard();
  void set_element_cursor(ElementId element_id, CursorShape cursor_shape);
  void request_render();
  void request_layout();
  void request_paint();
  void defer(DeferredCallback callback);
  [[nodiscard]] TimerId schedule_timer(
      std::uint64_t delay_ms,
      TimerCallback callback);
  [[nodiscard]] TimerId schedule_repeating_timer(
      std::uint64_t interval_ms,
      TimerCallback callback);
  [[nodiscard]] bool cancel_timer(TimerId id);
  void advance_time(std::uint64_t delta_ms);
  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback);
  [[nodiscard]] bool complete_task(TaskId id);
  void drain_task_completions();
  void batch_updates(UpdateBatchCallback callback);
  void clear_invalidation();
  [[nodiscard]] InvalidationState invalidation_state() const;
  [[nodiscard]] NativeMenuInstallation install_native_menu(
      NativeMenuModel menu);
  [[nodiscard]] const NativeMenuInstallation& native_menu_installation() const;
  [[nodiscard]] NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options);
  [[nodiscard]] const NativeFileDialogResult& native_file_dialog_result()
      const;
  [[nodiscard]] std::optional<RenderRecord> last_render_record() const;
  [[nodiscard]] RuntimeDiagnosticsSnapshot diagnostics_snapshot() const;
  [[nodiscard]] std::span<const PlatformDiagnosticEvent>
  platform_diagnostics() const;
  [[nodiscard]] std::span<const EntitySubscription> subscriptions_for_view(
      ViewId view_id) const;
  [[nodiscard]] bool subscription_connected(SubscriptionId id) const;
  [[nodiscard]] bool remove_subscription(SubscriptionId id);
  [[nodiscard]] ViewId allocate_view_id();
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> upgrade_view(WeakView view) const;
  Result<void> resize_surface(Size size, DpiScale scale);

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
  bool notify_entity_changed(EntityId<T> entity_id);

 private:
  friend class TaskHandle;

  void handle_event(const PlatformEvent& event);
  void handle_resize(const WindowResized& event);
  void handle_redraw();
  void schedule_redraw();
  void flush_deferred_redraw_request();
  void request_platform_wakeup();
  void handle_wakeup();
  void drain_deferred_callbacks();
  void fire_due_timers();
  void update_platform_accessibility_tree();
  [[nodiscard]] PlatformAccessibilityTreeUpdate build_platform_accessibility_update()
      const;
  [[nodiscard]] bool task_active(TaskId id) const;
  [[nodiscard]] bool task_complete(TaskId id) const;
  void apply_cursor_shape(CursorShape cursor_shape);
  void apply_focused_text_ime_placement();
  void record_platform_diagnostic(PlatformDiagnosticEvent event);
  void fail_and_quit(Error error);
  void activate_native_window_for_record(WindowRuntimeRecord& record);
  void handle_native_additional_window_event(
      WindowRuntimeId runtime_id,
      const PlatformEvent& event);
  void deactivate_native_additional_windows();
  [[nodiscard]] WindowRuntimeId allocate_window_runtime_id();
  [[nodiscard]] WindowRuntimeRecord* find_window_runtime_record(
      WindowRuntimeId runtime_id);
  [[nodiscard]] const WindowRuntimeRecord* find_window_runtime_record(
      WindowRuntimeId runtime_id) const;
  void refresh_route_ancestry(EventRoute& route) const;
  [[nodiscard]] std::vector<ElementId> element_ancestry_for(
      ElementId element_id) const;
  [[nodiscard]] std::vector<ViewId> view_ancestry_for(
      ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> child_view_target_for(
      ElementId element_id) const;
  [[nodiscard]] bool focus_next_element(bool reverse);
  [[nodiscard]] std::optional<ViewId> action_dispatch_view_id() const;
  [[nodiscard]] ScrollState* scroll_state_for_route(const EventRoute& route);
  void record_lifecycle_event(const PlatformEvent& event);
  [[nodiscard]] bool apply_text_pointer_selection(
      const PlatformEvent& event,
      const EventRoute& route);
  [[nodiscard]] std::optional<std::size_t> text_offset_for_point(
      const TextElement& element,
      Point point) const;
  [[nodiscard]] TextInputElement* routed_text_input(ElementId element_id);
  [[nodiscard]] const TextInputElement* routed_text_input(
      ElementId element_id) const;
  [[nodiscard]] EventResult dispatch_routed_element_event(
      const PlatformEvent& event,
      const EventRoute& route);
  [[nodiscard]] Element* routed_element(ElementId element_id);
  [[nodiscard]] const Element* routed_element(ElementId element_id) const;
  [[nodiscard]] WindowRuntimeContext context();
  template <typename T>
  [[nodiscard]] EntityStore<T>& entity_store();
  template <typename T>
  [[nodiscard]] const EntityStore<T>* find_entity_store() const;
  template <typename T>
  [[nodiscard]] EntityStore<T>* find_entity_store();
  [[nodiscard]] bool notify_entity_changed(
      std::type_index entity_type,
      std::uint64_t entity_id_value);

  struct RegisteredView {
    View* view = nullptr;
    std::unique_ptr<View> owned_view;
  };

  struct RuntimeTimer {
    TimerId id;
    std::uint64_t due_ms = 0;
    std::uint64_t interval_ms = 0;
    bool repeating = false;
    TimerCallback callback;
  };

  struct RuntimeTask {
    TaskId id;
    TaskCompletionCallback callback;
    bool queued = false;
    bool completed = false;
  };

  struct TextPointerSelectionDrag {
    ElementId element_id;
    std::size_t anchor_offset = 0;
  };

  PlatformApplication& application_;
  View& view_;
  RendererFactory renderer_factory_;
  WindowRuntimeFrameCallback after_frame_callback_;
  WindowRuntimeRenderCallback after_render_callback_;
  WindowRuntimeEventCallback after_event_callback_;
  WindowRuntimeFrameCallback close_requested_callback_;
  WindowRuntimeErrorCallback error_callback_;
  PlatformWindow* window_ = nullptr;
  Renderer* renderer_ = nullptr;
  Size framebuffer_size_{};
  Size viewport_size_{};
  DpiScale scale_{};
  ViewInputState input_{};
  std::optional<PointerCaptureOwner> pointer_capture_owner_;
  std::optional<ViewId> keyboard_focus_owner_;
  std::optional<ElementId> keyboard_focus_element_owner_;
  std::optional<TextPointerSelectionDrag> text_pointer_selection_drag_;
  std::optional<ElementId> hovered_element_id_;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  CursorShape applied_cursor_shape_ = CursorShape::default_arrow;
  EventResult last_event_result_{};
  std::optional<RenderRecord> last_render_record_;
  std::optional<FrameStatistics> last_frame_statistics_;
  std::optional<EventDispatchRecord> last_event_dispatch_;
  std::optional<ActionDispatchResult> last_action_dispatch_;
  std::optional<EventRoute> current_event_route_;
  std::unique_ptr<ElementTree> owned_element_tree_;
  const Element* element_root_ = nullptr;
  ViewId root_view_id_{1};
  std::uint64_t next_view_id_ = 2;
  std::unordered_map<std::uint64_t, RegisteredView> view_registry_;
  std::unordered_set<std::uint64_t> removed_view_ids_;
  int event_dispatch_sequence_ = 0;
  int render_sequence_ = 0;
  int frame_index_ = 0;
  std::unordered_map<std::type_index, std::any> globals_;
  std::unordered_map<std::type_index, std::any> entity_stores_;
  std::size_t entity_count_ = 0;
  std::unordered_map<std::string, ActionHandler> action_handlers_;
  std::unordered_map<std::string, ActionHandler> window_action_handlers_;
  std::unordered_map<
      std::uint64_t,
      std::unordered_map<std::string, ActionHandler>>
      view_action_handlers_;
  std::unordered_map<
      std::uint64_t,
      std::unordered_map<std::string, ActionHandler>>
      focused_element_action_handlers_;
  std::vector<CommandPaletteEntry> command_palette_entries_;
  std::vector<KeyBinding> key_bindings_;
  std::vector<TextEditBinding> text_edit_bindings_;
  std::unordered_map<std::uint64_t, TextModel*> text_models_;
  Clipboard* clipboard_ = nullptr;
  std::unordered_map<std::uint64_t, CursorShape> element_cursors_;
  std::optional<ImeTextInputPlacement> applied_ime_text_input_placement_;
  std::optional<PlatformAccessibilityTreeUpdate>
      last_platform_accessibility_update_;
  std::vector<PlatformDiagnosticEvent> platform_diagnostics_;
  int platform_diagnostic_sequence_ = 0;
  std::vector<EntitySubscription> entity_subscriptions_;
  std::vector<EntityObserver> entity_observers_;
  std::uint64_t next_subscription_id_ = 1;
  std::vector<DeferredCallback> deferred_callbacks_;
  std::vector<RuntimeTimer> timers_;
  std::uint64_t next_timer_id_ = 1;
  std::uint64_t current_time_ms_ = 0;
  bool firing_timers_ = false;
  std::vector<RuntimeTask> tasks_;
  std::vector<TaskId> task_completion_queue_;
  std::uint64_t next_task_id_ = 1;
  bool draining_task_completions_ = false;
  bool handling_wakeup_ = false;
  int update_batch_depth_ = 0;
  std::vector<AppOpenedWindow> app_opened_windows_;
  WindowRuntimeId root_window_runtime_id_{1};
  std::uint64_t next_window_runtime_id_ = 2;
  std::vector<WindowRuntimeRecord> window_runtime_records_;
  std::vector<std::unique_ptr<PlatformWindow>> native_additional_windows_;
  NativeMenuInstallation native_menu_installation_;
  NativeFileDialogResult native_file_dialog_result_;
  mutable std::vector<EntitySubscription> subscription_query_buffer_;
  InvalidationState invalidation_state_;
  bool dispatching_view_event_ = false;
  bool draining_deferred_callbacks_ = false;
  bool redraw_scheduled_ = false;
  bool deferred_redraw_request_ = false;
  bool should_quit_ = false;
  bool failed_ = false;
};

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    FrameStatistics* statistics = nullptr);
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    FrameStatistics* statistics = nullptr);
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    FrameStatistics* statistics = nullptr);
[[nodiscard]] int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options = {});

template <typename T>
void AppContext::set_global(T global_value) const {
  runtime.set_global<T>(std::move(global_value));
}

template <typename T>
const T* AppContext::global() const {
  return runtime.global<T>();
}

template <typename T, typename Update>
bool AppContext::update_global(Update&& update) const {
  return runtime.update_global<T>(std::forward<Update>(update));
}

template <typename T>
void WindowRuntimeContext::set_global(T global_value) const {
  runtime.set_global<T>(std::move(global_value));
}

template <typename T>
const T* WindowRuntimeContext::global() const {
  return runtime.global<T>();
}

template <typename T, typename Update>
bool WindowRuntimeContext::update_global(Update&& update) const {
  return runtime.update_global<T>(std::forward<Update>(update));
}

template <typename T>
T* WindowRuntimeContext::element_state(ElementId element_id) const {
  return runtime.element_state<T>(element_id);
}

template <typename T, typename... Args>
T* WindowRuntimeContext::emplace_element_state(
    ElementId element_id,
    Args&&... args) const {
  return runtime.emplace_element_state<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* WindowRuntimeContext::element_state_or_init(
    ElementId element_id,
    Args&&... args) const {
  return runtime.element_state_or_init<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T>
EntityId<T> WindowRuntimeContext::insert_entity(T entity) const {
  return runtime.insert_entity<T>(std::move(entity));
}

template <typename T, typename... Args>
EntityId<T> WindowRuntimeContext::emplace_entity(Args&&... args) const {
  return runtime.emplace_entity<T>(std::forward<Args>(args)...);
}

template <typename T>
const T* WindowRuntimeContext::read_entity(EntityId<T> id) const {
  return runtime.read_entity(id);
}

template <typename T>
T* WindowRuntimeContext::mutate_entity(EntityId<T> id) const {
  return runtime.mutate_entity(id);
}

template <typename T>
bool WindowRuntimeContext::remove_entity(EntityId<T> id) const {
  return runtime.remove_entity(id);
}

template <typename T, typename... Args>
Model<T> WindowRuntimeContext::new_model(Args&&... args) const {
  return runtime.emplace_entity<T>(std::forward<Args>(args)...);
}

template <typename T>
const T* WindowRuntimeContext::read_model(Model<T> model) const {
  return runtime.read_entity(model);
}

template <typename T>
std::optional<Model<T>> WindowRuntimeContext::upgrade_entity(
    WeakEntity<T> entity) const {
  return runtime.upgrade_entity(entity);
}

template <typename T, typename Update>
bool WindowRuntimeContext::update_model(Model<T> model, Update&& update) const {
  T* stored_model = runtime.mutate_entity(model);
  if (stored_model == nullptr) {
    return false;
  }
  std::forward<Update>(update)(*stored_model);
  return runtime.notify_entity_changed(model);
}

template <typename T, typename Observer>
bool WindowRuntimeContext::observe_model(
    Model<T> model,
    Observer&& observer) const {
  return runtime.observe_model(model, std::forward<Observer>(observer));
}

template <typename T, typename Observer>
Subscription WindowRuntimeContext::observe_model_subscription(
    Model<T> model,
    Observer&& observer) const {
  return runtime.observe_model_subscription(
      model,
      std::forward<Observer>(observer));
}

template <typename T>
bool WindowRuntimeContext::remove_model(Model<T> model) const {
  return runtime.remove_entity(model);
}

template <typename T>
void WindowRuntimeContext::subscribe_view_to_entity(
    ViewId view_id,
    EntityId<T> entity_id) const {
  runtime.subscribe_view_to_entity(view_id, entity_id);
}

template <typename T>
EntityId<T> WindowRuntime::insert_entity(T entity) {
  const EntityId<T> id = entity_store<T>().insert(std::move(entity));
  entity_count_ += 1;
  return id;
}

template <typename T, typename... Args>
EntityId<T> WindowRuntime::emplace_entity(Args&&... args) {
  const EntityId<T> id =
      entity_store<T>().emplace(std::forward<Args>(args)...);
  entity_count_ += 1;
  return id;
}

template <typename T>
const T* WindowRuntime::read_entity(EntityId<T> id) const {
  const EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return nullptr;
  }
  return store->get(id);
}

template <typename T>
std::optional<EntityId<T>> WindowRuntime::upgrade_entity(
    WeakEntity<T> entity) const {
  if (entity.empty() || read_entity(entity.id()) == nullptr) {
    return std::nullopt;
  }
  return entity.id();
}

template <typename T>
T* WindowRuntime::mutate_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return nullptr;
  }
  return store->get(id);
}

template <typename T, typename Observer>
bool WindowRuntime::observe_model(
    EntityId<T> entity_id,
    Observer&& observer) {
  ModelObserver<T> observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn || read_entity(entity_id) == nullptr) {
    return false;
  }

  entity_observers_.push_back(EntityObserver{
      .subscription_id = {},
      .entity_type = std::type_index(typeid(T)),
      .entity_id_value = entity_id.value,
      .callback =
          [observer = std::move(observer_fn)](
              const WindowRuntimeContext& context,
              std::uint64_t entity_id_value) {
            observer(context, Model<T>{entity_id_value});
          },
  });
  return true;
}

template <typename T, typename Observer>
Subscription WindowRuntime::observe_model_subscription(
    EntityId<T> entity_id,
    Observer&& observer) {
  ModelObserver<T> observer_fn{std::forward<Observer>(observer)};
  if (!observer_fn || read_entity(entity_id) == nullptr) {
    return {};
  }

  const SubscriptionId subscription_id{next_subscription_id_++};
  entity_observers_.push_back(EntityObserver{
      .subscription_id = subscription_id,
      .entity_type = std::type_index(typeid(T)),
      .entity_id_value = entity_id.value,
      .callback =
          [observer = std::move(observer_fn)](
              const WindowRuntimeContext& context,
              std::uint64_t entity_id_value) {
            observer(context, Model<T>{entity_id_value});
          },
  });
  return Subscription(*this, subscription_id);
}

template <typename T>
bool WindowRuntime::remove_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return false;
  }
  const bool removed = store->remove(id);
  if (removed) {
    if (entity_count_ > 0) {
      entity_count_ -= 1;
    }
    (void)notify_entity_changed(id);
  }
  return removed;
}

template <typename T>
void WindowRuntime::subscribe_view_to_entity(
    ViewId view_id,
    EntityId<T> entity_id) {
  if (view_id.value == 0 || entity_id.value == 0) {
    return;
  }

  const std::type_index entity_type(typeid(T));
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.view_id == view_id &&
        subscription.entity_type == entity_type &&
        subscription.entity_id_value == entity_id.value) {
      return;
    }
  }

  entity_subscriptions_.push_back(EntitySubscription{
      .view_id = view_id,
      .entity_type = entity_type,
      .entity_id_value = entity_id.value,
  });
}

template <typename T>
bool WindowRuntime::notify_entity_changed(EntityId<T> entity_id) {
  return notify_entity_changed(std::type_index(typeid(T)), entity_id.value);
}

template <typename T>
void WindowRuntime::set_global(T global_value) {
  globals_[std::type_index(typeid(T))] = std::move(global_value);
  request_render();
}

template <typename T>
const T* WindowRuntime::global() const {
  const auto entry = globals_.find(std::type_index(typeid(T)));
  if (entry == globals_.end()) {
    return nullptr;
  }
  return &std::any_cast<const T&>(entry->second);
}

template <typename T>
T* WindowRuntime::global() {
  auto entry = globals_.find(std::type_index(typeid(T)));
  if (entry == globals_.end()) {
    return nullptr;
  }
  return &std::any_cast<T&>(entry->second);
}

template <typename T, typename Update>
bool WindowRuntime::update_global(Update&& update) {
  T* stored_global = global<T>();
  if (stored_global == nullptr) {
    return false;
  }
  std::forward<Update>(update)(*stored_global);
  request_render();
  return true;
}

template <typename T>
T* WindowRuntime::element_state(ElementId element_id) {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->state<T>(element_id);
}

template <typename T>
const T* WindowRuntime::element_state(ElementId element_id) const {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->state<T>(element_id);
}

template <typename T, typename... Args>
T* WindowRuntime::emplace_element_state(ElementId element_id, Args&&... args) {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->emplace_state<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* WindowRuntime::element_state_or_init(
    ElementId element_id,
    Args&&... args) {
  if (owned_element_tree_ == nullptr) {
    return nullptr;
  }
  return owned_element_tree_->state_or_init<T>(
      element_id,
      std::forward<Args>(args)...);
}

template <typename T>
EntityStore<T>& WindowRuntime::entity_store() {
  auto [entry, inserted] = entity_stores_.try_emplace(
      std::type_index(typeid(T)),
      EntityStore<T>{});
  (void)inserted;
  return std::any_cast<EntityStore<T>&>(entry->second);
}

template <typename T>
const EntityStore<T>* WindowRuntime::find_entity_store() const {
  const auto entry = entity_stores_.find(std::type_index(typeid(T)));
  if (entry == entity_stores_.end()) {
    return nullptr;
  }
  return &std::any_cast<const EntityStore<T>&>(entry->second);
}

template <typename T>
EntityStore<T>* WindowRuntime::find_entity_store() {
  auto entry = entity_stores_.find(std::type_index(typeid(T)));
  if (entry == entity_stores_.end()) {
    return nullptr;
  }
  return &std::any_cast<EntityStore<T>&>(entry->second);
}

} // namespace cgpui
