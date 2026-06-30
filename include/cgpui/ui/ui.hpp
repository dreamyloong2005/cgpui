#pragma once

#include "cgpui/core/entity.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/text.hpp"

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
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

using ActionHandler =
    std::function<EventResult(const WindowRuntimeContext&)>;
using FocusedTextModelMutation = std::function<void(TextModel&)>;

template <typename T>
using ModelObserver =
    std::function<void(const WindowRuntimeContext&, Model<T>)>;

enum class PaintCommandKind {
  solid_rect,
  rounded_rect,
};

struct RoundedRect {
  Rect rect;
  Color color;
  BorderRadii radius;
};

struct PaintCommand {
  PaintCommandKind kind = PaintCommandKind::solid_rect;
  SolidRect solid_rect;
  RoundedRect rounded_rect;
  std::optional<Rect> clip_rect;
};

class PaintList {
 public:
  void clear();
  void push_clip(Rect rect);
  void pop_clip();
  void fill_rect(Rect rect, Color color);
  void fill_rounded_rect(Rect rect, Color color, BorderRadii radius);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
  std::vector<Rect> clip_stack_;
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

struct WindowOptions {
  WindowDescriptor descriptor;

  WindowOptions& title(std::string title);
  WindowOptions& size(Size size);
  WindowOptions& size(float width, float height);
  [[nodiscard]] WindowDescriptor to_descriptor() const;
};

struct AppOpenedWindow {
  WindowDescriptor descriptor;
  ViewId root_view_id;
};

struct AppContext {
  WindowRuntime& runtime;

  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options) const;
  [[nodiscard]] AppOpenedWindow open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view) const;
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

enum class EventKind {
  unknown,
  window_focused,
  pointer_moved,
  pointer_button,
  pointer_scrolled,
  keyboard_key,
  text_input,
  ime_composition,
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

struct ActionDispatchResult {
  std::string name;
  bool handled = false;
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

struct RenderRecord {
  int sequence = 0;
  ViewId view_id;
  Size viewport_size;
  std::optional<ElementId> root_element_id;
};

struct EntitySubscription {
  ViewId view_id;
  std::type_index entity_type{typeid(void)};
  std::uint64_t entity_id_value = 0;
};

struct EntityObserver {
  std::type_index entity_type{typeid(void)};
  std::uint64_t entity_id_value = 0;
  std::function<void(const WindowRuntimeContext&, std::uint64_t)> callback;
};

enum class CursorShape {
  default_arrow,
  pointing_hand,
  text,
  crosshair,
  resize_left_right,
  resize_up_down,
  not_allowed,
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

struct WindowRuntimeContext {
  WindowRuntime& runtime;
  PlatformApplication& application;
  PlatformWindow& window;
  Renderer& renderer;
  ViewId view_id;
  Size viewport_size;
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
  void set_element_tree(std::unique_ptr<ElementTree> tree) const;
  void register_action(std::string name, ActionHandler handler) const;
  [[nodiscard]] ActionDispatchResult dispatch_action(std::string name) const;
  [[nodiscard]] std::optional<ActionDispatchResult> last_action_dispatch() const;
  void bind_key(KeyBinding binding) const;
  void bind_text_edit_action(TextEditBinding binding) const;
  void bind_text_model(ElementId element_id, TextModel* model) const;
  [[nodiscard]] bool mutate_focused_text_model(
      FocusedTextModelMutation mutation) const;
  void set_element_cursor(ElementId element_id, CursorShape cursor_shape) const;
  [[nodiscard]] bool paste_clipboard_text() const;
  [[nodiscard]] bool copy_selection_to_clipboard() const;
  [[nodiscard]] bool cut_selection_to_clipboard() const;
  void request_render() const;
  void request_layout() const;
  void request_paint() const;
  void clear_invalidation() const;
  [[nodiscard]] InvalidationState invalidation_state() const;

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
  void capture_pointer(PointerCaptureOwner owner);
  void release_pointer(PointerCaptureOwner owner);
  void request_keyboard_focus();
  void request_keyboard_focus(ViewId view_id);
  void request_keyboard_focus(ElementId element_id);
  void release_keyboard_focus();
  void release_keyboard_focus(ViewId view_id);
  void release_keyboard_focus(ElementId element_id);
  void register_action(std::string name, ActionHandler handler);
  [[nodiscard]] ActionDispatchResult dispatch_action(std::string name);
  [[nodiscard]] std::optional<ActionDispatchResult> last_action_dispatch() const;
  void bind_key(KeyBinding binding);
  void bind_text_edit_action(TextEditBinding binding);
  void bind_text_model(ElementId element_id, TextModel* model);
  [[nodiscard]] TextModel* focused_text_model();
  [[nodiscard]] const TextModel* focused_text_model() const;
  void set_clipboard(Clipboard* clipboard);
  [[nodiscard]] bool paste_clipboard_text();
  [[nodiscard]] bool copy_selection_to_clipboard();
  [[nodiscard]] bool cut_selection_to_clipboard();
  void set_element_cursor(ElementId element_id, CursorShape cursor_shape);
  void request_render();
  void request_layout();
  void request_paint();
  void clear_invalidation();
  [[nodiscard]] InvalidationState invalidation_state() const;
  [[nodiscard]] std::optional<RenderRecord> last_render_record() const;
  [[nodiscard]] std::span<const EntitySubscription> subscriptions_for_view(
      ViewId view_id) const;
  [[nodiscard]] ViewId allocate_view_id();
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> upgrade_view(WeakView view) const;
  Result<void> resize_surface(Size size, DpiScale scale);

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

