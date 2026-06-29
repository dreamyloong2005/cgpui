#include "cgpui/ui/ui.hpp"

#include <expected>
#include <memory>
#include <utility>
#include <variant>

namespace cgpui {
namespace {

EventKind event_kind_for(const PlatformEvent& event) {
  if (std::holds_alternative<WindowFocused>(event)) {
    return EventKind::window_focused;
  }
  if (std::holds_alternative<PointerMoved>(event)) {
    return EventKind::pointer_moved;
  }
  if (std::holds_alternative<PointerButton>(event)) {
    return EventKind::pointer_button;
  }
  if (std::holds_alternative<PointerScrolled>(event)) {
    return EventKind::pointer_scrolled;
  }
  if (std::holds_alternative<KeyboardKey>(event)) {
    return EventKind::keyboard_key;
  }
  if (std::holds_alternative<TextInput>(event)) {
    return EventKind::text_input;
  }
  return EventKind::unknown;
}

} // namespace

void PaintList::clear() {
  commands_.clear();
}

void PaintList::fill_rect(Rect rect, Color color) {
  commands_.push_back(PaintCommand{.solid_rect = SolidRect{.rect = rect, .color = color}});
}

std::span<const PaintCommand> PaintList::commands() const {
  return commands_;
}

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size) {
  auto frame = renderer.begin_frame();
  if (!frame) {
    return std::unexpected(frame.error());
  }
  if (!*frame) {
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "Renderer returned an empty frame",
    });
  }

  (*frame)->clear(Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});

  PaintList paint_list;
  view.paint(paint_list, viewport_size);
  for (const auto& command : paint_list.commands()) {
    (*frame)->draw_rect(command.solid_rect);
  }

  return (*frame)->present();
}

EventResult View::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  (void)event;
  (void)context;
  return EventResult::unhandled();
}

EventRoute EventRouter::route_to_root(
    const PlatformEvent& event,
    ViewId root_view_id) {
  return EventRoute{
      .target_view_id = root_view_id,
      .event_kind = event_kind_for(event)};
}

WindowRuntime::WindowRuntime(
    PlatformApplication& application,
    View& view,
    RendererFactory renderer_factory)
    : application_(application),
      view_(view),
      renderer_factory_(std::move(renderer_factory)) {}

int WindowRuntime::run(
    const WindowDescriptor& descriptor,
    WindowRuntimeOptions options) {
  should_quit_ = false;
  failed_ = false;
  window_ = nullptr;
  renderer_ = nullptr;
  viewport_size_ = descriptor.size;
  input_ = {};
  pointer_capture_owner_.reset();
  keyboard_focus_owner_.reset();
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_.reset();
  current_event_route_.reset();
  event_dispatch_sequence_ = 0;
  frame_index_ = 0;

  auto window_result = application_.create_window(
      descriptor,
      [this](const PlatformEvent& event) { handle_event(event); });
  if (!window_result) {
    if (error_callback_) {
      error_callback_(window_result.error());
    }
    return 1;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  window_ = window.get();
  const WindowState window_state = window_->state();
  viewport_size_ = window_state.framebuffer_size;

  auto renderer_result = renderer_factory_(RenderSurfaceDescriptor{
      .native_surface = window_->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale});
  if (!renderer_result || *renderer_result == nullptr) {
    if (error_callback_) {
      if (renderer_result) {
        error_callback_(Error{
            .code = ErrorCode::renderer_initialization_failed,
            .message = "Renderer factory returned an empty renderer"});
      } else {
        error_callback_(renderer_result.error());
      }
    }
    return 1;
  }
  renderer_ = *renderer_result;

  if (options.request_initial_redraw) {
    window_->request_redraw();
  }

  const int run_result = application_.run();
  window_ = nullptr;
  renderer_ = nullptr;

  if (failed_) {
    return 1;
  }
  return run_result;
}

void WindowRuntime::handle_event(const PlatformEvent& event) {
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    should_quit_ = true;
    if (close_requested_callback_ && window_ != nullptr &&
        renderer_ != nullptr) {
      close_requested_callback_(context());
    }
    application_.quit();
    return;
  }

  if (const auto* resized = std::get_if<WindowResized>(&event);
      resized != nullptr) {
    handle_resize(*resized);
    return;
  }

  if (std::holds_alternative<WindowRedrawRequested>(event)) {
    handle_redraw();
    return;
  }

  if (window_ != nullptr && renderer_ != nullptr) {
    if (const auto* focused = std::get_if<WindowFocused>(&event);
        focused != nullptr) {
      input_.focused = focused->focused;
    } else if (const auto* moved = std::get_if<PointerMoved>(&event);
               moved != nullptr) {
      input_.pointer_position = moved->position;
    } else if (const auto* button = std::get_if<PointerButton>(&event);
               button != nullptr) {
      input_.pointer_position = button->position;
    } else if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
               scrolled != nullptr) {
      input_.pointer_position = scrolled->position;
    }
    current_event_route_ = EventRouter::route_to_root(event, root_view_id_);
    last_event_result_ = view_.handle_event(event, context());
    last_event_dispatch_ = EventDispatchRecord{
        .sequence = ++event_dispatch_sequence_,
        .view_id = current_event_route_->target_view_id,
        .event_kind = current_event_route_->event_kind,
        .route = *current_event_route_,
        .result = last_event_result_};
    if (after_event_callback_) {
      after_event_callback_(context(), *last_event_dispatch_);
    }
  }
}

