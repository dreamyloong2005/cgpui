#include "cgpui/platform/platform.hpp"

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

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

constexpr std::uint32_t xdg_surface_configure = 0;
constexpr std::uint32_t xdg_toplevel_close = 1;

struct TestCompositor;

struct SurfaceState {
  TestCompositor* compositor = nullptr;
  wl_resource* surface = nullptr;
  wl_resource* xdg_surface = nullptr;
  wl_resource* toplevel = nullptr;
  bool configured = false;
  bool close_sent = false;
};

void destroy_resource(wl_client*, wl_resource* resource) {
  wl_resource_destroy(resource);
}

void noop_resource(wl_client*, wl_resource*) {}

void noop_surface_attach(
    wl_client*,
    wl_resource*,
    wl_resource*,
    std::int32_t,
    std::int32_t) {}

void noop_surface_damage(
    wl_client*,
    wl_resource*,
    std::int32_t,
    std::int32_t,
    std::int32_t,
    std::int32_t) {}

void noop_surface_frame(wl_client* client, wl_resource*, std::uint32_t id) {
  auto* callback = wl_resource_create(client, &wl_callback_interface, 1, id);
  if (callback != nullptr) {
    wl_callback_send_done(callback, 0);
    wl_resource_destroy(callback);
  }
}

void noop_surface_region(wl_client*, wl_resource*, wl_resource*) {}

void noop_surface_transform(wl_client*, wl_resource*, std::int32_t) {}

void noop_surface_scale(wl_client*, wl_resource*, std::int32_t) {}

void noop_surface_offset(wl_client*, wl_resource*, std::int32_t, std::int32_t) {}

void noop_surface_release(wl_client*, wl_resource*, std::uint32_t) {}

const struct wl_region_interface region_implementation{
    .destroy = destroy_resource,
    .add = [](
        wl_client*,
        wl_resource*,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t) {},
    .subtract = [](
        wl_client*,
        wl_resource*,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t) {},
};

struct TestCompositor {
  TestCompositor() {
    runtime_dir = std::filesystem::temp_directory_path() /
        ("cgpui-wayland-close-" + std::to_string(::getpid()));
    std::filesystem::create_directories(runtime_dir);
    std::filesystem::permissions(
        runtime_dir,
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::replace);
  }

  ~TestCompositor() {
    stop();
    if (compositor_global != nullptr) {
      wl_global_destroy(compositor_global);
    }
    if (shell_global != nullptr) {
      wl_global_destroy(shell_global);
    }
    if (display != nullptr) {
      wl_display_destroy(display);
    }
    std::filesystem::remove_all(runtime_dir);
  }

  bool start() {
    display = wl_display_create();
    if (display == nullptr) {
      return false;
    }

    const auto runtime_string = runtime_dir.string();
    setenv("XDG_RUNTIME_DIR", runtime_string.c_str(), 1);

    const char* socket = wl_display_add_socket_auto(display);
    if (socket == nullptr) {
      return false;
    }
    socket_name = socket;

    compositor_global = wl_global_create(
        display,
        &wl_compositor_interface,
        4,
        this,
        &bind_compositor);
    shell_global = wl_global_create(
        display,
        &xdg_wm_base_interface,
        1,
        this,
        &bind_shell);
    if (compositor_global == nullptr || shell_global == nullptr) {
      return false;
    }

    running.store(true);
    server_thread = std::thread([this] { run(); });
    return true;
  }

  void stop() {
    if (running.exchange(false) && display != nullptr) {
      wl_display_terminate(display);
    }
    if (server_thread.joinable()) {
      server_thread.join();
    }
  }

  [[nodiscard]] bool wait_for_close_sent() const {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline) {
      if (close_sent.load()) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return close_sent.load();
  }

  static void bind_compositor(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<TestCompositor*>(data);
    auto* resource = wl_resource_create(
        client,
        &wl_compositor_interface,
        std::min<std::uint32_t>(version, 4),
        id);
    wl_resource_set_implementation(
        resource,
        &compositor_implementation,
        compositor,
        nullptr);
  }