  template <typename T>
  bool remove_entity(EntityId<T> id);

  template <typename T>
  void subscribe_view_to_entity(ViewId view_id, EntityId<T> entity_id);

  template <typename T>
  bool notify_entity_changed(EntityId<T> entity_id);

 private:
  void handle_event(const PlatformEvent& event);
  void handle_resize(const WindowResized& event);
  void handle_redraw();
  void schedule_redraw();
  void flush_deferred_redraw_request();
  void fail_and_quit(Error error);
  void refresh_route_ancestry(EventRoute& route) const;
  [[nodiscard]] std::vector<ElementId> element_ancestry_for(
      ElementId element_id) const;
  [[nodiscard]] std::vector<ViewId> view_ancestry_for(
      ViewId view_id) const;
  [[nodiscard]] std::optional<ViewId> child_view_target_for(
      ElementId element_id) const;
  [[nodiscard]] bool focus_next_element(bool reverse);
  [[nodiscard]] ScrollState* scroll_state_for_route(const EventRoute& route);
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
  Size viewport_size_{};
  ViewInputState input_{};
  std::optional<PointerCaptureOwner> pointer_capture_owner_;
  std::optional<ViewId> keyboard_focus_owner_;
  std::optional<ElementId> keyboard_focus_element_owner_;
  std::optional<ElementId> hovered_element_id_;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  EventResult last_event_result_{};
  std::optional<RenderRecord> last_render_record_;
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
  std::unordered_map<std::type_index, std::any> entity_stores_;
  std::unordered_map<std::string, ActionHandler> action_handlers_;
  std::vector<KeyBinding> key_bindings_;
  std::vector<TextEditBinding> text_edit_bindings_;
  std::unordered_map<std::uint64_t, TextModel*> text_models_;
  Clipboard* clipboard_ = nullptr;
  std::unordered_map<std::uint64_t, CursorShape> element_cursors_;
  std::vector<EntitySubscription> entity_subscriptions_;
  std::vector<EntityObserver> entity_observers_;
  std::vector<AppOpenedWindow> app_opened_windows_;
  mutable std::vector<EntitySubscription> subscription_query_buffer_;
  InvalidationState invalidation_state_;
  bool dispatching_view_event_ = false;
  bool redraw_scheduled_ = false;
  bool deferred_redraw_request_ = false;
  bool should_quit_ = false;
  bool failed_ = false;
};

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size);
[[nodiscard]] int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options = {});

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
  return entity_store<T>().insert(std::move(entity));
}

template <typename T, typename... Args>
EntityId<T> WindowRuntime::emplace_entity(Args&&... args) {
  return entity_store<T>().emplace(std::forward<Args>(args)...);
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

template <typename T>
bool WindowRuntime::remove_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return false;
  }
  const bool removed = store->remove(id);
  if (removed) {
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
