#pragma once

#include "cgpui/core/entity.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/element.hpp"

#include <any>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cgpui {

struct WindowRuntimeContext;

struct EventResult {
  bool consumed = false;
  bool cancelled = false;

  [[nodiscard]] static constexpr EventResult unhandled() {
    return {};
  }

  [[nodiscard]] static constexpr EventResult consumed_event() {
    return {.consumed = true};
  }

  [[nodiscard]] static constexpr EventResult cancelled_event() {
    return {.consumed = true, .cancelled = true};
  }
};

struct PaintCommand {
  SolidRect solid_rect;
};

class PaintList {
 public:
  void clear();
  void fill_rect(Rect rect, Color color);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
};

class View {
 public:
  virtual ~View() = default;
  virtual void paint(PaintList& paint_list, Size viewport_size) = 0;
  virtual EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
};

struct WindowRuntimeOptions {
  bool request_initial_redraw = true;
};

class WindowRuntime;

struct ViewId {
  std::uint64_t value = 0;

  friend bool operator==(ViewId, ViewId) = default;
};

enum class EventKind {
  unknown,
  window_focused,
  pointer_moved,
  pointer_button,
  pointer_scrolled,
  keyboard_key,
  text_input,
};

struct EventRoute {
  ViewId target_view_id;
  std::optional<ElementId> target_element_id;
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

struct ViewInputState {
  bool focused = false;
  bool pointer_captured = false;
  std::optional<ViewId> pointer_capture_owner;
  std::optional<ElementId> pointer_capture_element_owner;
  bool keyboard_focused = false;
  std::optional<ViewId> keyboard_focus_owner;
  std::optional<ElementId> keyboard_focus_element_owner;
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

  [[nodiscard]] ViewId allocate_view_id() const;
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;

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
};

using RendererFactory =
    std::function<Result<Renderer*>(const RenderSurfaceDescriptor&)>;
using WindowRuntimeFrameCallback =
    std::function<void(const WindowRuntimeContext&)>;
using WindowRuntimeEventCallback =
    std::function<void(const WindowRuntimeContext&, const EventDispatchRecord&)>;
using ActionHandler =
    std::function<EventResult(const WindowRuntimeContext&)>;
using WindowRuntimeErrorCallback =
    std::function<void(const Error&)>;

class WindowRuntime {
 public:
  WindowRuntime(
      PlatformApplication& application,
      View& view,
      RendererFactory renderer_factory);

  [[nodiscard]] int run(
      const WindowDescriptor& descriptor,
      WindowRuntimeOptions options = {});

  void set_after_frame_callback(WindowRuntimeFrameCallback callback);
  void set_after_event_callback(WindowRuntimeEventCallback callback);
  void set_close_requested_callback(WindowRuntimeFrameCallback callback);
  void set_error_callback(WindowRuntimeErrorCallback callback);
  void set_element_root(const Element* element);
  void capture_pointer();
  void capture_pointer(ViewId view_id);
  void capture_pointer(ElementId element_id);
  void release_pointer();
  void release_pointer(ViewId view_id);
  void release_pointer(ElementId element_id);
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
  [[nodiscard]] ViewId allocate_view_id();
  [[nodiscard]] bool is_view_id_allocated(ViewId view_id) const;
  Result<void> resize_surface(Size size, DpiScale scale);

  template <typename T>
  EntityId<T> insert_entity(T entity);

  template <typename T, typename... Args>
  EntityId<T> emplace_entity(Args&&... args);

  template <typename T>
  [[nodiscard]] const T* read_entity(EntityId<T> id) const;

  template <typename T>
  [[nodiscard]] T* mutate_entity(EntityId<T> id);

  template <typename T>
  bool remove_entity(EntityId<T> id);

 private:
  void handle_event(const PlatformEvent& event);
  void handle_resize(const WindowResized& event);
  void handle_redraw();
  void fail_and_quit(Error error);
  [[nodiscard]] WindowRuntimeContext context();
  template <typename T>
  [[nodiscard]] EntityStore<T>& entity_store();
  template <typename T>
  [[nodiscard]] const EntityStore<T>* find_entity_store() const;
  template <typename T>
  [[nodiscard]] EntityStore<T>* find_entity_store();

  PlatformApplication& application_;
  View& view_;
  RendererFactory renderer_factory_;
  WindowRuntimeFrameCallback after_frame_callback_;
  WindowRuntimeEventCallback after_event_callback_;
  WindowRuntimeFrameCallback close_requested_callback_;
  WindowRuntimeErrorCallback error_callback_;
  PlatformWindow* window_ = nullptr;
  Renderer* renderer_ = nullptr;
  Size viewport_size_{};
  ViewInputState input_{};
  std::optional<ViewId> pointer_capture_owner_;
  std::optional<ElementId> pointer_capture_element_owner_;
  std::optional<ViewId> keyboard_focus_owner_;
  std::optional<ElementId> keyboard_focus_element_owner_;
  EventResult last_event_result_{};
  std::optional<EventDispatchRecord> last_event_dispatch_;
  std::optional<ActionDispatchResult> last_action_dispatch_;
  std::optional<EventRoute> current_event_route_;
  const Element* element_root_ = nullptr;
  ViewId root_view_id_{1};
  std::uint64_t next_view_id_ = 2;
  int event_dispatch_sequence_ = 0;
  int frame_index_ = 0;
  std::unordered_map<std::type_index, std::any> entity_stores_;
  std::unordered_map<std::string, ActionHandler> action_handlers_;
  std::vector<KeyBinding> key_bindings_;
  bool should_quit_ = false;
  bool failed_ = false;
};

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size);

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
T* WindowRuntime::mutate_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return nullptr;
  }
  return store->get(id);
}

template <typename T>
bool WindowRuntime::remove_entity(EntityId<T> id) {
  EntityStore<T>* store = find_entity_store<T>();
  if (store == nullptr) {
    return false;
  }
  return store->remove(id);
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
