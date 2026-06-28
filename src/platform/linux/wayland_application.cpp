#include "cgpui/platform/platform.hpp"

#include <wayland-client.h>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

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

  [[nodiscard]] bool configured() const { return configured_; }

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
  bool configured_ = false;
  bool resize_pending_surface_configure_ = false;
};

class WaylandApplication final : public PlatformApplication {
 public:
  WaylandApplication() : display_(wl_display_connect(nullptr)) {
    if (display_ == nullptr) {
      return;
    }

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
  }

  ~WaylandApplication() override {
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

    return std::unique_ptr<PlatformWindow>(std::move(*window));
  }

  int run() override {
    while (running_ && display_ != nullptr) {
      if (wl_display_dispatch(display_) == -1) {
        return 1;
      }
    }
    return 0;
  }

  void quit() override { running_ = false; }

 private:
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

  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  wl_compositor* compositor_ = nullptr;
  xdg_wm_base* shell_ = nullptr;
  std::string initialization_error_;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<WaylandApplication>();
}

} // namespace cgpui
