#include "wayland_test_compositor.hpp"

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
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
constexpr std::uint32_t xdg_toplevel_configure = 0;
constexpr std::uint32_t xdg_toplevel_close = 1;

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

} // namespace

namespace cgpui::test {

struct WaylandTestCompositor::State {
  struct SurfaceState;

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

  struct PointerButtonRequest {
    std::uint32_t button = 0;
    bool pressed = false;
  };

  explicit State(std::string test_name) {
    runtime_dir = std::filesystem::temp_directory_path() /
        ("cgpui-wayland-" + std::move(test_name) + "-" + std::to_string(::getpid()));
    std::filesystem::create_directories(runtime_dir);
    std::filesystem::permissions(
        runtime_dir,
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::replace);
  }

  ~State() {
    stop();
    if (compositor_global != nullptr) {
      wl_global_destroy(compositor_global);
    }
    if (shell_global != nullptr) {
      wl_global_destroy(shell_global);
    }
    if (seat_global != nullptr) {
      wl_global_destroy(seat_global);
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
        &State::bind_compositor);
    shell_global = wl_global_create(
        display,
        &xdg_wm_base_interface,
        1,
        this,
        &State::bind_shell);
    seat_global = wl_global_create(
        display,
        &wl_seat_interface,
        5,
        this,
        &State::bind_seat);
    if (compositor_global == nullptr || shell_global == nullptr ||
        seat_global == nullptr) {
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

  [[nodiscard]] bool wait_for_flag(const std::atomic_bool& flag) const {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline) {
      if (flag.load()) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return flag.load();
  }

  void request_resize_configure(std::int32_t width, std::int32_t height) {
    resize_width.store(width);
    resize_height.store(height);
    resize_configure_pending.store(true);
  }

  void request_pointer_move(std::int32_t x, std::int32_t y) {
    pointer_x.store(x);
    pointer_y.store(y);
    pointer_move_pending.store(true);
  }

  void request_pointer_button(std::uint32_t button, bool pressed) {
    {
      std::lock_guard lock(pointer_button_mutex);
      pointer_buttons.push_back(PointerButtonRequest{
          .button = button,
          .pressed = pressed,
      });
    }
    pointer_button_pending.store(true);
  }

  void request_close() {
    close_pending.store(true);
  }

  static void bind_compositor(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<State*>(data);
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
    auto* compositor = static_cast<State*>(data);
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

  static void bind_seat(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<State*>(data);
    auto* resource = wl_resource_create(
        client,
        &wl_seat_interface,
        std::min<std::uint32_t>(version, 5),
        id);
    compositor->seat_resource = resource;
    wl_resource_set_implementation(
        resource,
        &seat_implementation,
        compositor,
        &State::handle_seat_destroyed);
    wl_seat_send_capabilities(resource, WL_SEAT_CAPABILITY_POINTER);
    if (wl_resource_get_version(resource) >= WL_SEAT_NAME_SINCE_VERSION) {
      wl_seat_send_name(resource, "test-seat");
    }
  }

  static void create_surface(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id);
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
      wl_resource* surface_resource);
  static void get_toplevel(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id);
  static void ack_configure(
      wl_client*,
      wl_resource* resource,
      std::uint32_t serial);
  static void surface_commit(wl_client*, wl_resource* resource);
  static void shell_get_positioner(
      wl_client* client,
      wl_resource*,
      std::uint32_t id) {
    auto* resource = wl_resource_create(client, &xdg_positioner_interface, 1, id);
    wl_resource_set_implementation(resource, nullptr, nullptr, nullptr);
  }
  static void seat_get_pointer(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id);
  static void handle_seat_destroyed(wl_resource* resource);
  static void pointer_set_cursor(
      wl_client*,
      wl_resource*,
      std::uint32_t,
      wl_resource*,
      std::int32_t,
      std::int32_t) {}
  static void handle_pointer_destroyed(wl_resource* resource);

  [[nodiscard]] SurfaceState* find_surface(wl_resource* resource) const;
  [[nodiscard]] SurfaceState* first_pointer_surface() const;

  void dispatch_pending_close();
  void dispatch_pending_resize_configure();
  void dispatch_pending_pointer_move();
  void dispatch_pending_pointer_button();

  void run() {
    while (running.load()) {
      dispatch_pending_close();
      dispatch_pending_resize_configure();
      dispatch_pending_pointer_move();
      dispatch_pending_pointer_button();
      const int result = wl_event_loop_dispatch(wl_display_get_event_loop(display), 10);
      if (result < 0) {
        running.store(false);
        break;
      }
      dispatch_pending_close();
      dispatch_pending_resize_configure();
      dispatch_pending_pointer_move();
      dispatch_pending_pointer_button();
      wl_display_flush_clients(display);
    }
  }

  static const struct wl_compositor_interface compositor_implementation;
  static const struct wl_surface_interface surface_implementation;
  static const struct wl_seat_interface seat_implementation;
  static const struct wl_pointer_interface pointer_implementation;
  static const XdgWmBaseImplementation shell_implementation;
  static const XdgSurfaceImplementation xdg_surface_implementation;
  static const XdgToplevelImplementation toplevel_implementation;

  wl_display* display = nullptr;
  wl_global* compositor_global = nullptr;
  wl_global* shell_global = nullptr;
  wl_global* seat_global = nullptr;
  wl_resource* seat_resource = nullptr;
  wl_resource* pointer_resource = nullptr;
  std::filesystem::path runtime_dir;
  std::string socket_name;
  std::vector<std::unique_ptr<SurfaceState>> surfaces;
  std::thread server_thread;
  std::atomic_bool running{false};
  std::atomic_bool close_on_initial_configure_ack{false};
  std::atomic_bool close_pending{false};
  std::atomic_bool close_sent{false};
  std::atomic_bool initial_configure_acked{false};
  std::atomic_bool resize_configure_pending{false};
  std::atomic_bool resize_configure_sent{false};
  std::atomic_bool resize_configure_acked{false};
  std::atomic_bool pointer_move_pending{false};
  std::atomic_bool pointer_move_sent{false};
  std::atomic_bool pointer_button_pending{false};
  std::atomic_bool pointer_button_sent{false};
  std::atomic_int resize_width{0};
  std::atomic_int resize_height{0};
  std::atomic_int pointer_x{0};
  std::atomic_int pointer_y{0};
  std::mutex pointer_button_mutex;
  std::deque<PointerButtonRequest> pointer_buttons;
  std::uint32_t next_configure_serial = 1;
  std::uint32_t resize_configure_serial = 0;
  std::uint32_t next_pointer_serial = 1;
  std::uint32_t pointer_time = 1;
  bool pointer_entered = false;
};

struct WaylandTestCompositor::State::SurfaceState {
  WaylandTestCompositor::State* compositor = nullptr;
  wl_resource* surface = nullptr;
  wl_resource* xdg_surface = nullptr;
  wl_resource* toplevel = nullptr;
  std::uint32_t initial_configure_serial = 0;
  bool initial_configure_sent = false;
};

void WaylandTestCompositor::State::create_surface(
    wl_client* client,
    wl_resource* resource,
    std::uint32_t id) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
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
      &surface_implementation,
      state,
      nullptr);
}

void WaylandTestCompositor::State::get_xdg_surface(
    wl_client* client,
    wl_resource* resource,
    std::uint32_t id,
    wl_resource* surface_resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
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

void WaylandTestCompositor::State::get_toplevel(
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

void WaylandTestCompositor::State::ack_configure(
    wl_client*,
    wl_resource* resource,
    std::uint32_t serial) {
  auto* state = static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
  if (state == nullptr) {
    return;
  }

  if (serial == state->initial_configure_serial) {
    state->compositor->initial_configure_acked.store(true);
    if (state->toplevel != nullptr &&
        state->compositor->close_on_initial_configure_ack.load() &&
        !state->compositor->close_sent.load()) {
      state->compositor->close_sent.store(true);
      wl_resource_post_event(state->toplevel, xdg_toplevel_close);
      wl_display_flush_clients(state->compositor->display);
    }
  }

  if (serial == state->compositor->resize_configure_serial) {
    state->compositor->resize_configure_acked.store(true);
  }
}

void WaylandTestCompositor::State::surface_commit(
    wl_client*,
    wl_resource* resource) {
  auto* state = static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
  if (state == nullptr || state->xdg_surface == nullptr ||
      state->initial_configure_sent) {
    return;
  }

  state->initial_configure_sent = true;
  state->initial_configure_serial = state->compositor->next_configure_serial++;
  wl_resource_post_event(
      state->xdg_surface,
      xdg_surface_configure,
      state->initial_configure_serial);
  wl_display_flush_clients(state->compositor->display);
}

WaylandTestCompositor::State::SurfaceState* WaylandTestCompositor::State::find_surface(
    wl_resource* resource) const {
  for (const auto& surface : surfaces) {
    if (surface->surface == resource) {
      return surface.get();
    }
  }
  return nullptr;
}

WaylandTestCompositor::State::SurfaceState*
WaylandTestCompositor::State::first_pointer_surface() const {
  for (const auto& surface : surfaces) {
    if (surface->surface != nullptr && surface->xdg_surface != nullptr &&
        surface->toplevel != nullptr) {
      return surface.get();
    }
  }
  return nullptr;
}

void WaylandTestCompositor::State::seat_get_pointer(
    wl_client* client,
    wl_resource* resource,
    std::uint32_t id) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  auto* pointer = wl_resource_create(
      client,
      &wl_pointer_interface,
      std::min<std::uint32_t>(wl_resource_get_version(resource), 5),
      id);
  compositor->pointer_resource = pointer;
  wl_resource_set_implementation(
      pointer,
      &pointer_implementation,
      compositor,
      &WaylandTestCompositor::State::handle_pointer_destroyed);
}

void WaylandTestCompositor::State::handle_seat_destroyed(wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->seat_resource == resource) {
    compositor->seat_resource = nullptr;
  }
}

void WaylandTestCompositor::State::handle_pointer_destroyed(wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->pointer_resource == resource) {
    compositor->pointer_resource = nullptr;
    compositor->pointer_entered = false;
  }
}

