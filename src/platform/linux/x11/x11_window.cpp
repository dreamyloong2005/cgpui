#include "x11_window_internal.hpp"

namespace cgpui {

X11Window::X11Window(
    xcb_connection_t* connection,
    xcb_screen_t* screen,
    X11Atoms atoms,
    DpiScale scale,
    std::shared_ptr<X11KeyboardState> keyboard,
    xcb_cursor_context_t* cursor_context,
    PlatformEventCallback callback,
    std::function<void(X11Window*)> unregister)
    : connection_(connection),
      screen_(screen),
      atoms_(atoms),
      keyboard_(std::move(keyboard)),
      cursor_context_(cursor_context),
      callback_(std::move(callback)),
      unregister_(std::move(unregister)) {
  state_.scale = scale;
}

Result<std::unique_ptr<X11Window>> X11Window::create(
    xcb_connection_t* connection,
    xcb_screen_t* screen,
    const X11Atoms& atoms,
    DpiScale scale,
    std::shared_ptr<X11KeyboardState> keyboard,
    xcb_cursor_context_t* cursor_context,
    xcb_window_t parent,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    std::function<void(X11Window*)> unregister) {
  auto window = std::unique_ptr<X11Window>(new X11Window(
      connection, screen, atoms, scale, std::move(keyboard), cursor_context,
      std::move(callback), std::move(unregister)));
  auto initialized = window->initialize(parent, descriptor);
  if (!initialized) return std::unexpected(initialized.error());
  return window;
}

Result<void> X11Window::initialize(
    xcb_window_t parent,
    const WindowDescriptor& descriptor) {
  window_ = xcb_generate_id(connection_);
  const std::uint32_t event_mask = XCB_EVENT_MASK_EXPOSURE |
      XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_FOCUS_CHANGE |
      XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
      XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
      XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
      XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  const std::uint32_t values[] = {screen_->black_pixel, event_mask};
  const auto create = xcb_create_window_checked(
      connection_, XCB_COPY_FROM_PARENT, window_,
      parent == XCB_WINDOW_NONE ? screen_->root : parent,
      descriptor.position ? static_cast<std::int16_t>(descriptor.position->x) : 0,
      descriptor.position ? static_cast<std::int16_t>(descriptor.position->y) : 0,
      static_cast<std::uint16_t>(
          std::max(1.0F, descriptor.size.width * state_.scale.value)),
      static_cast<std::uint16_t>(
          std::max(1.0F, descriptor.size.height * state_.scale.value)),
      0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen_->root_visual,
      XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, values);
  xcb_generic_error_t* error = xcb_request_check(connection_, create);
  if (error != nullptr) {
    const std::uint8_t code = error->error_code;
    std::free(error);
    window_ = XCB_WINDOW_NONE;
    return std::unexpected(x11_error(
        ErrorCode::window_creation_failed,
        "xcb_create_window failed (error " + std::to_string(code) + ")"));
  }
  xcb_change_property(
      connection_, XCB_PROP_MODE_REPLACE, window_, atoms_.wm_protocols,
      XCB_ATOM_ATOM, 32, 1, &atoms_.wm_delete_window);
  state_ = WindowState{
      .framebuffer_size = Size{
          descriptor.size.width * state_.scale.value,
          descriptor.size.height * state_.scale.value},
      .scale = state_.scale,
      .close_requested = false,
  };
  position_ = descriptor.position;
  set_title(descriptor.title);
  apply_window_chrome(descriptor.chrome);
  const auto map = xcb_map_window_checked(connection_, window_);
  error = xcb_request_check(connection_, map);
  if (error != nullptr) {
    std::free(error);
    destroy_native_window();
    return std::unexpected(x11_error(
        ErrorCode::window_creation_failed, "xcb_map_window failed"));
  }
  xcb_flush(connection_);
  configured_ = true;
  callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  return {};
}

X11Window::~X11Window() {
  set_pointer_capture(false);
  if (cursor_ != XCB_CURSOR_NONE) xcb_free_cursor(connection_, cursor_);
  if (unregister_) unregister_(this);
  destroy_native_window();
}

void X11Window::destroy_native_window() {
  if (window_ == XCB_WINDOW_NONE || connection_ == nullptr) return;
  xcb_destroy_window(connection_, window_);
  xcb_flush(connection_);
  window_ = XCB_WINDOW_NONE;
  configured_ = false;
  active_ = false;
  focused_ = false;
}

NativeSurfaceHandle X11Window::native_surface() const {
  return X11SurfaceHandle{.display = connection_, .window = window_};
}

WindowState X11Window::state() const { return state_; }

void X11Window::request_redraw() { callback_(WindowRedrawRequested{}); }

void X11Window::set_title(std::string_view title) {
  if (window_ == XCB_WINDOW_NONE) return;
  xcb_change_property(
      connection_, XCB_PROP_MODE_REPLACE, window_, XCB_ATOM_WM_NAME,
      XCB_ATOM_STRING, 8, static_cast<std::uint32_t>(title.size()), title.data());
  xcb_change_property(
      connection_, XCB_PROP_MODE_REPLACE, window_, atoms_.net_wm_name,
      atoms_.utf8_string, 8, static_cast<std::uint32_t>(title.size()), title.data());
  xcb_flush(connection_);
}

void X11Window::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement> placement) {
  state_.ime_text_input_placement = std::move(placement);
}

void X11Window::wakeup_requested() { callback_(WindowWakeupRequested{}); }

}  // namespace cgpui
