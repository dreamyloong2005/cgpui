#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <expected>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <optional>
#include <poll.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>

struct xdg_positioner;
struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;

namespace {

extern const wl_interface xdg_positioner_interface;
extern const wl_interface xdg_wm_base_interface;
extern const wl_interface xdg_surface_interface;
extern const wl_interface xdg_toplevel_interface;

const wl_interface xdg_positioner_interface{
    "xdg_positioner", 1, 0, nullptr, 0, nullptr};

const wl_message xdg_toplevel_requests[]{
    {"destroy", "", nullptr},
    {"set_parent", "?o", nullptr},
    {"set_title", "s", nullptr},
};
const wl_message xdg_toplevel_events[]{
    {"configure", "iia", nullptr},
    {"close", "", nullptr},
};
const wl_interface xdg_toplevel_interface{
    "xdg_toplevel",
    1,
    3,
    xdg_toplevel_requests,
    2,
    xdg_toplevel_events,
};

const wl_interface* xdg_surface_get_toplevel_types[]{
    &xdg_toplevel_interface,
};
const wl_message xdg_surface_requests[]{
    {"destroy", "", nullptr},
    {"get_toplevel", "n", xdg_surface_get_toplevel_types},
    {"get_popup", "noo", nullptr},
    {"set_window_geometry", "iiii", nullptr},
    {"ack_configure", "u", nullptr},
};
const wl_message xdg_surface_events[]{
    {"configure", "u", nullptr},
};
const wl_interface xdg_surface_interface{
    "xdg_surface",
    1,
    5,
    xdg_surface_requests,
    1,
    xdg_surface_events,
};

const wl_interface* xdg_wm_base_create_positioner_types[]{
    &xdg_positioner_interface,
};
const wl_interface* xdg_wm_base_get_xdg_surface_types[]{
    &xdg_surface_interface,
    &wl_surface_interface,
};
const wl_message xdg_wm_base_requests[]{
    {"destroy", "", nullptr},
    {"create_positioner", "n", xdg_wm_base_create_positioner_types},
    {"get_xdg_surface", "no", xdg_wm_base_get_xdg_surface_types},
    {"pong", "u", nullptr},
};
const wl_message xdg_wm_base_events[]{
    {"ping", "u", nullptr},
};
const wl_interface xdg_wm_base_interface{
    "xdg_wm_base",
    1,
    4,
    xdg_wm_base_requests,
    1,
    xdg_wm_base_events,
};

struct xdg_wm_base_listener {
  void (*ping)(void* data, xdg_wm_base* shell, std::uint32_t serial);
};

struct xdg_surface_listener {
  void (*configure)(void* data, xdg_surface* surface, std::uint32_t serial);
};

struct xdg_toplevel_listener {
  void (*configure)(
      void* data,
      xdg_toplevel* toplevel,
      std::int32_t width,
      std::int32_t height,
      wl_array* states);
  void (*close)(void* data, xdg_toplevel* toplevel);
};

int xdg_wm_base_add_listener(
    xdg_wm_base* shell,
    const xdg_wm_base_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(shell),
      reinterpret_cast<void (**)(void)>(
          const_cast<xdg_wm_base_listener*>(listener)),
      data);
}

void xdg_wm_base_destroy(xdg_wm_base* shell) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(shell),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(shell)),
      WL_MARSHAL_FLAG_DESTROY);
}

xdg_surface* xdg_wm_base_get_xdg_surface(
    xdg_wm_base* shell,
    wl_surface* surface) {
  wl_proxy* proxy = wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(shell),
      2,
      &xdg_surface_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(shell)),
      0,
      nullptr,
      surface);
  return reinterpret_cast<xdg_surface*>(proxy);
}

void xdg_wm_base_pong(xdg_wm_base* shell, std::uint32_t serial) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(shell),
      3,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(shell)),
      0,
      serial);
}

int xdg_surface_add_listener(
    xdg_surface* surface,
    const xdg_surface_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(surface),
      reinterpret_cast<void (**)(void)>(
          const_cast<xdg_surface_listener*>(listener)),
      data);
}

void xdg_surface_destroy(xdg_surface* surface) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(surface),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(surface)),
      WL_MARSHAL_FLAG_DESTROY);
}

xdg_toplevel* xdg_surface_get_toplevel(xdg_surface* surface) {
  wl_proxy* proxy = wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(surface),
      1,
      &xdg_toplevel_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(surface)),
      0,
      nullptr);
  return reinterpret_cast<xdg_toplevel*>(proxy);
}

void xdg_surface_ack_configure(
    xdg_surface* surface,
    std::uint32_t serial) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(surface),
      4,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(surface)),
      0,
      serial);
}

int xdg_toplevel_add_listener(
    xdg_toplevel* toplevel,
    const xdg_toplevel_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(toplevel),
      reinterpret_cast<void (**)(void)>(
          const_cast<xdg_toplevel_listener*>(listener)),
      data);
}

void xdg_toplevel_destroy(xdg_toplevel* toplevel) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)),
      WL_MARSHAL_FLAG_DESTROY);
}

void xdg_toplevel_set_title(
    xdg_toplevel* toplevel,
    const char* title) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel),
      2,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)),
      0,
      title);
}

} // namespace