  static void bind_shell(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<TestCompositor*>(data);
    auto* resource = wl_resource_create(
        client,
        &xdg_wm_base_interface,
        std::min<std::uint32_t>(version, 1),
        id);
    wl_resource_set_implementation(
        resource,
        &shell_implementation,
        compositor,
        nullptr);
  }

  static void create_surface(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id) {
    auto* compositor =
        static_cast<TestCompositor*>(wl_resource_get_user_data(resource));
    auto surface = std::make_unique<SurfaceState>();
    surface->compositor = compositor;
    wl_resource* surface_resource = wl_resource_create(
        client,
        &wl_surface_interface,
        4,
        id);
    surface->surface = surface_resource;
    SurfaceState* state = surface.get();
    compositor->surfaces.push_back(std::move(surface));
    wl_resource_set_implementation(
        surface_resource,
        &surface_implementation_with_commit,
        state,
        nullptr);
  }

  static void create_region(
      wl_client* client,
      wl_resource*,
      std::uint32_t id) {
    auto* resource = wl_resource_create(client, &wl_region_interface, 1, id);
    wl_resource_set_implementation(
        resource,
        &region_implementation,
        nullptr,
        nullptr);
  }

  static void get_xdg_surface(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id,
      wl_resource* surface_resource) {
    auto* compositor =
        static_cast<TestCompositor*>(wl_resource_get_user_data(resource));
    SurfaceState* state = compositor->find_surface(surface_resource);
    wl_resource* xdg_surface_resource = wl_resource_create(
        client,
        &xdg_surface_interface,
        1,
        id);
    if (state != nullptr) {
      state->xdg_surface = xdg_surface_resource;
    }
    wl_resource_set_implementation(
        xdg_surface_resource,
        &xdg_surface_implementation,
        state,
        nullptr);
  }

  static void get_toplevel(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id) {
    auto* state = static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
    wl_resource* toplevel_resource = wl_resource_create(
        client,
        &xdg_toplevel_interface,
        1,
        id);
    if (state != nullptr) {
      state->toplevel = toplevel_resource;
    }
    wl_resource_set_implementation(
        toplevel_resource,
        &toplevel_implementation,
        state,
        nullptr);
  }

  static void ack_configure(
      wl_client*,
      wl_resource* resource,
      std::uint32_t) {
    auto* state = static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
    if (state != nullptr && state->toplevel != nullptr && !state->close_sent) {
      state->close_sent = true;
      state->compositor->close_sent.store(true);
      wl_resource_post_event(state->toplevel, xdg_toplevel_close);
      wl_display_flush_clients(state->compositor->display);
    }
  }

  static void surface_commit(wl_client*, wl_resource* resource) {
    auto* state = static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
    if (state == nullptr || state->xdg_surface == nullptr || state->configured) {
      return;
    }

    state->configured = true;
    wl_resource_post_event(state->xdg_surface, xdg_surface_configure, 1U);
    wl_display_flush_clients(state->compositor->display);
  }

  static void shell_get_positioner(
      wl_client* client,
      wl_resource*,
      std::uint32_t id) {
    auto* resource = wl_resource_create(client, &xdg_positioner_interface, 1, id);
    wl_resource_set_implementation(resource, nullptr, nullptr, nullptr);
  }

  [[nodiscard]] SurfaceState* find_surface(wl_resource* resource) const {
    for (const auto& surface : surfaces) {
      if (surface->surface == resource) {
        return surface.get();
      }
    }
    return nullptr;
  }

  void run() {
    while (running.load()) {
      const int result = wl_event_loop_dispatch(wl_display_get_event_loop(display), 10);
      if (result < 0) {
        running.store(false);
        break;
      }
      wl_display_flush_clients(display);
    }
  }