void WindowRuntime::handle_resize(const WindowResized& event) {
  (void)resize_surface(event.size, event.scale);
}

void WindowRuntime::handle_redraw() {
  if (renderer_ == nullptr || should_quit_) {
    return;
  }

  auto result = render_view(*renderer_, view_, viewport_size_);
  if (!result) {
    fail_and_quit(result.error());
    return;
  }

  frame_index_ += 1;
  if (after_frame_callback_) {
    after_frame_callback_(context());
  }
}

void WindowRuntime::fail_and_quit(Error error) {
  failed_ = true;
  should_quit_ = true;
  if (error_callback_) {
    error_callback_(error);
  }
  application_.quit();
}

WindowRuntimeContext WindowRuntime::context() {
  ViewInputState input = input_;
  input.pointer_capture_owner = pointer_capture_owner_;
  input.pointer_captured = pointer_capture_owner_ == root_view_id_;
  input.keyboard_focus_owner = keyboard_focus_owner_;
  input.keyboard_focused = keyboard_focus_owner_ == root_view_id_;

  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      .window = *window_,
      .renderer = *renderer_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .input = input,
      .event_route = current_event_route_,
      .last_event_result = last_event_result_,
      .last_event_dispatch = last_event_dispatch_,
      .frame_index = frame_index_};
}

void WindowRuntime::set_after_frame_callback(
    WindowRuntimeFrameCallback callback) {
  after_frame_callback_ = std::move(callback);
}

void WindowRuntime::set_after_event_callback(
    WindowRuntimeEventCallback callback) {
  after_event_callback_ = std::move(callback);
}

void WindowRuntime::set_close_requested_callback(
    WindowRuntimeFrameCallback callback) {
  close_requested_callback_ = std::move(callback);
}

void WindowRuntime::set_error_callback(WindowRuntimeErrorCallback callback) {
  error_callback_ = std::move(callback);
}

void WindowRuntime::capture_pointer() {
  capture_pointer(root_view_id_);
}

void WindowRuntime::capture_pointer(ViewId view_id) {
  pointer_capture_owner_ = view_id;
}

void WindowRuntime::release_pointer() {
  release_pointer(root_view_id_);
}

void WindowRuntime::release_pointer(ViewId view_id) {
  if (pointer_capture_owner_ == view_id) {
    pointer_capture_owner_.reset();
  }
}

void WindowRuntime::request_keyboard_focus() {
  request_keyboard_focus(root_view_id_);
}

void WindowRuntime::request_keyboard_focus(ViewId view_id) {
  keyboard_focus_owner_ = view_id;
}

void WindowRuntime::release_keyboard_focus() {
  release_keyboard_focus(root_view_id_);
}

void WindowRuntime::release_keyboard_focus(ViewId view_id) {
  if (keyboard_focus_owner_ == view_id) {
    keyboard_focus_owner_.reset();
  }
}

Result<void> WindowRuntime::resize_surface(Size size, DpiScale scale) {
  viewport_size_ = size;
  if (renderer_ == nullptr) {
    return {};
  }

  auto result = renderer_->resize(size, scale);
  if (!result) {
    fail_and_quit(result.error());
  }
  return result;
}

} // namespace cgpui