namespace cgpui {
namespace {

Error wayland_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

constexpr std::uint32_t linux_button_left = 0x110;
constexpr std::uint32_t linux_button_right = 0x111;
constexpr std::uint32_t linux_button_middle = 0x112;

MouseButton mouse_button_from_wayland(std::uint32_t button) {
  switch (button) {
    case linux_button_left:
      return MouseButton::left;
    case linux_button_right:
      return MouseButton::right;
    case linux_button_middle:
      return MouseButton::middle;
    default:
      return MouseButton::other;
  }
}

Point point_from_fixed(wl_fixed_t x, wl_fixed_t y) {
  return Point{
      static_cast<float>(wl_fixed_to_double(x)),
      static_cast<float>(wl_fixed_to_double(y)),
  };
}

KeyboardModifiers modifiers_from_xkb_state(xkb_state* state) {
  if (state == nullptr) {
    return {};
  }

  return KeyboardModifiers{
      .shift = xkb_state_mod_name_is_active(
                   state,
                   XKB_MOD_NAME_SHIFT,
                   XKB_STATE_MODS_EFFECTIVE) == 1,
      .control = xkb_state_mod_name_is_active(
                     state,
                     XKB_MOD_NAME_CTRL,
                     XKB_STATE_MODS_EFFECTIVE) == 1,
      .alt = xkb_state_mod_name_is_active(
                 state,
                 XKB_MOD_NAME_ALT,
                 XKB_STATE_MODS_EFFECTIVE) == 1,
      .super = xkb_state_mod_name_is_active(
                   state,
                   XKB_MOD_NAME_LOGO,
                   XKB_STATE_MODS_EFFECTIVE) == 1,
  };
}

class WaylandTextInput {
 public:
  [[nodiscard]] ImeTextInputSupport support() const {
    return ImeTextInputSupport::unsupported;
  }

  void set_placement(std::optional<ImeTextInputPlacement> placement) {
    placement_ = placement;
  }

  [[nodiscard]] std::optional<ImeTextInputPlacement> placement() const {
    return placement_;
  }

 private:
  std::optional<ImeTextInputPlacement> placement_;
};

class WaylandWindow final : public PlatformWindow {
 public:
  static Result<std::unique_ptr<WaylandWindow>> create(
      wl_display* display,
      wl_compositor* compositor,
      xdg_wm_base* shell,
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) {
    auto window = std::unique_ptr<WaylandWindow>(
        new WaylandWindow(display, std::move(callback), WindowState{
            .framebuffer_size = descriptor.size,
            .scale = DpiScale{1.0F},
            .close_requested = false,
        }));

    auto initialized = window->initialize(compositor, shell, descriptor);
    if (!initialized) {
      return std::unexpected(initialized.error());
    }

    return window;
  }

  ~WaylandWindow() override {
    if (toplevel_ != nullptr) {
      xdg_toplevel_destroy(toplevel_);
    }
    if (xdg_surface_ != nullptr) {
      xdg_surface_destroy(xdg_surface_);
    }
    if (surface_ != nullptr) {
      wl_surface_destroy(surface_);
    }
  }

  [[nodiscard]] NativeSurfaceHandle native_surface() const override {
    return WaylandSurfaceHandle{.display = display_, .surface = surface_};
  }

  [[nodiscard]] wl_surface* surface() const { return surface_; }

  [[nodiscard]] WindowState state() const override { return state_; }

  void request_redraw() override {
    wl_surface_commit(surface_);
    callback_(WindowRedrawRequested{});
  }

  void request_close() override { close_requested(); }

  void set_title(std::string_view title) override {
    const std::string title_string(title);
    xdg_toplevel_set_title(toplevel_, title_string.c_str());
    wl_surface_commit(surface_);
  }

  void set_cursor(CursorShape cursor_shape) override {
    cursor_shape_ = cursor_shape;
  }

  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) override {
    text_input_.set_placement(placement);
    state_.ime_text_input_support = text_input_.support();
    state_.ime_text_input_placement = text_input_.placement();
  }

  [[nodiscard]] CursorShape cursor_shape() const {
    return cursor_shape_;
  }

  [[nodiscard]] bool configured() const { return configured_; }

  void pointer_moved(Point position) {
    callback_(PointerMoved{.position = position});
  }

  void pointer_button(MouseButton button, bool pressed, Point position) {
    callback_(PointerButton{
        .button = button,
        .pressed = pressed,
        .position = position,
    });
  }

  void pointer_scrolled(Point delta, Point position) {
    callback_(PointerScrolled{.delta = delta, .position = position});
  }

  void drag_entered(Point position, DragDropPayload payload) {
    callback_(DragEntered{.position = position, .payload = std::move(payload)});
  }

  void drag_updated(Point position, DragDropPayload payload) {
    callback_(DragUpdated{.position = position, .payload = std::move(payload)});
  }

  void drag_dropped(Point position, DragDropPayload payload) {
    callback_(DragDropped{.position = position, .payload = std::move(payload)});
  }

  void drag_exited(Point position) {
    callback_(DragExited{.position = position, .payload = {}});
  }

  void wakeup_requested() { callback_(WindowWakeupRequested{}); }

  void keyboard_key(
      std::uint32_t key,
      KeyAction action,
      KeyboardModifiers modifiers) {
    callback_(KeyboardKey{
        .key_code = key,
        .action = action,
        .modifiers = modifiers});
  }

