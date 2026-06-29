#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
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

struct EventDispatchRecord {
  int sequence = 0;
  ViewId view_id;
  EventKind event_kind = EventKind::unknown;
  EventResult result;
};

struct ViewInputState {
  bool focused = false;
  bool pointer_captured = false;
  std::optional<ViewId> pointer_capture_owner;
  bool keyboard_focused = false;
  std::optional<ViewId> keyboard_focus_owner;
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
  EventResult last_event_result;
  std::optional<EventDispatchRecord> last_event_dispatch;
  int frame_index = 0;
};

using RendererFactory =
    std::function<Result<Renderer*>(const RenderSurfaceDescriptor&)>;
using WindowRuntimeFrameCallback =
    std::function<void(const WindowRuntimeContext&)>;
using WindowRuntimeEventCallback =
    std::function<void(const WindowRuntimeContext&, const EventDispatchRecord&)>;
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
  void capture_pointer();
  void capture_pointer(ViewId view_id);
  void release_pointer();
  void release_pointer(ViewId view_id);
  void request_keyboard_focus();
  void request_keyboard_focus(ViewId view_id);
  void release_keyboard_focus();
  void release_keyboard_focus(ViewId view_id);
  Result<void> resize_surface(Size size, DpiScale scale);

 private:
  void handle_event(const PlatformEvent& event);
  void handle_resize(const WindowResized& event);
  void handle_redraw();
  void fail_and_quit(Error error);
  [[nodiscard]] WindowRuntimeContext context();

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
  std::optional<ViewId> keyboard_focus_owner_;
  EventResult last_event_result_{};
  std::optional<EventDispatchRecord> last_event_dispatch_;
  ViewId root_view_id_{1};
  int event_dispatch_sequence_ = 0;
  int frame_index_ = 0;
  bool should_quit_ = false;
  bool failed_ = false;
};

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size);

} // namespace cgpui