void WaylandTestCompositor::State::dispatch_pending_close() {
  if (!close_pending.exchange(false)) {
    return;
  }

  for (const auto& surface : surfaces) {
    if (surface->toplevel == nullptr) {
      continue;
    }

    close_sent.store(true);
    wl_resource_post_event(surface->toplevel, xdg_toplevel_close);
    wl_display_flush_clients(display);
    return;
  }

  close_pending.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_resize_configure() {
  if (!resize_configure_pending.exchange(false)) {
    return;
  }

  for (const auto& surface : surfaces) {
    if (surface->xdg_surface == nullptr || surface->toplevel == nullptr) {
      continue;
    }

    wl_array states{};
    wl_array_init(&states);
    resize_configure_serial = next_configure_serial++;
    resize_configure_sent.store(true);
    wl_resource_post_event(
        surface->toplevel,
        xdg_toplevel_configure,
        static_cast<std::int32_t>(resize_width.load()),
        static_cast<std::int32_t>(resize_height.load()),
        &states);
    wl_resource_post_event(
        surface->xdg_surface,
        xdg_surface_configure,
        resize_configure_serial);
    wl_array_release(&states);
    wl_display_flush_clients(display);
    return;
  }

  resize_configure_pending.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_pointer_move() {
  if (!pointer_move_pending.exchange(false)) {
    return;
  }

  if (pointer_resource == nullptr) {
    pointer_move_pending.store(true);
    return;
  }

  const SurfaceState* surface = first_pointer_surface();
  if (surface == nullptr) {
    pointer_move_pending.store(true);
    return;
  }

  const std::int32_t x = pointer_x.load();
  const std::int32_t y = pointer_y.load();
  if (!pointer_entered) {
    wl_pointer_send_enter(
        pointer_resource,
        next_pointer_serial++,
        surface->surface,
        wl_fixed_from_int(x),
        wl_fixed_from_int(y));
    pointer_entered = true;
  }
  wl_pointer_send_motion(
      pointer_resource,
      pointer_time++,
      wl_fixed_from_int(x),
      wl_fixed_from_int(y));
  wl_display_flush_clients(display);
  pointer_move_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_pointer_button() {
  if (!pointer_button_pending.exchange(false)) {
    return;
  }

  PointerButtonRequest request{};
  {
    std::lock_guard lock(pointer_button_mutex);
    if (pointer_buttons.empty()) {
      return;
    }
    request = pointer_buttons.front();
    pointer_buttons.pop_front();
    if (!pointer_buttons.empty()) {
      pointer_button_pending.store(true);
    }
  }

  if (pointer_resource == nullptr || first_pointer_surface() == nullptr) {
    {
      std::lock_guard lock(pointer_button_mutex);
      pointer_buttons.push_front(request);
    }
    pointer_button_pending.store(true);
    return;
  }

  wl_pointer_send_button(
      pointer_resource,
      next_pointer_serial++,
      pointer_time++,
      request.button,
      request.pressed ? WL_POINTER_BUTTON_STATE_PRESSED
                      : WL_POINTER_BUTTON_STATE_RELEASED);
  wl_display_flush_clients(display);
  pointer_button_sent.store(true);
}

const struct wl_compositor_interface WaylandTestCompositor::State::compositor_implementation{
    .create_surface = &WaylandTestCompositor::State::create_surface,
    .create_region = &WaylandTestCompositor::State::create_region,
    .release = noop_resource,
};

const struct wl_surface_interface WaylandTestCompositor::State::surface_implementation{
    .destroy = destroy_resource,
    .attach = noop_surface_attach,
    .damage = noop_surface_damage,
    .frame = noop_surface_frame,
    .set_opaque_region = noop_surface_region,
    .set_input_region = noop_surface_region,
    .commit = &WaylandTestCompositor::State::surface_commit,
    .set_buffer_transform = noop_surface_transform,
    .set_buffer_scale = noop_surface_scale,
    .damage_buffer = noop_surface_damage,
    .offset = noop_surface_offset,
    .get_release = noop_surface_release,
};

const struct wl_seat_interface WaylandTestCompositor::State::seat_implementation{
    .get_pointer = &WaylandTestCompositor::State::seat_get_pointer,
    .get_keyboard = [](wl_client*, wl_resource*, std::uint32_t) {},
    .get_touch = [](wl_client*, wl_resource*, std::uint32_t) {},
    .release = noop_resource,
};

const struct wl_pointer_interface WaylandTestCompositor::State::pointer_implementation{
    .set_cursor = &WaylandTestCompositor::State::pointer_set_cursor,
    .release = noop_resource,
};

const WaylandTestCompositor::State::XdgWmBaseImplementation
    WaylandTestCompositor::State::shell_implementation{
        .destroy = destroy_resource,
        .create_positioner = &WaylandTestCompositor::State::shell_get_positioner,
        .get_xdg_surface = &WaylandTestCompositor::State::get_xdg_surface,
        .pong = [](wl_client*, wl_resource*, std::uint32_t) {},
};

const WaylandTestCompositor::State::XdgSurfaceImplementation
    WaylandTestCompositor::State::xdg_surface_implementation{
        .destroy = destroy_resource,
        .get_toplevel = &WaylandTestCompositor::State::get_toplevel,
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
        .ack_configure = &WaylandTestCompositor::State::ack_configure,
};

const WaylandTestCompositor::State::XdgToplevelImplementation
    WaylandTestCompositor::State::toplevel_implementation{
        .destroy = destroy_resource,
        .set_parent = [](wl_client*, wl_resource*, wl_resource*) {},
        .set_title = [](wl_client*, wl_resource*, const char*) {},
};

WaylandTestCompositor::WaylandTestCompositor(std::string name)
    : state_(std::make_unique<State>(std::move(name))) {}

WaylandTestCompositor::~WaylandTestCompositor() = default;

bool WaylandTestCompositor::start() {
  return state_->start();
}

void WaylandTestCompositor::stop() {
  state_->stop();
}

const std::string& WaylandTestCompositor::socket_name() const {
  return state_->socket_name;
}

void WaylandTestCompositor::set_close_on_initial_configure_ack(bool enabled) {
  state_->close_on_initial_configure_ack.store(enabled);
}

void WaylandTestCompositor::request_close() {
  state_->request_close();
}

void WaylandTestCompositor::request_resize_configure(
    std::int32_t width,
    std::int32_t height) {
  state_->request_resize_configure(width, height);
}

void WaylandTestCompositor::request_pointer_move(std::int32_t x, std::int32_t y) {
  state_->request_pointer_move(x, y);
}

void WaylandTestCompositor::request_pointer_button(
    std::uint32_t button,
    bool pressed) {
  state_->request_pointer_button(button, pressed);
}

bool WaylandTestCompositor::wait_for_close_sent() const {
  return state_->wait_for_flag(state_->close_sent);
}

bool WaylandTestCompositor::wait_for_resize_configure_sent() const {
  return state_->wait_for_flag(state_->resize_configure_sent);
}

bool WaylandTestCompositor::wait_for_resize_configure_acked() const {
  return state_->wait_for_flag(state_->resize_configure_acked);
}

bool WaylandTestCompositor::wait_for_pointer_move_sent() const {
  return state_->wait_for_flag(state_->pointer_move_sent);
}

bool WaylandTestCompositor::wait_for_pointer_button_sent() const {
  return state_->wait_for_flag(state_->pointer_button_sent);
}

} // namespace cgpui::test