  void text_input(std::string text, KeyboardModifiers modifiers) {
    callback_(TextInput{
        .text = std::move(text),
        .modifiers = modifiers});
  }

  void focus_changed(bool focused) {
    callback_(WindowFocused{.focused = focused});
  }

 private:
  WaylandWindow(
      wl_display* display,
      PlatformEventCallback callback,
      WindowState state)
      : display_(display), callback_(std::move(callback)), state_(state) {}

  Result<void> initialize(
      wl_compositor* compositor,
      xdg_wm_base* shell,
      const WindowDescriptor& descriptor) {
    surface_ = wl_compositor_create_surface(compositor);
    if (surface_ == nullptr) {
      return std::unexpected(wayland_error(
          ErrorCode::window_creation_failed,
          "wl_compositor_create_surface failed"));
    }

    xdg_surface_ = xdg_wm_base_get_xdg_surface(shell, surface_);
    if (xdg_surface_ == nullptr) {
      return std::unexpected(wayland_error(
          ErrorCode::window_creation_failed,
          "xdg_surface creation failed"));
    }
    static const xdg_surface_listener surface_listener{
        .configure = &WaylandWindow::handle_surface_configure,
    };
    xdg_surface_add_listener(xdg_surface_, &surface_listener, this);

    toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
    if (toplevel_ == nullptr) {
      return std::unexpected(wayland_error(
          ErrorCode::window_creation_failed,
          "xdg_toplevel creation failed"));
    }
    static const xdg_toplevel_listener toplevel_listener{
        .configure = &WaylandWindow::handle_toplevel_configure,
        .close = &WaylandWindow::handle_toplevel_close,
    };
    xdg_toplevel_add_listener(toplevel_, &toplevel_listener, this);
    set_title(descriptor.title);

    wl_surface_commit(surface_);
    while (!configured_) {
      if (wl_display_dispatch(display_) == -1) {
        return std::unexpected(wayland_error(
            ErrorCode::window_creation_failed,
            "wl_display_dispatch failed while waiting for configure"));
      }
    }

    callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
    return {};
  }

  static void handle_surface_configure(
      void* data,
      xdg_surface* surface,
      std::uint32_t serial) {
    auto* window = static_cast<WaylandWindow*>(data);
    const bool was_configured = window->configured_;
    xdg_surface_ack_configure(surface, serial);
    (void)wl_display_flush(window->display_);
    window->configured_ = true;
    if (was_configured && window->resize_pending_surface_configure_) {
      window->resize_pending_surface_configure_ = false;
      window->callback_(WindowResized{
          .size = window->state_.framebuffer_size,
          .scale = window->state_.scale});
    }
  }

  static void handle_toplevel_configure(
      void* data,
      xdg_toplevel* toplevel,
      std::int32_t width,
      std::int32_t height,
      wl_array* states) {
    (void)toplevel;
    (void)states;
    auto* window = static_cast<WaylandWindow*>(data);
    if (width > 0 && height > 0) {
      window->state_.framebuffer_size =
          Size{static_cast<float>(width), static_cast<float>(height)};
      if (window->configured_) {
        window->resize_pending_surface_configure_ = true;
      }
    }
  }

  static void handle_toplevel_close(void* data, xdg_toplevel* toplevel) {
    (void)toplevel;
    auto* window = static_cast<WaylandWindow*>(data);
    window->close_requested();
  }

  void close_requested() {
    state_.close_requested = true;
    callback_(WindowCloseRequested{});
  }

  wl_display* display_ = nullptr;
  wl_surface* surface_ = nullptr;
  xdg_surface* xdg_surface_ = nullptr;
  xdg_toplevel* toplevel_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
  WaylandTextInput text_input_;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  bool configured_ = false;
  bool resize_pending_surface_configure_ = false;
};

class WaylandDataDevice {
 public:
  using WindowLookup = std::function<WaylandWindow*(wl_surface*)>;

  void set_manager(wl_data_device_manager* manager) {
    manager_ = manager;
  }

  void set_window_lookup(WindowLookup lookup) {
    find_window_ = std::move(lookup);
  }

  void bind_to_seat(wl_seat* seat) {
    if (manager_ == nullptr || seat == nullptr || data_device_ != nullptr) {
      return;
    }

    data_device_ = wl_data_device_manager_get_data_device(manager_, seat);
    if (data_device_ == nullptr) {
      return;
    }

    static const wl_data_device_listener data_device_listener{
        .data_offer = &WaylandDataDevice::handle_data_offer,
        .enter = &WaylandDataDevice::handle_enter,
        .leave = &WaylandDataDevice::handle_leave,
        .motion = &WaylandDataDevice::handle_motion,
        .drop = &WaylandDataDevice::handle_drop,
        .selection = &WaylandDataDevice::handle_selection,
    };
    wl_data_device_add_listener(data_device_, &data_device_listener, this);
  }

  void reset_device() {
    clear_active_offer();
    if (pending_offer_ != nullptr) {
      wl_data_offer_destroy(pending_offer_);
      pending_offer_ = nullptr;
    }
    drag_window_ = nullptr;
    last_drag_position_ = {};
    if (data_device_ != nullptr) {
      wl_data_device_destroy(data_device_);
      data_device_ = nullptr;
    }
  }

