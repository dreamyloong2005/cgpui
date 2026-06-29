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

std::optional<Point> pointer_position_for(const PlatformEvent& event) {
  if (const auto* moved = std::get_if<PointerMoved>(&event);
      moved != nullptr) {
    return moved->position;
  }
  if (const auto* button = std::get_if<PointerButton>(&event);
      button != nullptr) {
    return button->position;
  }
  if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
      scrolled != nullptr) {
    return scrolled->position;
  }
  return {};
}

bool is_keyboard_routed_event(const PlatformEvent& event) {
  return std::holds_alternative<KeyboardKey>(event) ||
         std::holds_alternative<TextInput>(event);
}

bool modifiers_equal(KeyboardModifiers lhs, KeyboardModifiers rhs) {
  return lhs.shift == rhs.shift && lhs.control == rhs.control &&
         lhs.alt == rhs.alt && lhs.super == rhs.super;
}

bool is_valid_pointer_capture_owner(const PointerCaptureOwner& owner) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    return view_id->value != 0;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    return element_id->value != 0;
  }
  return false;
}

void apply_pointer_capture_owner_to_route(
    const PointerCaptureOwner& owner,
    EventRoute& route) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    route.target_view_id = *view_id;
    return;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    route.target_element_id = *element_id;
  }
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

void StyledElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (bounds.has_value() && style_.background_color.has_value()) {
    paint_list.fill_rect(*bounds, *style_.background_color);
  }
  if (child_ != nullptr) {
    child_->paint(paint_list);
  }
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
  keyboard_focus_element_owner_.reset();
  hovered_element_id_.reset();
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_.reset();
  last_action_dispatch_.reset();
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
    std::optional<ElementId> hit_element_id;
    if (element_root_ != nullptr) {
      if (const std::optional<Point> pointer_position =
              pointer_position_for(event);
          pointer_position.has_value()) {
        const ElementId hit = element_root_->hit_test(*pointer_position);
        if (hit.value != 0) {
          hit_element_id = hit;
        }
        if (std::holds_alternative<PointerMoved>(event)) {
          hovered_element_id_ = hit_element_id;
        }
      }
    } else if (std::holds_alternative<PointerMoved>(event)) {
      hovered_element_id_.reset();
    }
    if (keyboard_focus_element_owner_.has_value() &&
        is_keyboard_routed_event(event)) {
      current_event_route_->target_element_id = keyboard_focus_element_owner_;
    } else if (pointer_capture_owner_.has_value() &&
        pointer_position_for(event).has_value()) {
      apply_pointer_capture_owner_to_route(
          *pointer_capture_owner_,
          *current_event_route_);
    } else if (hit_element_id.has_value()) {
      current_event_route_->target_element_id = hit_element_id;
    }
    if (const auto* key = std::get_if<KeyboardKey>(&event); key != nullptr) {
      for (const KeyBinding& binding : key_bindings_) {
        if (binding.key_code == key->key_code &&
            binding.action == key->action &&
            modifiers_equal(binding.modifiers, key->modifiers)) {
          (void)dispatch_action(binding.action_name);
          break;
        }
      }
    }
    if (const auto* text = std::get_if<TextInput>(&event);
        text != nullptr && keyboard_focus_element_owner_.has_value()) {
      const auto model =
          text_models_.find(keyboard_focus_element_owner_->value);
      if (model != text_models_.end() && model->second != nullptr) {
        model->second->insert_text(text->text);
      }
    }
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
  input.pointer_captured = pointer_capture_owner_.has_value();
  input.keyboard_focus_owner = keyboard_focus_owner_;
  input.keyboard_focus_element_owner = keyboard_focus_element_owner_;
  input.hovered_element_id = hovered_element_id_;
  input.keyboard_focused = keyboard_focus_owner_ == root_view_id_ ||
                           keyboard_focus_element_owner_.has_value();

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

void WindowRuntime::set_element_root(const Element* element) {
  element_root_ = element;
}

void WindowRuntime::capture_pointer(PointerCaptureOwner owner) {
  if (is_valid_pointer_capture_owner(owner)) {
    pointer_capture_owner_ = owner;
  }
}

void WindowRuntime::release_pointer(PointerCaptureOwner owner) {
  if (pointer_capture_owner_ == owner) {
    pointer_capture_owner_.reset();
  }
}

void WindowRuntime::request_keyboard_focus() {
  request_keyboard_focus(root_view_id_);
}

void WindowRuntime::request_keyboard_focus(ViewId view_id) {
  keyboard_focus_owner_ = view_id;
}

void WindowRuntime::request_keyboard_focus(ElementId element_id) {
  if (element_id.value != 0) {
    keyboard_focus_element_owner_ = element_id;
  }
}

void WindowRuntime::release_keyboard_focus() {
  release_keyboard_focus(root_view_id_);
}

void WindowRuntime::release_keyboard_focus(ViewId view_id) {
  if (keyboard_focus_owner_ == view_id) {
    keyboard_focus_owner_.reset();
  }
}

void WindowRuntime::release_keyboard_focus(ElementId element_id) {
  if (keyboard_focus_element_owner_ == element_id) {
    keyboard_focus_element_owner_.reset();
  }
}

void WindowRuntime::register_action(std::string name, ActionHandler handler) {
  if (!name.empty() && handler) {
    action_handlers_[std::move(name)] = std::move(handler);
  }
}

ActionDispatchResult WindowRuntime::dispatch_action(std::string name) {
  ActionDispatchResult dispatch{
      .name = std::move(name),
      .result = EventResult::unhandled()};

  const auto handler = action_handlers_.find(dispatch.name);
  if (handler != action_handlers_.end()) {
    dispatch.handled = true;
    dispatch.result = handler->second(context());
  }

  last_action_dispatch_ = dispatch;
  return dispatch;
}

std::optional<ActionDispatchResult> WindowRuntime::last_action_dispatch()
    const {
  return last_action_dispatch_;
}

void WindowRuntime::bind_key(KeyBinding binding) {
  if (!binding.action_name.empty()) {
    key_bindings_.push_back(std::move(binding));
  }
}

void WindowRuntime::bind_text_model(ElementId element_id, TextModel* model) {
  if (element_id.value == 0) {
    return;
  }
  if (model == nullptr) {
    text_models_.erase(element_id.value);
    return;
  }
  text_models_[element_id.value] = model;
}

ViewId WindowRuntime::allocate_view_id() {
  const ViewId view_id{next_view_id_};
  next_view_id_ += 1;
  return view_id;
}

bool WindowRuntime::is_view_id_allocated(ViewId view_id) const {
  return view_id.value != 0 && view_id.value < next_view_id_;
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

ViewId WindowRuntimeContext::allocate_view_id() const {
  return runtime.allocate_view_id();
}

bool WindowRuntimeContext::is_view_id_allocated(ViewId view_id) const {
  return runtime.is_view_id_allocated(view_id);
}

} // namespace cgpui