  struct XdgWmBaseImplementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*create_positioner)(wl_client*, wl_resource*, std::uint32_t);
    void (*get_xdg_surface)(wl_client*, wl_resource*, std::uint32_t, wl_resource*);
    void (*pong)(wl_client*, wl_resource*, std::uint32_t);
  };

  struct XdgSurfaceImplementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*get_toplevel)(wl_client*, wl_resource*, std::uint32_t);
    void (*get_popup)(
        wl_client*,
        wl_resource*,
        std::uint32_t,
        wl_resource*,
        wl_resource*);
    void (*set_window_geometry)(
        wl_client*,
        wl_resource*,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t);
    void (*ack_configure)(wl_client*, wl_resource*, std::uint32_t);
  };

  struct XdgToplevelImplementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*set_parent)(wl_client*, wl_resource*, wl_resource*);
    void (*set_title)(wl_client*, wl_resource*, const char*);
  };

  static const struct wl_compositor_interface compositor_implementation;
  static const struct wl_surface_interface surface_implementation_with_commit;
  static const XdgWmBaseImplementation shell_implementation;
  static const XdgSurfaceImplementation xdg_surface_implementation;
  static const XdgToplevelImplementation toplevel_implementation;

  wl_display* display = nullptr;
  wl_global* compositor_global = nullptr;
  wl_global* shell_global = nullptr;
  std::filesystem::path runtime_dir;
  std::string socket_name;
  std::vector<std::unique_ptr<SurfaceState>> surfaces;
  std::thread server_thread;
  std::atomic_bool running{false};
  std::atomic_bool close_sent{false};
};

const struct wl_compositor_interface TestCompositor::compositor_implementation{
    .create_surface = &TestCompositor::create_surface,
    .create_region = &TestCompositor::create_region,
    .release = noop_resource,
};

const struct wl_surface_interface TestCompositor::surface_implementation_with_commit{
    .destroy = destroy_resource,
    .attach = noop_surface_attach,
    .damage = noop_surface_damage,
    .frame = noop_surface_frame,
    .set_opaque_region = noop_surface_region,
    .set_input_region = noop_surface_region,
    .commit = &TestCompositor::surface_commit,
    .set_buffer_transform = noop_surface_transform,
    .set_buffer_scale = noop_surface_scale,
    .damage_buffer = noop_surface_damage,
    .offset = noop_surface_offset,
    .get_release = noop_surface_release,
};

const TestCompositor::XdgWmBaseImplementation TestCompositor::shell_implementation{
    .destroy = destroy_resource,
    .create_positioner = &TestCompositor::shell_get_positioner,
    .get_xdg_surface = &TestCompositor::get_xdg_surface,
    .pong = [](wl_client*, wl_resource*, std::uint32_t) {},
};

const TestCompositor::XdgSurfaceImplementation TestCompositor::xdg_surface_implementation{
    .destroy = destroy_resource,
    .get_toplevel = &TestCompositor::get_toplevel,
    .get_popup = [](
        wl_client*,
        wl_resource*,
        std::uint32_t,
        wl_resource*,
        wl_resource*) {},
    .set_window_geometry = [](
        wl_client*,
        wl_resource*,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t) {},
    .ack_configure = &TestCompositor::ack_configure,
};

const TestCompositor::XdgToplevelImplementation TestCompositor::toplevel_implementation{
    .destroy = destroy_resource,
    .set_parent = [](wl_client*, wl_resource*, wl_resource*) {},
    .set_title = [](wl_client*, wl_resource*, const char*) {},
};

} // namespace

int main() {
  TestCompositor compositor;
  if (!compositor.start()) {
    return 2;
  }

  setenv("WAYLAND_DISPLAY", compositor.socket_name.c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 3;
  }

  bool close_requested = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Close Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          close_requested = true;
          (*app)->quit();
        }
      });
  if (!window) {
    return 4;
  }

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (!run_finished.load()) {
    (*app)->quit();
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return 9;
  }

  if (client_thread.joinable()) {
    client_thread.join();
  }
  compositor.stop();

  if (run_result != 0) {
    return 5;
  }
  if (!compositor.wait_for_close_sent()) {
    return 6;
  }
  if (!close_requested) {
    return 7;
  }
  if (!(*window)->state().close_requested) {
    return 8;
  }

  return 0;
}