 private:
  static void handle_data_offer(
      void* data,
      wl_data_device* data_device,
      wl_data_offer* offer) {
    (void)data_device;
    auto* self = static_cast<WaylandDataDevice*>(data);
    if (self->pending_offer_ != nullptr && self->pending_offer_ != offer) {
      wl_data_offer_destroy(self->pending_offer_);
    }
    self->pending_offer_ = offer;
  }

  static void handle_enter(
      void* data,
      wl_data_device* data_device,
      std::uint32_t serial,
      wl_surface* surface,
      wl_fixed_t x,
      wl_fixed_t y,
      wl_data_offer* offer) {
    (void)data_device;
    (void)serial;
    auto* self = static_cast<WaylandDataDevice*>(data);
    self->drag_window_ = self->find_window_ ? self->find_window_(surface) : nullptr;
    self->last_drag_position_ = point_from_fixed(x, y);
    self->replace_active_offer(offer);
    if (self->drag_window_ != nullptr) {
      self->drag_window_->drag_entered(
          self->last_drag_position_,
          self->payload_from_active_offer());
    }
  }

  static void handle_leave(void* data, wl_data_device* data_device) {
    (void)data_device;
    auto* self = static_cast<WaylandDataDevice*>(data);
    if (self->drag_window_ != nullptr) {
      self->drag_window_->drag_exited(self->last_drag_position_);
    }
    self->drag_window_ = nullptr;
    self->clear_active_offer();
  }

  static void handle_motion(
      void* data,
      wl_data_device* data_device,
      std::uint32_t time,
      wl_fixed_t x,
      wl_fixed_t y) {
    (void)data_device;
    (void)time;
    auto* self = static_cast<WaylandDataDevice*>(data);
    self->last_drag_position_ = point_from_fixed(x, y);
    if (self->drag_window_ != nullptr) {
      self->drag_window_->drag_updated(
          self->last_drag_position_,
          self->payload_from_active_offer());
    }
  }

  static void handle_drop(void* data, wl_data_device* data_device) {
    (void)data_device;
    auto* self = static_cast<WaylandDataDevice*>(data);
    if (self->drag_window_ != nullptr) {
      self->drag_window_->drag_dropped(
          self->last_drag_position_,
          self->payload_from_active_offer());
    }
  }

  static void handle_selection(
      void* data,
      wl_data_device* data_device,
      wl_data_offer* offer) {
    (void)data_device;
    auto* self = static_cast<WaylandDataDevice*>(data);
    if (offer != nullptr) {
      wl_data_offer_destroy(offer);
    }
    if (self->pending_offer_ == offer) {
      self->pending_offer_ = nullptr;
    }
  }

  void replace_active_offer(wl_data_offer* offer) {
    if (active_offer_ != nullptr && active_offer_ != offer) {
      wl_data_offer_destroy(active_offer_);
    }
    active_offer_ = offer;
    if (pending_offer_ == offer) {
      pending_offer_ = nullptr;
    }
  }

  void clear_active_offer() {
    if (active_offer_ != nullptr) {
      wl_data_offer_destroy(active_offer_);
      active_offer_ = nullptr;
    }
  }

  [[nodiscard]] DragDropPayload payload_from_active_offer() const {
    return {};
  }

  wl_data_device_manager* manager_ = nullptr;
  wl_data_device* data_device_ = nullptr;
  wl_data_offer* pending_offer_ = nullptr;
  wl_data_offer* active_offer_ = nullptr;
  WaylandWindow* drag_window_ = nullptr;
  Point last_drag_position_{};
  WindowLookup find_window_;
};

class WaylandApplication final : public PlatformApplication {
 public:
  WaylandApplication() : display_(wl_display_connect(nullptr)) {
    if (display_ == nullptr) {
      return;
    }
    if (pipe2(wakeup_pipe_, O_NONBLOCK | O_CLOEXEC) == -1) {
      initialization_error_ = "pipe2 failed while creating event-loop wakeup pipe";
      return;
    }
    configure_data_device_lookup();

    registry_ = wl_display_get_registry(display_);
    if (registry_ == nullptr) {
      initialization_error_ = "wl_display_get_registry failed";
      return;
    }

    static const wl_registry_listener registry_listener{
        .global = &WaylandApplication::handle_global,
        .global_remove = &WaylandApplication::handle_global_remove,
    };
    wl_registry_add_listener(registry_, &registry_listener, this);
    if (wl_display_roundtrip(display_) == -1) {
      initialization_error_ = "wl_display_roundtrip failed";
      return;
    }

    if (shell_ != nullptr) {
      static const xdg_wm_base_listener shell_listener{
          .ping = &WaylandApplication::handle_shell_ping,
      };
      xdg_wm_base_add_listener(shell_, &shell_listener, this);
    }
    if (seat_ != nullptr) {
      static const wl_seat_listener seat_listener{
          .capabilities = &WaylandApplication::handle_seat_capabilities,
          .name = &WaylandApplication::handle_seat_name,
      };
      wl_seat_add_listener(seat_, &seat_listener, this);
      data_device_.bind_to_seat(seat_);
      if (wl_display_roundtrip(display_) == -1) {
        initialization_error_ = "wl_display_roundtrip failed while waiting for seat";
      }
    }
  }

  ~WaylandApplication() override {
    data_device_.reset_device();
    reset_keyboard_state();
    if (keyboard_ != nullptr) {
      wl_keyboard_destroy(keyboard_);
    }
    if (pointer_ != nullptr) {
      wl_pointer_destroy(pointer_);
    }
    if (seat_ != nullptr) {
      wl_seat_destroy(seat_);
    }
    if (data_device_manager_ != nullptr) {
      wl_data_device_manager_destroy(data_device_manager_);
    }
    if (shell_ != nullptr) {
      xdg_wm_base_destroy(shell_);
    }
    if (compositor_ != nullptr) {
      wl_compositor_destroy(compositor_);
    }
    if (registry_ != nullptr) {
      wl_registry_destroy(registry_);
    }
    if (display_ != nullptr) {
      wl_display_disconnect(display_);
    }
    if (wakeup_pipe_[0] != -1) {
      close(wakeup_pipe_[0]);
    }
    if (wakeup_pipe_[1] != -1) {
      close(wakeup_pipe_[1]);
    }
  }

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    if (display_ == nullptr) {
      return std::unexpected(wayland_error(
          ErrorCode::platform_initialization_failed,
          "wl_display_connect failed"));
    }
    if (!initialization_error_.empty()) {
      return std::unexpected(wayland_error(
          ErrorCode::platform_initialization_failed,
          initialization_error_));
    }
    if (compositor_ == nullptr) {
      return std::unexpected(wayland_error(
          ErrorCode::platform_initialization_failed,
          "wl_compositor global not available"));
    }
    if (shell_ == nullptr) {
      return std::unexpected(wayland_error(
          ErrorCode::platform_initialization_failed,
          "xdg_wm_base global not available"));
    }

    auto window = WaylandWindow::create(
        display_, compositor_, shell_, descriptor, std::move(callback));
    if (!window) {
      return std::unexpected(window.error());
    }

    register_window(window->get());
    return std::unique_ptr<PlatformWindow>(
        new RegisteredWaylandWindow(std::move(*window), *this));
  }

  int run() override {
    while (running_ && display_ != nullptr) {
      const int pending = wl_display_dispatch_pending(display_);
      if (pending == -1) {
        return 1;
      }
      if (pending > 0) {
        continue;
      }
      (void)wl_display_flush(display_);

      std::array<pollfd, 2> fds{
          pollfd{
              .fd = wl_display_get_fd(display_),
              .events = POLLIN,
              .revents = 0,
          },
          pollfd{
              .fd = wakeup_pipe_[0],
              .events = POLLIN,
              .revents = 0,
          },
      };
      const int poll_result = poll(fds.data(), fds.size(), -1);
      if (poll_result == -1) {
        if (errno == EINTR) {
          continue;
        }
        return 1;
      }
      if ((fds[1].revents & POLLIN) != 0) {
        drain_wakeup_pipe();
        dispatch_wakeup();
      }
      if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
        return 1;
      }
      if ((fds[0].revents & POLLIN) != 0 &&
          wl_display_dispatch(display_) == -1) {
        return 1;
      }
    }
    return 0;
  }

  void request_wakeup() override {
    if (wakeup_pipe_[1] == -1) {
      return;
    }
    const std::uint8_t byte = 1;
    const ssize_t written = write(wakeup_pipe_[1], &byte, sizeof(byte));
    (void)written;
  }

  void quit() override {
    running_ = false;
    request_wakeup();
  }

  [[nodiscard]] FontDatabase discover_fonts() const override {
    return {};
  }

 private:
  void configure_data_device_lookup() {
    data_device_.set_window_lookup(
        [this](wl_surface* surface) { return find_window(surface); });
  }

  static void handle_global(
      void* data,
      wl_registry* registry,
      std::uint32_t name,
      const char* interface,
      std::uint32_t version) {
    auto* app = static_cast<WaylandApplication*>(data);
    const std::string_view interface_name(interface);
    if (interface_name == wl_compositor_interface.name) {
      app->compositor_ = static_cast<wl_compositor*>(wl_registry_bind(
          registry,
          name,
          &wl_compositor_interface,
          std::min<std::uint32_t>(version, 4)));
      return;
    }
    if (interface_name == xdg_wm_base_interface.name) {
      app->shell_ = static_cast<xdg_wm_base*>(wl_registry_bind(
          registry,
          name,
          &xdg_wm_base_interface,
          1));
      return;
    }
    if (interface_name == wl_seat_interface.name) {
      app->seat_ = static_cast<wl_seat*>(wl_registry_bind(
          registry,
          name,
          &wl_seat_interface,
          std::min<std::uint32_t>(version, 5)));
      return;
    }
    if (interface_name == wl_data_device_manager_interface.name) {
      app->data_device_manager_ =
          static_cast<wl_data_device_manager*>(wl_registry_bind(
              registry,
              name,
              &wl_data_device_manager_interface,
              std::min<std::uint32_t>(version, 3)));
      app->data_device_.set_manager(app->data_device_manager_);
    }
  }

  static void handle_global_remove(
      void* data,
      wl_registry* registry,
      std::uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
  }

  static void handle_shell_ping(
      void* data,
      xdg_wm_base* shell,
      std::uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(shell, serial);
  }

  static void handle_seat_capabilities(
      void* data,
      wl_seat* seat,
      std::uint32_t capabilities) {
    auto* app = static_cast<WaylandApplication*>(data);
    const bool has_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER) != 0U;
    const bool has_keyboard = (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) != 0U;

    if (has_pointer) {
      if (app->pointer_ == nullptr) {
        app->pointer_ = wl_seat_get_pointer(seat);
        static const wl_pointer_listener pointer_listener{
            .enter = &WaylandApplication::handle_pointer_enter,
            .leave = &WaylandApplication::handle_pointer_leave,
            .motion = &WaylandApplication::handle_pointer_motion,
            .button = &WaylandApplication::handle_pointer_button,
            .axis = &WaylandApplication::handle_pointer_axis,
            .frame = &WaylandApplication::handle_pointer_frame,
            .axis_source = &WaylandApplication::handle_pointer_axis_source,
            .axis_stop = &WaylandApplication::handle_pointer_axis_stop,
            .axis_discrete = &WaylandApplication::handle_pointer_axis_discrete,
            .axis_value120 = &WaylandApplication::handle_pointer_axis_value120,
            .axis_relative_direction =
                &WaylandApplication::handle_pointer_axis_relative_direction,
        };
        wl_pointer_add_listener(app->pointer_, &pointer_listener, app);
      }
    } else if (app->pointer_ != nullptr) {
      wl_pointer_destroy(app->pointer_);
      app->pointer_ = nullptr;
      app->pointer_window_ = nullptr;
      app->pending_scroll_delta_ = {};
      app->pointer_scroll_pending_ = false;
    }

    if (capabilities != 0U) {
      app->data_device_.bind_to_seat(seat);
    } else {
      app->data_device_.reset_device();
    }

    if (has_keyboard) {
      if (app->keyboard_ == nullptr) {
        app->keyboard_ = wl_seat_get_keyboard(seat);
        static const wl_keyboard_listener keyboard_listener{
            .keymap = &WaylandApplication::handle_keyboard_keymap,
            .enter = &WaylandApplication::handle_keyboard_enter,
            .leave = &WaylandApplication::handle_keyboard_leave,
            .key = &WaylandApplication::handle_keyboard_key,
            .modifiers = &WaylandApplication::handle_keyboard_modifiers,
            .repeat_info = &WaylandApplication::handle_keyboard_repeat_info,
        };
        wl_keyboard_add_listener(app->keyboard_, &keyboard_listener, app);
      }
    } else if (app->keyboard_ != nullptr) {
      wl_keyboard_destroy(app->keyboard_);
      app->keyboard_ = nullptr;
      app->keyboard_window_ = nullptr;
      app->reset_keyboard_state();
    }
  }

  static void handle_seat_name(void* data, wl_seat* seat, const char* name) {
    (void)data;
    (void)seat;
    (void)name;
  }

  static void handle_keyboard_keymap(
      void* data,
      wl_keyboard* keyboard,
      std::uint32_t format,
      std::int32_t fd,
      std::uint32_t size) {
    (void)keyboard;
    auto* app = static_cast<WaylandApplication*>(data);
    app->load_keyboard_keymap(format, fd, size);
  }

  static void handle_keyboard_enter(
      void* data,
      wl_keyboard* keyboard,
      std::uint32_t serial,
      wl_surface* surface,
      wl_array* keys) {
    (void)keyboard;
    (void)serial;
    (void)keys;
    auto* app = static_cast<WaylandApplication*>(data);
    app->keyboard_window_ = app->find_window(surface);
    if (app->keyboard_window_ != nullptr) {
      app->keyboard_window_->focus_changed(true);
    }
  }

  static void handle_keyboard_leave(
      void* data,
      wl_keyboard* keyboard,
      std::uint32_t serial,
      wl_surface* surface) {
    (void)keyboard;
    (void)serial;
    auto* app = static_cast<WaylandApplication*>(data);
    if (app->keyboard_window_ != nullptr &&
        app->keyboard_window_->surface() == surface) {
      app->keyboard_window_->focus_changed(false);
      app->keyboard_window_ = nullptr;
    }
  }

  static void handle_keyboard_key(
      void* data,
      wl_keyboard* keyboard,
      std::uint32_t serial,
      std::uint32_t time,
      std::uint32_t key,
      std::uint32_t state) {
    (void)keyboard;
    (void)serial;
    (void)time;
    auto* app = static_cast<WaylandApplication*>(data);
    if (app->keyboard_window_ != nullptr) {
      const auto modifiers = app->keyboard_modifiers();
      const auto action = state == WL_KEYBOARD_KEY_STATE_RELEASED
          ? KeyAction::released
          : KeyAction::pressed;
      app->keyboard_window_->keyboard_key(
          key,
          action,
          modifiers);
      if (action == KeyAction::pressed) {
        auto text = app->text_for_key(key);
        if (!text.empty()) {
          app->keyboard_window_->text_input(std::move(text), modifiers);
        }
      }
    }
  }

  static void handle_keyboard_modifiers(
      void* data,
      wl_keyboard* keyboard,
      std::uint32_t serial,
      std::uint32_t mods_depressed,
      std::uint32_t mods_latched,
      std::uint32_t mods_locked,
      std::uint32_t group) {
    (void)keyboard;
    (void)serial;
    auto* app = static_cast<WaylandApplication*>(data);
    if (app->xkb_state_ != nullptr) {
      xkb_state_update_mask(
          app->xkb_state_,
          mods_depressed,
          mods_latched,
          mods_locked,
          0,
          0,
          group);
    }
  }

  static void handle_keyboard_repeat_info(
      void* data,
      wl_keyboard* keyboard,
      std::int32_t rate,
      std::int32_t delay) {
    (void)data;
    (void)keyboard;
    (void)rate;
    (void)delay;
  }

  static void handle_pointer_enter(
      void* data,
      wl_pointer* pointer,
      std::uint32_t serial,
      wl_surface* surface,
      wl_fixed_t surface_x,
      wl_fixed_t surface_y) {
    (void)pointer;
    auto* app = static_cast<WaylandApplication*>(data);
    app->pointer_enter_serial_ = serial;
    app->pointer_window_ = app->find_window(surface);
    app->pointer_position_ = point_from_fixed(surface_x, surface_y);
    app->apply_cursor_for(app->pointer_window_);
  }

  static void handle_pointer_leave(
      void* data,
      wl_pointer* pointer,
      std::uint32_t serial,
      wl_surface* surface) {
    (void)pointer;
    (void)serial;
    auto* app = static_cast<WaylandApplication*>(data);
    if (app->pointer_window_ != nullptr &&
        app->pointer_window_->surface() == surface) {
      app->pointer_window_ = nullptr;
      app->pending_scroll_delta_ = {};
      app->pointer_scroll_pending_ = false;
    }
  }

  static void handle_pointer_motion(
      void* data,
      wl_pointer* pointer,
      std::uint32_t time,
      wl_fixed_t surface_x,
      wl_fixed_t surface_y) {
    (void)pointer;
    (void)time;
    auto* app = static_cast<WaylandApplication*>(data);
    app->pointer_position_ = point_from_fixed(surface_x, surface_y);
    if (app->pointer_window_ != nullptr) {
      app->pointer_window_->pointer_moved(app->pointer_position_);
    }
  }

  static void handle_pointer_button(
      void* data,
      wl_pointer* pointer,
      std::uint32_t serial,
      std::uint32_t time,
      std::uint32_t button,
      std::uint32_t state) {
    (void)pointer;
    (void)serial;
    (void)time;
    auto* app = static_cast<WaylandApplication*>(data);
    if (app->pointer_window_ != nullptr) {
      app->pointer_window_->pointer_button(
          mouse_button_from_wayland(button),
          state == WL_POINTER_BUTTON_STATE_PRESSED,
          app->pointer_position_);
    }
  }

  static void handle_pointer_axis(
      void* data,
      wl_pointer* pointer,
      std::uint32_t time,
      std::uint32_t axis,
      wl_fixed_t value) {
    auto* app = static_cast<WaylandApplication*>(data);
    (void)time;
    const auto delta = static_cast<float>(wl_fixed_to_double(value));
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
      app->pending_scroll_delta_.y += delta;
    } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
      app->pending_scroll_delta_.x += delta;
    } else {
      return;
    }

    app->pointer_scroll_pending_ = true;
    const auto pointer_version =
        wl_proxy_get_version(reinterpret_cast<wl_proxy*>(pointer));
    if (pointer_version < WL_POINTER_FRAME_SINCE_VERSION) {
      app->dispatch_pointer_scroll();
    }
  }

  static void handle_pointer_frame(void* data, wl_pointer* pointer) {
    (void)pointer;
    auto* app = static_cast<WaylandApplication*>(data);
    app->dispatch_pointer_scroll();
  }

  static void handle_pointer_axis_source(
      void* data,
      wl_pointer* pointer,
      std::uint32_t axis_source) {
    (void)data;
    (void)pointer;
    (void)axis_source;
  }

  static void handle_pointer_axis_stop(
      void* data,
      wl_pointer* pointer,
      std::uint32_t time,
      std::uint32_t axis) {
    (void)data;
    (void)pointer;
    (void)time;
    (void)axis;
  }

  static void handle_pointer_axis_discrete(
      void* data,
      wl_pointer* pointer,
      std::uint32_t axis,
      std::int32_t discrete) {
    (void)data;
    (void)pointer;
    (void)axis;
    (void)discrete;
  }

  static void handle_pointer_axis_value120(
      void* data,
      wl_pointer* pointer,
      std::uint32_t axis,
      std::int32_t value120) {
    (void)data;
    (void)pointer;
    (void)axis;
    (void)value120;
  }

  static void handle_pointer_axis_relative_direction(
      void* data,
      wl_pointer* pointer,
      std::uint32_t axis,
      std::uint32_t direction) {
    (void)data;
    (void)pointer;
    (void)axis;
    (void)direction;
  }

  void register_window(WaylandWindow* window) {
    windows_.push_back(window);
  }

  void unregister_window(WaylandWindow* window) {
    std::erase(windows_, window);
    if (pointer_window_ == window) {
      pointer_window_ = nullptr;
      pending_scroll_delta_ = {};
      pointer_scroll_pending_ = false;
    }
    if (keyboard_window_ == window) {
      keyboard_window_ = nullptr;
    }
  }

  void dispatch_pointer_scroll() {
    if (!pointer_scroll_pending_) {
      return;
    }

    const Point delta = pending_scroll_delta_;
    pending_scroll_delta_ = {};
    pointer_scroll_pending_ = false;
    if (pointer_window_ != nullptr) {
      pointer_window_->pointer_scrolled(delta, pointer_position_);
    }
  }

  void drain_wakeup_pipe() {
    if (wakeup_pipe_[0] == -1) {
      return;
    }
    std::array<std::uint8_t, 64> buffer{};
    while (read(wakeup_pipe_[0], buffer.data(), buffer.size()) > 0) {}
  }

  void dispatch_wakeup() {
    for (WaylandWindow* window : windows_) {
      if (window != nullptr) {
        window->wakeup_requested();
      }
    }
  }

  void set_window_cursor(WaylandWindow& window, CursorShape cursor_shape) {
    window.set_cursor(cursor_shape);
    if (pointer_window_ == &window) {
      apply_cursor_for(&window);
    }
  }

  void apply_cursor_for(WaylandWindow* window) {
    if (window == nullptr || pointer_ == nullptr) {
      return;
    }
    wl_pointer_set_cursor(pointer_, pointer_enter_serial_, nullptr, 0, 0);
    (void)wl_display_flush(display_);
  }

  void load_keyboard_keymap(
      std::uint32_t format,
      std::int32_t fd,
      std::uint32_t size) {
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || size == 0) {
      close(fd);
      return;
    }

    void* mapped = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
      close(fd);
      return;
    }

    auto* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    xkb_keymap* keymap = nullptr;
    xkb_state* state = nullptr;
    if (context != nullptr) {
      keymap = xkb_keymap_new_from_string(
          context,
          static_cast<const char*>(mapped),
          XKB_KEYMAP_FORMAT_TEXT_V1,
          XKB_KEYMAP_COMPILE_NO_FLAGS);
    }
    if (keymap != nullptr) {
      state = xkb_state_new(keymap);
    }

    munmap(mapped, size);
    close(fd);

    if (context == nullptr || keymap == nullptr || state == nullptr) {
      if (state != nullptr) {
        xkb_state_unref(state);
      }
      if (keymap != nullptr) {
        xkb_keymap_unref(keymap);
      }
      if (context != nullptr) {
        xkb_context_unref(context);
      }
      return;
    }

    reset_keyboard_state();
    xkb_context_ = context;
    xkb_keymap_ = keymap;
    xkb_state_ = state;
  }

  void reset_keyboard_state() {
    if (xkb_state_ != nullptr) {
      xkb_state_unref(xkb_state_);
      xkb_state_ = nullptr;
    }
    if (xkb_keymap_ != nullptr) {
      xkb_keymap_unref(xkb_keymap_);
      xkb_keymap_ = nullptr;
    }
    if (xkb_context_ != nullptr) {
      xkb_context_unref(xkb_context_);
      xkb_context_ = nullptr;
    }
  }

  [[nodiscard]] KeyboardModifiers keyboard_modifiers() const {
    return modifiers_from_xkb_state(xkb_state_);
  }

  [[nodiscard]] std::string text_for_key(std::uint32_t key) const {
    if (xkb_state_ == nullptr) {
      return {};
    }

    std::array<char, 64> buffer{};
    const auto length = xkb_state_key_get_utf8(
        xkb_state_,
        key + 8,
        buffer.data(),
        buffer.size());
    if (length <= 0 || static_cast<std::size_t>(length) >= buffer.size()) {
      return {};
    }
    return std::string(buffer.data(), static_cast<std::size_t>(length));
  }

  [[nodiscard]] WaylandWindow* find_window(wl_surface* surface) const {
    for (WaylandWindow* window : windows_) {
      if (window->surface() == surface) {
        return window;
      }
    }
    return nullptr;
  }

  class RegisteredWaylandWindow final : public PlatformWindow {
   public:
    RegisteredWaylandWindow(
        std::unique_ptr<WaylandWindow> window,
        WaylandApplication& app)
        : window_(std::move(window)), app_(app) {}

    ~RegisteredWaylandWindow() override { app_.unregister_window(window_.get()); }

    [[nodiscard]] NativeSurfaceHandle native_surface() const override {
      return window_->native_surface();
    }

    [[nodiscard]] WindowState state() const override {
      return window_->state();
    }

    void request_redraw() override { window_->request_redraw(); }

    void request_close() override { window_->request_close(); }

    void set_title(std::string_view title) override {
      window_->set_title(title);
    }

    void set_cursor(CursorShape cursor_shape) override {
      app_.set_window_cursor(*window_, cursor_shape);
    }

    void set_ime_text_input_placement(
        std::optional<ImeTextInputPlacement> placement) override {
      window_->set_ime_text_input_placement(placement);
    }

   private:
    std::unique_ptr<WaylandWindow> window_;
    WaylandApplication& app_;
  };

  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  wl_compositor* compositor_ = nullptr;
  xdg_wm_base* shell_ = nullptr;
  wl_seat* seat_ = nullptr;
  wl_data_device_manager* data_device_manager_ = nullptr;
  wl_pointer* pointer_ = nullptr;
  wl_keyboard* keyboard_ = nullptr;
  WaylandDataDevice data_device_;
  xkb_context* xkb_context_ = nullptr;
  xkb_keymap* xkb_keymap_ = nullptr;
  xkb_state* xkb_state_ = nullptr;
  std::vector<WaylandWindow*> windows_;
  WaylandWindow* pointer_window_ = nullptr;
  WaylandWindow* keyboard_window_ = nullptr;
  int wakeup_pipe_[2] = {-1, -1};
  Point pointer_position_{};
  Point pending_scroll_delta_{};
  std::string initialization_error_;
  std::uint32_t pointer_enter_serial_ = 0;
  bool pointer_scroll_pending_ = false;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<WaylandApplication>();
}

} // namespace cgpui
