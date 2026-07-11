#include "wayland_test_compositor.hpp"

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fcntl.h>
#include <poll.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <unistd.h>

struct xdg_positioner;
struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;
struct zxdg_decoration_manager_v1;
struct zxdg_toplevel_decoration_v1;
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;

namespace {

extern const wl_interface xdg_positioner_interface;
extern const wl_interface xdg_wm_base_interface;
extern const wl_interface xdg_surface_interface;
extern const wl_interface xdg_toplevel_interface;
extern const wl_interface zxdg_decoration_manager_v1_interface;
extern const wl_interface zxdg_toplevel_decoration_v1_interface;
extern const wl_interface zwp_text_input_manager_v3_interface;
extern const wl_interface zwp_text_input_v3_interface;

const wl_interface xdg_positioner_interface{
    "xdg_positioner", 1, 0, nullptr, 0, nullptr};

const wl_interface* zxdg_decoration_get_toplevel_types[]{
    &zxdg_toplevel_decoration_v1_interface,
    &xdg_toplevel_interface,
};
const wl_message zxdg_decoration_manager_requests[]{
    {"destroy", "", nullptr},
    {"get_toplevel_decoration", "no", zxdg_decoration_get_toplevel_types},
};
const wl_interface zxdg_decoration_manager_v1_interface{
    "zxdg_decoration_manager_v1", 1, 2,
    zxdg_decoration_manager_requests, 0, nullptr};
const wl_message zxdg_toplevel_decoration_requests[]{
    {"destroy", "", nullptr},
    {"set_mode", "u", nullptr},
    {"unset_mode", "", nullptr},
};
const wl_message zxdg_toplevel_decoration_events[]{
    {"configure", "u", nullptr},
};
const wl_interface zxdg_toplevel_decoration_v1_interface{
    "zxdg_toplevel_decoration_v1", 1, 3,
    zxdg_toplevel_decoration_requests, 1,
    zxdg_toplevel_decoration_events};

const wl_interface* xdg_toplevel_set_parent_types[]{&xdg_toplevel_interface};
const wl_interface* xdg_toplevel_show_window_menu_types[]{
    &wl_seat_interface, nullptr, nullptr, nullptr};
const wl_interface* xdg_toplevel_move_types[]{&wl_seat_interface, nullptr};
const wl_interface* xdg_toplevel_resize_types[]{
    &wl_seat_interface, nullptr, nullptr};
const wl_interface* xdg_toplevel_set_fullscreen_types[]{&wl_output_interface};
const wl_message xdg_toplevel_requests[]{
    {"destroy", "", nullptr},
    {"set_parent", "?o", xdg_toplevel_set_parent_types},
    {"set_title", "s", nullptr},
    {"set_app_id", "s", nullptr},
    {"show_window_menu", "ouii", xdg_toplevel_show_window_menu_types},
    {"move", "ou", xdg_toplevel_move_types},
    {"resize", "ouu", xdg_toplevel_resize_types},
    {"set_max_size", "ii", nullptr},
    {"set_min_size", "ii", nullptr},
    {"set_maximized", "", nullptr},
    {"unset_maximized", "", nullptr},
    {"set_fullscreen", "?o", xdg_toplevel_set_fullscreen_types},
    {"unset_fullscreen", "", nullptr},
    {"set_minimized", "", nullptr},
};
const wl_message xdg_toplevel_events[]{
    {"configure", "iia", nullptr},
    {"close", "", nullptr},
};
const wl_interface xdg_toplevel_interface{
    "xdg_toplevel",
    1,
    14,
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

const wl_interface* zwp_text_input_manager_v3_get_text_input_types[]{
    &zwp_text_input_v3_interface,
    &wl_seat_interface,
};
const wl_message zwp_text_input_manager_v3_requests[]{
    {"destroy", "", nullptr},
    {
        "get_text_input",
        "no",
        zwp_text_input_manager_v3_get_text_input_types,
    },
};
const wl_interface zwp_text_input_manager_v3_interface{
    "zwp_text_input_manager_v3",
    1,
    2,
    zwp_text_input_manager_v3_requests,
    0,
    nullptr,
};

const wl_message zwp_text_input_v3_requests[]{
    {"destroy", "", nullptr},
    {"enable", "", nullptr},
    {"disable", "", nullptr},
    {"set_surrounding_text", "sii", nullptr},
    {"set_text_change_cause", "u", nullptr},
    {"set_content_type", "uu", nullptr},
    {"set_cursor_rectangle", "iiii", nullptr},
    {"commit", "", nullptr},
};
const wl_interface* zwp_text_input_v3_enter_types[]{
    &wl_surface_interface,
};
const wl_interface* zwp_text_input_v3_leave_types[]{
    &wl_surface_interface,
};
const wl_message zwp_text_input_v3_events[]{
    {"enter", "o", zwp_text_input_v3_enter_types},
    {"leave", "o", zwp_text_input_v3_leave_types},
    {"preedit_string", "sii", nullptr},
    {"commit_string", "s", nullptr},
    {"delete_surrounding_text", "uu", nullptr},
    {"done", "u", nullptr},
};
const wl_interface zwp_text_input_v3_interface{
    "zwp_text_input_v3",
    1,
    8,
    zwp_text_input_v3_requests,
    6,
    zwp_text_input_v3_events,
};

constexpr std::uint32_t xdg_surface_configure = 0;
constexpr std::uint32_t xdg_toplevel_configure = 0;
constexpr std::uint32_t xdg_toplevel_close = 1;
constexpr std::uint32_t xdg_toplevel_state_maximized = 1;
constexpr std::uint32_t xdg_toplevel_state_fullscreen = 2;
constexpr std::uint32_t xdg_toplevel_state_activated = 4;
constexpr std::uint32_t zwp_text_input_v3_enter = 0;
constexpr std::uint32_t zwp_text_input_v3_leave = 1;
constexpr std::uint32_t zwp_text_input_v3_preedit_string = 2;
constexpr std::uint32_t zwp_text_input_v3_commit_string = 3;
constexpr std::uint32_t zwp_text_input_v3_delete_surrounding_text = 4;
constexpr std::uint32_t zwp_text_input_v3_done = 5;

constexpr std::string_view test_keymap = R"(xkb_keymap {
xkb_keycodes "test" {
    minimum = 8;
    maximum = 255;
    <AE01> = 18;
    <AC01> = 38;
    <LFSH> = 50;
};
xkb_types "test" {
    virtual_modifiers NumLock,Alt,LevelThree,LAlt,RAlt,RControl,LControl,ScrollLock,LevelFive;
    type "ONE_LEVEL" {
        modifiers = none;
        map[none] = Level1;
    };
    type "ALPHABETIC" {
        modifiers = Shift+Lock;
        map[Shift] = Level2;
        map[Lock] = Level2;
        level_name[Level1] = "Base";
        level_name[Level2] = "Caps";
    };
};
xkb_compatibility "test" {
    interpret Shift_L+AnyOfOrNone(all) {
        action = SetMods(modifiers=Shift);
    };
};
xkb_symbols "test" {
    key <AE01> { [ 1, exclam ] };
    key <AC01> { type="ALPHABETIC", [ a, A ] };
    key <LFSH> { [ Shift_L ] };
    modifier_map Shift { <LFSH> };
};
};)";

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
  struct ClientDataSource {
    State* compositor = nullptr;
    wl_resource* resource = nullptr;
    std::vector<std::string> mime_types;
  };

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
    void (*set_app_id)(wl_client*, wl_resource*, const char*);
    void (*show_window_menu)(
        wl_client*, wl_resource*, wl_resource*, std::uint32_t,
        std::int32_t, std::int32_t);
    void (*move)(wl_client*, wl_resource*, wl_resource*, std::uint32_t);
    void (*resize)(
        wl_client*, wl_resource*, wl_resource*, std::uint32_t, std::uint32_t);
    void (*set_max_size)(wl_client*, wl_resource*, std::int32_t, std::int32_t);
    void (*set_min_size)(wl_client*, wl_resource*, std::int32_t, std::int32_t);
    void (*set_maximized)(wl_client*, wl_resource*);
    void (*unset_maximized)(wl_client*, wl_resource*);
    void (*set_fullscreen)(wl_client*, wl_resource*, wl_resource*);
    void (*unset_fullscreen)(wl_client*, wl_resource*);
    void (*set_minimized)(wl_client*, wl_resource*);
  };

  struct ZxdgDecorationManagerImplementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*get_toplevel_decoration)(
        wl_client*, wl_resource*, std::uint32_t, wl_resource*);
  };

  struct ZxdgToplevelDecorationImplementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*set_mode)(wl_client*, wl_resource*, std::uint32_t);
    void (*unset_mode)(wl_client*, wl_resource*);
  };

  struct PointerButtonRequest {
    std::uint32_t button = 0;
    bool pressed = false;
  };

  struct PointerScrollRequest {
    float delta_x = 0.0F;
    float delta_y = 0.0F;
  };

  struct KeyboardKeyRequest {
    std::uint32_t key = 0;
    bool pressed = false;
  };

  struct KeyboardModifiersRequest {
    bool shift = false;
    bool control = false;
    bool alt = false;
    bool super = false;
  };

  struct TextInputDeleteSurroundingRequest {
    std::uint32_t before_length = 0;
    std::uint32_t after_length = 0;
    std::optional<std::uint32_t> serial;
  };

  struct TextInputPreeditRequest {
    std::string text;
    std::int32_t cursor_begin = 0;
    std::int32_t cursor_end = 0;
    std::optional<std::uint32_t> serial;
  };

  struct TextInputCommitRequest {
    std::string text;
    std::optional<std::uint32_t> serial;
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
    if (decoration_manager_global != nullptr) {
      wl_global_destroy(decoration_manager_global);
    }
    if (seat_global != nullptr) {
      wl_global_destroy(seat_global);
    }
    if (output_global != nullptr) {
      wl_global_destroy(output_global);
    }
    if (data_device_manager_global != nullptr) {
      wl_global_destroy(data_device_manager_global);
    }
    if (text_input_manager_global != nullptr) {
      wl_global_destroy(text_input_manager_global);
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
    decoration_manager_global = wl_global_create(
        display,
        &zxdg_decoration_manager_v1_interface,
        1,
        this,
        &State::bind_decoration_manager);
    seat_global = wl_global_create(
        display,
        &wl_seat_interface,
        5,
        this,
        &State::bind_seat);
    output_global = wl_global_create(
        display,
        &wl_output_interface,
        2,
        this,
        &State::bind_output);
    data_device_manager_global = wl_global_create(
        display,
        &wl_data_device_manager_interface,
        3,
        this,
        &State::bind_data_device_manager);
    text_input_manager_global = wl_global_create(
        display,
        &zwp_text_input_manager_v3_interface,
        1,
        this,
        &State::bind_text_input_manager);
    if (compositor_global == nullptr || shell_global == nullptr ||
        decoration_manager_global == nullptr ||
        seat_global == nullptr || output_global == nullptr ||
        data_device_manager_global == nullptr ||
        text_input_manager_global == nullptr) {
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

  [[nodiscard]] bool wait_for_flag_value(
      const std::atomic_bool& flag,
      bool value) const {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline) {
      if (flag.load() == value) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return flag.load() == value;
  }

  [[nodiscard]] bool wait_for_cursor_count(std::uint32_t count) const {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline) {
      if (pointer_cursor_set_count.load() >= count) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return pointer_cursor_set_count.load() >= count;
  }

  void request_resize_configure(std::int32_t width, std::int32_t height) {
    request_resize_configure_state(width, height, false, false, false);
  }

  void request_resize_configure_state(
      std::int32_t width,
      std::int32_t height,
      bool activated,
      bool maximized,
      bool fullscreen) {
    resize_width.store(width);
    resize_height.store(height);
    resize_configure_activated.store(activated);
    resize_configure_maximized.store(maximized);
    resize_configure_fullscreen.store(fullscreen);
    resize_configure_pending.store(true);
  }

  void request_output_scale(std::int32_t scale) {
    output_scale.store(std::max(scale, 1));
    output_scale_sent.store(false);
    output_scale_pending.store(true);
  }

  void request_seat_capabilities(std::uint32_t capabilities) {
    seat_capabilities.store(capabilities);
    seat_capabilities_pending.store(true);
  }

  [[nodiscard]] WaylandConfigureState last_resize_configure_state() const {
    return WaylandConfigureState{
        .width = static_cast<std::int32_t>(resize_width.load()),
        .height = static_cast<std::int32_t>(resize_height.load()),
        .serial = last_resize_configure_serial.load(),
        .acked_serial = last_resize_configure_acked_serial.load(),
        .activated = resize_configure_activated.load(),
        .maximized = resize_configure_maximized.load(),
        .fullscreen = resize_configure_fullscreen.load(),
        .acked = resize_configure_acked.load(),
    };
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

  void request_pointer_scroll(float delta_x, float delta_y) {
    {
      std::lock_guard lock(pointer_scroll_mutex);
      pointer_scrolls.push_back(PointerScrollRequest{
          .delta_x = delta_x,
          .delta_y = delta_y,
      });
    }
    pointer_scroll_pending.store(true);
  }

  void request_drag_enter(std::int32_t x, std::int32_t y) {
    drag_x.store(x);
    drag_y.store(y);
    drag_enter_pending.store(true);
  }

  void request_drag_motion(std::int32_t x, std::int32_t y) {
    drag_x.store(x);
    drag_y.store(y);
    drag_motion_pending.store(true);
  }

  void request_drag_drop() {
    drag_drop_pending.store(true);
  }

  void request_drag_leave() {
    drag_leave_pending.store(true);
  }

  void set_drag_payloads(std::vector<WaylandMimePayload> payloads) {
    std::lock_guard lock(drag_payload_mutex);
    drag_payloads = std::move(payloads);
  }

  void set_drag_source_actions(
      std::uint32_t source_actions,
      std::uint32_t selected_action) {
    drag_source_actions.store(source_actions);
    drag_selected_action.store(selected_action);
  }

  void request_keyboard_key(std::uint32_t key, bool pressed) {
    {
      std::lock_guard lock(keyboard_key_mutex);
      keyboard_keys.push_back(KeyboardKeyRequest{
          .key = key,
          .pressed = pressed,
      });
    }
    keyboard_key_pending.store(true);
  }

  void request_keyboard_modifiers(
      bool shift,
      bool control,
      bool alt,
      bool super) {
    {
      std::lock_guard lock(keyboard_modifiers_mutex);
      keyboard_modifiers = KeyboardModifiersRequest{
          .shift = shift,
          .control = control,
          .alt = alt,
          .super = super,
      };
    }
    keyboard_modifiers_pending.store(true);
  }

  void request_keyboard_leave() {
    keyboard_leave_pending.store(true);
  }

  void request_text_input_enter() {
    text_input_enter_pending.store(true);
  }

  void request_text_input_preedit(std::string text) {
    const auto cursor = static_cast<std::int32_t>(text.size());
    request_text_input_preedit(std::move(text), cursor, cursor);
  }

  void request_text_input_preedit(
      std::string text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end) {
    request_text_input_preedit_with_serial(
        std::move(text),
        cursor_begin,
        cursor_end,
        std::nullopt);
  }

  void request_text_input_preedit_with_serial(
      std::string text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end,
      std::optional<std::uint32_t> serial) {
    {
      std::lock_guard lock(text_input_event_mutex);
      pending_text_input_preedit =
          TextInputPreeditRequest{
              .text = std::move(text),
              .cursor_begin = cursor_begin,
              .cursor_end = cursor_end,
              .serial = serial,
          };
    }
    text_input_preedit_pending.store(true);
  }

  void request_text_input_delete_surrounding(
      std::uint32_t before_length,
      std::uint32_t after_length) {
    {
      std::lock_guard lock(text_input_event_mutex);
      pending_text_input_delete_surrounding =
          TextInputDeleteSurroundingRequest{
              .before_length = before_length,
              .after_length = after_length,
          };
    }
    text_input_delete_surrounding_pending.store(true);
  }

  void request_text_input_commit(std::string text) {
    request_text_input_commit_with_serial(std::move(text), std::nullopt);
  }

  void request_text_input_commit_with_serial(
      std::string text,
      std::optional<std::uint32_t> serial) {
    {
      std::lock_guard lock(text_input_event_mutex);
      pending_text_input_commit =
          TextInputCommitRequest{
              .text = std::move(text),
              .serial = serial,
          };
    }
    text_input_commit_pending.store(true);
  }

  void request_text_input_leave() {
    text_input_leave_pending.store(true);
  }

  void request_close() {
    close_pending.store(true);
  }

  void set_clipboard_selection(
      std::vector<WaylandMimePayload> payloads) {
    {
      std::lock_guard lock(clipboard_selection_mutex);
      clipboard_selection_payloads = std::move(payloads);
    }
    {
      std::lock_guard lock(clipboard_receive_mutex);
      last_clipboard_receive_mime.clear();
    }
    clipboard_selection_sent.store(false);
    clipboard_selection_pending.store(true);
  }

  void request_clipboard_client_selection(std::string_view mime_type) {
    {
      std::lock_guard lock(clipboard_client_selection_request_mutex);
      pending_clipboard_client_selection_mime = std::string(mime_type);
    }
    {
      std::lock_guard lock(clipboard_client_selection_payload_mutex);
      last_clipboard_client_selection_mime.clear();
      last_clipboard_client_selection_payload_value.clear();
    }
    clipboard_client_selection_payload_received.store(false);
    clipboard_client_selection_request_pending.store(true);
  }

  [[nodiscard]] std::vector<std::string>
  clipboard_client_selection_mime_types() const {
    std::lock_guard lock(clipboard_client_selection_mutex);
    if (clipboard_client_selection_source == nullptr) {
      return {};
    }
    return clipboard_client_selection_source->mime_types;
  }

  [[nodiscard]] std::string last_clipboard_client_selection_payload() const {
    std::lock_guard lock(clipboard_client_selection_payload_mutex);
    return last_clipboard_client_selection_payload_value;
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

  static void bind_decoration_manager(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* resource = wl_resource_create(
        client, &zxdg_decoration_manager_v1_interface,
        std::min<std::uint32_t>(version, 1), id);
    wl_resource_set_implementation(
        resource, &decoration_manager_implementation, data, nullptr);
  }

  static void get_toplevel_decoration(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id,
      wl_resource*) {
    auto* decoration = wl_resource_create(
        client, &zxdg_toplevel_decoration_v1_interface, 1, id);
    wl_resource_set_implementation(
        decoration, &toplevel_decoration_implementation,
        wl_resource_get_user_data(resource), nullptr);
  }

  static void set_decoration_mode(
      wl_client*, wl_resource* resource, std::uint32_t mode) {
    auto* state = static_cast<State*>(wl_resource_get_user_data(resource));
    if (state == nullptr) {
      return;
    }
    state->client_side_decoration_requested.store(mode == 1);
    state->server_side_decoration_requested.store(mode == 2);
    wl_resource_post_event(resource, 0, mode);
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
    wl_seat_send_capabilities(resource, compositor->seat_capabilities.load());
    if (wl_resource_get_version(resource) >= WL_SEAT_NAME_SINCE_VERSION) {
      wl_seat_send_name(resource, "test-seat");
    }
  }

  static void bind_output(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<State*>(data);
    auto* resource = wl_resource_create(
        client,
        &wl_output_interface,
        std::min<std::uint32_t>(version, 2),
        id);
    compositor->output_resource = resource;
    wl_resource_set_implementation(
        resource,
        &output_implementation,
        compositor,
        [](wl_resource* destroyed) {
          auto* state =
              static_cast<State*>(wl_resource_get_user_data(destroyed));
          if (state != nullptr && state->output_resource == destroyed) {
            state->output_resource = nullptr;
          }
        });
    wl_output_send_geometry(
        resource,
        0,
        0,
        600,
        340,
        WL_OUTPUT_SUBPIXEL_UNKNOWN,
        "CGPUI",
        "Test Output",
        WL_OUTPUT_TRANSFORM_NORMAL);
    wl_output_send_mode(
        resource,
        WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED,
        1920,
        1080,
        60000);
    wl_output_send_scale(resource, compositor->output_scale.load());
    wl_output_send_done(resource);
  }

  static void bind_data_device_manager(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<State*>(data);
    auto* resource = wl_resource_create(
        client,
        &wl_data_device_manager_interface,
        std::min<std::uint32_t>(version, 3),
        id);
    wl_resource_set_implementation(
        resource,
        &data_device_manager_implementation,
        compositor,
        nullptr);
  }
  static void bind_text_input_manager(
      wl_client* client,
      void* data,
      std::uint32_t version,
      std::uint32_t id) {
    auto* compositor = static_cast<State*>(data);
    auto* resource = wl_resource_create(
        client,
        &zwp_text_input_manager_v3_interface,
        std::min<std::uint32_t>(version, 1),
        id);
    wl_resource_set_implementation(
        resource,
        &text_input_manager_implementation,
        compositor,
        nullptr);
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
  static void seat_get_keyboard(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id);
  static void data_device_manager_create_data_source(
      wl_client* client,
      wl_resource*,
      std::uint32_t id);
  static void data_device_manager_get_data_device(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id,
      wl_resource*) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    auto* data_device = wl_resource_create(
        client,
        &wl_data_device_interface,
        std::min<std::uint32_t>(wl_resource_get_version(resource), 3),
        id);
    compositor->data_device_resource = data_device;
    wl_resource_set_implementation(
        data_device,
        &data_device_implementation,
        compositor,
        &State::handle_data_device_destroyed);
    compositor->clipboard_selection_pending.store(true);
  }
  static void text_input_manager_get_text_input(
      wl_client* client,
      wl_resource* resource,
      std::uint32_t id,
      wl_resource*) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    auto* text_input = wl_resource_create(
        client,
        &zwp_text_input_v3_interface,
        std::min<std::uint32_t>(wl_resource_get_version(resource), 1),
        id);
    compositor->text_input_resource = text_input;
    wl_resource_set_implementation(
        text_input,
        &text_input_implementation,
        compositor,
        &State::handle_text_input_destroyed);
  }
  static void text_input_enable(wl_client*, wl_resource* resource) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      std::lock_guard lock(compositor->text_input_client_state_mutex);
      compositor->text_input_client_state.enabled = true;
    }
  }
  static void text_input_disable(wl_client*, wl_resource* resource) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      std::lock_guard lock(compositor->text_input_client_state_mutex);
      compositor->text_input_client_state.enabled = false;
    }
  }
  static void text_input_set_surrounding_text(
      wl_client*,
      wl_resource* resource,
      const char* text,
      std::int32_t cursor,
      std::int32_t anchor) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      std::lock_guard lock(compositor->text_input_client_state_mutex);
      compositor->text_input_client_state.surrounding_text =
          text == nullptr ? std::string{} : std::string{text};
      compositor->text_input_client_state.cursor = cursor;
      compositor->text_input_client_state.anchor = anchor;
    }
  }
  static void text_input_set_text_change_cause(
      wl_client*,
      wl_resource*,
      std::uint32_t) {}
  static void text_input_set_content_type(
      wl_client*,
      wl_resource* resource,
      std::uint32_t hint,
      std::uint32_t purpose) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      std::lock_guard lock(compositor->text_input_client_state_mutex);
      compositor->text_input_client_state.content_hint = hint;
      compositor->text_input_client_state.content_purpose = purpose;
    }
  }
  static void text_input_set_cursor_rectangle(
      wl_client*,
      wl_resource* resource,
      std::int32_t x,
      std::int32_t y,
      std::int32_t width,
      std::int32_t height) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      std::lock_guard lock(compositor->text_input_client_state_mutex);
      compositor->text_input_client_state.cursor_rect =
          WaylandTextInputRect{.x = x, .y = y, .width = width, .height = height};
    }
  }
  static void text_input_commit(wl_client*, wl_resource* resource) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      compositor->text_input_client_state_committed.store(true);
    }
  }
  void send_keyboard_keymap();
  static void handle_seat_destroyed(wl_resource* resource);
  static void handle_data_device_destroyed(wl_resource* resource);
  static void handle_text_input_destroyed(wl_resource* resource);
  static void handle_clipboard_offer_destroyed(wl_resource* resource);
  static void handle_drag_offer_destroyed(wl_resource* resource);
  static void handle_client_data_source_destroyed(wl_resource* resource);
  static void data_source_offer(
      wl_client*,
      wl_resource* resource,
      const char* mime_type);
  static void data_source_destroy(wl_client*, wl_resource* resource);
  static void data_source_set_actions(wl_client*, wl_resource*, std::uint32_t) {}
  static void data_device_set_selection(
      wl_client*,
      wl_resource* resource,
      wl_resource* source,
      std::uint32_t);
  static void data_offer_receive(
      wl_client*,
      wl_resource* resource,
      const char* mime_type,
      int fd);
  static void data_offer_accept(
      wl_client*,
      wl_resource* resource,
      std::uint32_t serial,
      const char* mime_type);
  static void data_offer_finish(wl_client*, wl_resource* resource);
  static void data_offer_set_actions(
      wl_client*,
      wl_resource* resource,
      std::uint32_t dnd_actions,
      std::uint32_t preferred_action);
  static void pointer_set_cursor(
      wl_client*,
      wl_resource* resource,
      std::uint32_t,
      wl_resource*,
      std::int32_t,
      std::int32_t) {
    auto* compositor =
        static_cast<State*>(wl_resource_get_user_data(resource));
    if (compositor != nullptr) {
      compositor->pointer_cursor_set.store(true);
      compositor->pointer_cursor_set_count.fetch_add(1);
    }
  }
  static void handle_pointer_destroyed(wl_resource* resource);
  static void handle_keyboard_destroyed(wl_resource* resource);

  [[nodiscard]] SurfaceState* find_surface(wl_resource* resource) const;
  [[nodiscard]] SurfaceState* first_pointer_surface() const;
  [[nodiscard]] SurfaceState* first_keyboard_surface() const;

  void dispatch_pending_close();
  void dispatch_pending_resize_configure();
  void dispatch_pending_output_scale();
  void dispatch_pending_seat_capabilities();
  void dispatch_pending_pointer_move();
  void dispatch_pending_pointer_button();
  void dispatch_pending_pointer_scroll();
  void dispatch_pending_drag_enter();
  void dispatch_pending_drag_motion();
  void dispatch_pending_drag_drop();
  void dispatch_pending_drag_leave();
  void dispatch_pending_keyboard_modifiers();
  void dispatch_pending_keyboard_key();
  void dispatch_pending_keyboard_leave();
  void dispatch_pending_text_input_enter();
  void dispatch_pending_text_input_preedit();
  void dispatch_pending_text_input_delete_surrounding();
  void dispatch_pending_text_input_commit();
  void dispatch_pending_text_input_leave();
  std::uint32_t consume_text_input_serial(
      std::optional<std::uint32_t> serial);
  void dispatch_pending_clipboard_selection();
  void dispatch_pending_clipboard_client_selection_request();

  void run() {
    while (running.load()) {
      dispatch_pending_clipboard_selection();
      dispatch_pending_clipboard_client_selection_request();
      dispatch_pending_close();
      dispatch_pending_resize_configure();
      dispatch_pending_output_scale();
      dispatch_pending_seat_capabilities();
      dispatch_pending_pointer_move();
      dispatch_pending_pointer_button();
      dispatch_pending_pointer_scroll();
      dispatch_pending_drag_enter();
      dispatch_pending_drag_motion();
      dispatch_pending_drag_drop();
      dispatch_pending_drag_leave();
      dispatch_pending_keyboard_modifiers();
      dispatch_pending_keyboard_key();
      dispatch_pending_keyboard_leave();
      dispatch_pending_text_input_enter();
      dispatch_pending_text_input_preedit();
      dispatch_pending_text_input_delete_surrounding();
      dispatch_pending_text_input_commit();
      dispatch_pending_text_input_leave();
      const int result = wl_event_loop_dispatch(wl_display_get_event_loop(display), 10);
      if (result < 0) {
        running.store(false);
        break;
      }
      dispatch_pending_clipboard_selection();
      dispatch_pending_clipboard_client_selection_request();
      dispatch_pending_close();
      dispatch_pending_resize_configure();
      dispatch_pending_output_scale();
      dispatch_pending_seat_capabilities();
      dispatch_pending_pointer_move();
      dispatch_pending_pointer_button();
      dispatch_pending_pointer_scroll();
      dispatch_pending_drag_enter();
      dispatch_pending_drag_motion();
      dispatch_pending_drag_drop();
      dispatch_pending_drag_leave();
      dispatch_pending_keyboard_modifiers();
      dispatch_pending_keyboard_key();
      dispatch_pending_keyboard_leave();
      dispatch_pending_text_input_enter();
      dispatch_pending_text_input_preedit();
      dispatch_pending_text_input_delete_surrounding();
      dispatch_pending_text_input_commit();
      dispatch_pending_text_input_leave();
      wl_display_flush_clients(display);
    }
  }

  static const struct wl_compositor_interface compositor_implementation;
  static const struct wl_surface_interface surface_implementation;
  static const struct wl_seat_interface seat_implementation;
  static const struct wl_output_interface output_implementation;
  static const struct wl_pointer_interface pointer_implementation;
  static const struct wl_keyboard_interface keyboard_implementation;
  static const struct wl_data_device_manager_interface
      data_device_manager_implementation;
  static const struct wl_data_device_interface data_device_implementation;
  static const struct wl_data_offer_interface data_offer_implementation;
  static const struct wl_data_source_interface data_source_implementation;
  struct ZwpTextInputManagerV3Implementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*get_text_input)(wl_client*, wl_resource*, std::uint32_t, wl_resource*);
  };
  struct ZwpTextInputV3Implementation {
    void (*destroy)(wl_client*, wl_resource*);
    void (*enable)(wl_client*, wl_resource*);
    void (*disable)(wl_client*, wl_resource*);
    void (*set_surrounding_text)(
        wl_client*,
        wl_resource*,
        const char*,
        std::int32_t,
        std::int32_t);
    void (*set_text_change_cause)(wl_client*, wl_resource*, std::uint32_t);
    void (*set_content_type)(
        wl_client*,
        wl_resource*,
        std::uint32_t,
        std::uint32_t);
    void (*set_cursor_rectangle)(
        wl_client*,
        wl_resource*,
        std::int32_t,
        std::int32_t,
        std::int32_t,
        std::int32_t);
    void (*commit)(wl_client*, wl_resource*);
  };
  static const ZwpTextInputManagerV3Implementation
      text_input_manager_implementation;
  static const ZwpTextInputV3Implementation text_input_implementation;
  static const XdgWmBaseImplementation shell_implementation;
  static const XdgSurfaceImplementation xdg_surface_implementation;
  static const XdgToplevelImplementation toplevel_implementation;
  static const ZxdgDecorationManagerImplementation
      decoration_manager_implementation;
  static const ZxdgToplevelDecorationImplementation
      toplevel_decoration_implementation;
  static void surface_set_buffer_scale(
      wl_client* client,
      wl_resource* resource,
      std::int32_t scale);

  wl_display* display = nullptr;
  wl_global* compositor_global = nullptr;
  wl_global* shell_global = nullptr;
  wl_global* decoration_manager_global = nullptr;
  wl_global* seat_global = nullptr;
  wl_global* output_global = nullptr;
  wl_global* data_device_manager_global = nullptr;
  wl_global* text_input_manager_global = nullptr;
  wl_resource* seat_resource = nullptr;
  wl_resource* pointer_resource = nullptr;
  wl_resource* keyboard_resource = nullptr;
  wl_resource* output_resource = nullptr;
  wl_resource* data_device_resource = nullptr;
  wl_resource* text_input_resource = nullptr;
  wl_resource* clipboard_offer_resource = nullptr;
  wl_resource* drag_offer_resource = nullptr;
  std::filesystem::path runtime_dir;
  std::string socket_name;
  std::vector<std::unique_ptr<SurfaceState>> surfaces;
  std::thread server_thread;
  std::atomic_bool running{false};
  std::atomic_bool close_on_initial_configure_ack{false};
  std::atomic_bool close_pending{false};
  std::atomic_bool close_sent{false};
  std::atomic_bool initial_configure_acked{false};
  std::atomic_bool minimize_requested{false};
  std::atomic_bool maximize_requested{false};
  std::atomic_bool unmaximize_requested{false};
  std::atomic_bool fullscreen_requested{false};
  std::atomic_bool unfullscreen_requested{false};
  std::atomic_bool client_side_decoration_requested{false};
  std::atomic_bool server_side_decoration_requested{false};
  std::atomic_bool parent_requested{false};
  std::atomic_bool resize_configure_pending{false};
  std::atomic_bool resize_configure_sent{false};
  std::atomic_bool resize_configure_acked{false};
  std::atomic_bool resize_configure_activated{false};
  std::atomic_bool resize_configure_maximized{false};
  std::atomic_bool resize_configure_fullscreen{false};
  std::atomic_bool output_scale_pending{false};
  std::atomic_bool output_scale_sent{false};
  std::atomic_bool seat_capabilities_pending{false};
  std::atomic_bool pointer_bound{false};
  std::atomic_bool keyboard_bound{false};
  std::atomic_bool pointer_move_pending{false};
  std::atomic_bool pointer_move_sent{false};
  std::atomic_bool pointer_button_pending{false};
  std::atomic_bool pointer_button_sent{false};
  std::atomic_bool pointer_scroll_pending{false};
  std::atomic_bool pointer_scroll_sent{false};
  std::atomic_bool drag_enter_pending{false};
  std::atomic_bool drag_enter_sent{false};
  std::atomic_bool drag_motion_pending{false};
  std::atomic_bool drag_motion_sent{false};
  std::atomic_bool drag_drop_pending{false};
  std::atomic_bool drag_drop_sent{false};
  std::atomic_bool drag_leave_pending{false};
  std::atomic_bool drag_leave_sent{false};
  std::atomic_bool drag_offer_accepted{false};
  std::atomic_bool drag_offer_actions_set{false};
  std::atomic_bool drag_offer_finished{false};
  std::atomic_bool pointer_cursor_set{false};
  std::atomic_uint32_t pointer_cursor_set_count{0};
  std::atomic_bool keyboard_keymap_sent{false};
  std::atomic_bool keyboard_modifiers_pending{false};
  std::atomic_bool keyboard_modifiers_sent{false};
  std::atomic_bool keyboard_key_pending{false};
  std::atomic_bool keyboard_key_sent{false};
  std::atomic_bool keyboard_leave_pending{false};
  std::atomic_bool keyboard_leave_sent{false};
  std::atomic_bool text_input_enter_pending{false};
  std::atomic_bool text_input_enter_sent{false};
  std::atomic_bool text_input_preedit_pending{false};
  std::atomic_bool text_input_preedit_sent{false};
  std::atomic_bool text_input_delete_surrounding_pending{false};
  std::atomic_bool text_input_delete_surrounding_sent{false};
  std::atomic_bool text_input_commit_pending{false};
  std::atomic_bool text_input_commit_sent{false};
  std::atomic_bool text_input_leave_pending{false};
  std::atomic_bool text_input_leave_sent{false};
  std::atomic_bool text_input_client_state_committed{false};
  std::atomic_bool clipboard_selection_pending{false};
  std::atomic_bool clipboard_selection_sent{false};
  std::atomic_bool clipboard_client_selection_set{false};
  std::atomic_bool clipboard_client_selection_request_pending{false};
  std::atomic_bool clipboard_client_selection_payload_received{false};
  std::atomic_int resize_width{0};
  std::atomic_int resize_height{0};
  std::atomic_int output_scale{1};
  std::atomic_uint32_t seat_capabilities{
      WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD};
  std::atomic_int surface_buffer_scale{1};
  std::atomic_int pointer_x{0};
  std::atomic_int pointer_y{0};
  std::atomic_int drag_x{0};
  std::atomic_int drag_y{0};
  std::mutex keyboard_key_mutex;
  std::deque<KeyboardKeyRequest> keyboard_keys;
  std::mutex keyboard_modifiers_mutex;
  KeyboardModifiersRequest keyboard_modifiers;
  std::mutex text_input_event_mutex;
  TextInputPreeditRequest pending_text_input_preedit;
  TextInputDeleteSurroundingRequest pending_text_input_delete_surrounding;
  TextInputCommitRequest pending_text_input_commit;
  mutable std::mutex text_input_client_state_mutex;
  WaylandTextInputClientState text_input_client_state;
  std::mutex pointer_button_mutex;
  std::deque<PointerButtonRequest> pointer_buttons;
  std::mutex pointer_scroll_mutex;
  std::deque<PointerScrollRequest> pointer_scrolls;
  std::mutex clipboard_selection_mutex;
  std::vector<WaylandMimePayload> clipboard_selection_payloads;
  mutable std::mutex clipboard_receive_mutex;
  std::string last_clipboard_receive_mime;
  mutable std::mutex clipboard_client_selection_mutex;
  ClientDataSource* clipboard_client_selection_source = nullptr;
  mutable std::mutex clipboard_client_selection_request_mutex;
  std::string pending_clipboard_client_selection_mime;
  mutable std::mutex clipboard_client_selection_payload_mutex;
  std::string last_clipboard_client_selection_mime;
  std::string last_clipboard_client_selection_payload_value;
  std::mutex drag_payload_mutex;
  std::vector<WaylandMimePayload> drag_payloads;
  mutable std::mutex drag_receive_mutex;
  std::string last_drag_receive_mime;
  mutable std::mutex drag_accept_mutex;
  std::string last_drag_accept_mime;
  std::atomic_uint32_t drag_source_actions{
      WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};
  std::atomic_uint32_t drag_selected_action{
      WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};
  std::atomic_uint32_t last_drag_offer_actions_value{
      WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};
  std::atomic_uint32_t last_drag_preferred_action_value{
      WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};
  std::uint32_t next_configure_serial = 1;
  std::uint32_t resize_configure_serial = 0;
  std::atomic_uint32_t last_resize_configure_serial{0};
  std::atomic_uint32_t last_resize_configure_acked_serial{0};
  std::uint32_t next_pointer_serial = 1;
  std::uint32_t pointer_time = 1;
  bool pointer_entered = false;
  std::uint32_t next_drag_serial = 1;
  std::uint32_t drag_time = 1;
  bool drag_entered = false;
  std::uint32_t next_keyboard_serial = 1;
  std::uint32_t keyboard_time = 1;
  bool keyboard_entered = false;
  std::uint32_t next_text_input_serial = 1;
  bool text_input_entered = false;
};

struct WaylandTestCompositor::State::SurfaceState {
  WaylandTestCompositor::State* compositor = nullptr;
  wl_resource* surface = nullptr;
  wl_resource* xdg_surface = nullptr;
  wl_resource* toplevel = nullptr;
  std::uint32_t initial_configure_serial = 0;
  bool initial_configure_sent = false;
  bool output_enter_sent = false;
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
    state->compositor->last_resize_configure_acked_serial.store(serial);
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
  if (!state->output_enter_sent &&
      state->compositor->output_resource != nullptr) {
    state->output_enter_sent = true;
    wl_surface_send_enter(
        state->surface,
        state->compositor->output_resource);
  }
  state->initial_configure_serial = state->compositor->next_configure_serial++;
  wl_resource_post_event(
      state->xdg_surface,
      xdg_surface_configure,
      state->initial_configure_serial);
  wl_display_flush_clients(state->compositor->display);
}

void WaylandTestCompositor::State::surface_set_buffer_scale(
    wl_client*,
    wl_resource* resource,
    std::int32_t scale) {
  auto* state = static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
  if (state == nullptr) {
    return;
  }
  state->compositor->surface_buffer_scale.store(scale);
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

WaylandTestCompositor::State::SurfaceState*
WaylandTestCompositor::State::first_keyboard_surface() const {
  return first_pointer_surface();
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
  compositor->pointer_bound.store(true);
  wl_resource_set_implementation(
      pointer,
      &pointer_implementation,
      compositor,
      &WaylandTestCompositor::State::handle_pointer_destroyed);
}

void WaylandTestCompositor::State::seat_get_keyboard(
    wl_client* client,
    wl_resource* resource,
    std::uint32_t id) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  auto* keyboard = wl_resource_create(
      client,
      &wl_keyboard_interface,
      std::min<std::uint32_t>(wl_resource_get_version(resource), 5),
      id);
  compositor->keyboard_resource = keyboard;
  compositor->keyboard_bound.store(true);
  wl_resource_set_implementation(
      keyboard,
      &keyboard_implementation,
      compositor,
      &WaylandTestCompositor::State::handle_keyboard_destroyed);
  compositor->send_keyboard_keymap();
}

void WaylandTestCompositor::State::send_keyboard_keymap() {
  if (keyboard_resource == nullptr) {
    return;
  }

  const auto keymap_size = static_cast<std::size_t>(test_keymap.size() + 1);
  char template_path[] = "/tmp/cgpui-keymap-XXXXXX";
  const int fd = mkstemp(template_path);
  if (fd < 0) {
    return;
  }
  unlink(template_path);
  if (ftruncate(fd, static_cast<off_t>(keymap_size)) != 0) {
    close(fd);
    return;
  }
  const auto written =
      write(fd, test_keymap.data(), static_cast<unsigned int>(test_keymap.size()));
  if (written != static_cast<ssize_t>(test_keymap.size())) {
    close(fd);
    return;
  }
  const char terminator = '\0';
  if (write(fd, &terminator, 1) != 1) {
    close(fd);
    return;
  }
  lseek(fd, 0, SEEK_SET);
  wl_keyboard_send_keymap(
      keyboard_resource,
      WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
      fd,
      static_cast<std::uint32_t>(keymap_size));
  close(fd);
  wl_display_flush_clients(display);
  keyboard_keymap_sent.store(true);
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
    compositor->pointer_bound.store(false);
    compositor->pointer_entered = false;
  }
}

void WaylandTestCompositor::State::handle_data_device_destroyed(
    wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->data_device_resource == resource) {
    compositor->data_device_resource = nullptr;
    compositor->clipboard_offer_resource = nullptr;
    compositor->drag_offer_resource = nullptr;
    compositor->drag_entered = false;
  }
}

void WaylandTestCompositor::State::handle_text_input_destroyed(
    wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->text_input_resource == resource) {
    compositor->text_input_resource = nullptr;
    compositor->text_input_entered = false;
  }
}

void WaylandTestCompositor::State::handle_clipboard_offer_destroyed(
    wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->clipboard_offer_resource == resource) {
    compositor->clipboard_offer_resource = nullptr;
  }
}

void WaylandTestCompositor::State::handle_drag_offer_destroyed(
    wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->drag_offer_resource == resource) {
    compositor->drag_offer_resource = nullptr;
  }
}

void WaylandTestCompositor::State::data_offer_receive(
    wl_client*,
    wl_resource* resource,
    const char* mime_type,
    int fd) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor == nullptr || mime_type == nullptr || fd < 0) {
    if (fd >= 0) {
      close(fd);
    }
    return;
  }

  std::string payload;
  bool found = false;
  const bool is_clipboard_offer = resource == compositor->clipboard_offer_resource;
  const bool is_drag_offer = resource == compositor->drag_offer_resource;
  if (is_clipboard_offer) {
    std::lock_guard lock(compositor->clipboard_selection_mutex);
    const auto selected = std::ranges::find_if(
        compositor->clipboard_selection_payloads,
        [mime_type](const WaylandMimePayload& candidate) {
          return candidate.mime_type == std::string_view{mime_type};
        });
    if (selected != compositor->clipboard_selection_payloads.end()) {
      payload = selected->payload;
      found = true;
    }
  } else if (is_drag_offer) {
    std::lock_guard lock(compositor->drag_payload_mutex);
    const auto selected = std::ranges::find_if(
        compositor->drag_payloads,
        [mime_type](const WaylandMimePayload& candidate) {
          return candidate.mime_type == std::string_view{mime_type};
        });
    if (selected != compositor->drag_payloads.end()) {
      payload = selected->payload;
      found = true;
    }
  }

  if (found) {
    std::size_t written = 0;
    while (written < payload.size()) {
      const auto count = write(
          fd,
          payload.data() + written,
          static_cast<unsigned int>(payload.size() - written));
      if (count <= 0) {
        break;
      }
      written += static_cast<std::size_t>(count);
    }
    if (is_clipboard_offer) {
      std::lock_guard lock(compositor->clipboard_receive_mutex);
      compositor->last_clipboard_receive_mime = mime_type;
    } else if (is_drag_offer) {
      std::lock_guard lock(compositor->drag_receive_mutex);
      compositor->last_drag_receive_mime = mime_type;
    }
  }
  close(fd);
}

void WaylandTestCompositor::State::data_offer_accept(
    wl_client*,
    wl_resource* resource,
    std::uint32_t serial,
    const char* mime_type) {
  (void)serial;
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor == nullptr || resource != compositor->drag_offer_resource) {
    return;
  }
  {
    std::lock_guard lock(compositor->drag_accept_mutex);
    compositor->last_drag_accept_mime =
        mime_type == nullptr ? std::string{} : std::string(mime_type);
  }
  compositor->drag_offer_accepted.store(true);
}

void WaylandTestCompositor::State::data_offer_finish(
    wl_client*,
    wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && resource == compositor->drag_offer_resource) {
    compositor->drag_offer_finished.store(true);
  }
}

void WaylandTestCompositor::State::data_offer_set_actions(
    wl_client*,
    wl_resource* resource,
    std::uint32_t dnd_actions,
    std::uint32_t preferred_action) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor == nullptr || resource != compositor->drag_offer_resource) {
    return;
  }
  compositor->last_drag_offer_actions_value.store(dnd_actions);
  compositor->last_drag_preferred_action_value.store(preferred_action);
  compositor->drag_offer_actions_set.store(true);
}

void WaylandTestCompositor::State::data_device_manager_create_data_source(
    wl_client* client,
    wl_resource* resource,
    std::uint32_t id) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  auto* source_state = new ClientDataSource{
      .compositor = compositor,
  };
  auto* source_resource =
      wl_resource_create(client, &wl_data_source_interface, 3, id);
  if (source_resource == nullptr) {
    delete source_state;
    return;
  }

  source_state->resource = source_resource;
  wl_resource_set_implementation(
      source_resource,
      &data_source_implementation,
      source_state,
      &State::handle_client_data_source_destroyed);
}

void WaylandTestCompositor::State::data_source_offer(
    wl_client*,
    wl_resource* resource,
    const char* mime_type) {
  auto* source = static_cast<ClientDataSource*>(wl_resource_get_user_data(resource));
  if (source == nullptr || source->compositor == nullptr || mime_type == nullptr) {
    return;
  }

  std::lock_guard lock(source->compositor->clipboard_client_selection_mutex);
  source->mime_types.emplace_back(mime_type);
}

void WaylandTestCompositor::State::data_source_destroy(
    wl_client*,
    wl_resource* resource) {
  wl_resource_destroy(resource);
}

void WaylandTestCompositor::State::data_device_set_selection(
    wl_client*,
    wl_resource* resource,
    wl_resource* source_resource,
    std::uint32_t) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor == nullptr) {
    return;
  }

  auto* source = source_resource == nullptr
      ? nullptr
      : static_cast<ClientDataSource*>(wl_resource_get_user_data(source_resource));
  {
    std::lock_guard lock(compositor->clipboard_client_selection_mutex);
    compositor->clipboard_client_selection_source = source;
  }
  compositor->clipboard_client_selection_payload_received.store(false);
  compositor->clipboard_client_selection_set.store(source != nullptr);
}

void WaylandTestCompositor::State::handle_client_data_source_destroyed(
    wl_resource* resource) {
  auto* source = static_cast<ClientDataSource*>(wl_resource_get_user_data(resource));
  if (source == nullptr) {
    return;
  }
  if (source->compositor != nullptr) {
    std::lock_guard lock(source->compositor->clipboard_client_selection_mutex);
    if (source->compositor->clipboard_client_selection_source == source) {
      source->compositor->clipboard_client_selection_source = nullptr;
      source->compositor->clipboard_client_selection_set.store(false);
    }
  }
  delete source;
}

void WaylandTestCompositor::State::handle_keyboard_destroyed(wl_resource* resource) {
  auto* compositor =
      static_cast<WaylandTestCompositor::State*>(wl_resource_get_user_data(resource));
  if (compositor != nullptr && compositor->keyboard_resource == resource) {
    compositor->keyboard_resource = nullptr;
    compositor->keyboard_bound.store(false);
    compositor->keyboard_entered = false;
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
    const auto add_state = [&states](std::uint32_t state) {
      auto* slot = static_cast<std::uint32_t*>(
          wl_array_add(&states, sizeof(std::uint32_t)));
      if (slot != nullptr) {
        *slot = state;
      }
    };
    if (resize_configure_maximized.load()) {
      add_state(xdg_toplevel_state_maximized);
    }
    if (resize_configure_fullscreen.load()) {
      add_state(xdg_toplevel_state_fullscreen);
    }
    if (resize_configure_activated.load()) {
      add_state(xdg_toplevel_state_activated);
    }
    resize_configure_serial = next_configure_serial++;
    last_resize_configure_serial.store(resize_configure_serial);
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

void WaylandTestCompositor::State::dispatch_pending_seat_capabilities() {
  if (!seat_capabilities_pending.exchange(false)) {
    return;
  }
  if (seat_resource == nullptr) {
    seat_capabilities_pending.store(true);
    return;
  }
  wl_seat_send_capabilities(seat_resource, seat_capabilities.load());
  wl_display_flush_clients(display);
}

void WaylandTestCompositor::State::dispatch_pending_output_scale() {
  if (!output_scale_pending.exchange(false)) {
    return;
  }
  if (output_resource == nullptr) {
    output_scale_pending.store(true);
    return;
  }
  wl_output_send_scale(output_resource, output_scale.load());
  wl_output_send_done(output_resource);
  wl_display_flush_clients(display);
  output_scale_sent.store(true);
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

void WaylandTestCompositor::State::dispatch_pending_pointer_scroll() {
  if (!pointer_scroll_pending.exchange(false)) {
    return;
  }

  PointerScrollRequest request{};
  {
    std::lock_guard lock(pointer_scroll_mutex);
    if (pointer_scrolls.empty()) {
      return;
    }
    request = pointer_scrolls.front();
    pointer_scrolls.pop_front();
    if (!pointer_scrolls.empty()) {
      pointer_scroll_pending.store(true);
    }
  }

  const SurfaceState* surface = first_pointer_surface();
  if (pointer_resource == nullptr || surface == nullptr) {
    {
      std::lock_guard lock(pointer_scroll_mutex);
      pointer_scrolls.push_front(request);
    }
    pointer_scroll_pending.store(true);
    return;
  }

  if (!pointer_entered) {
    wl_pointer_send_enter(
        pointer_resource,
        next_pointer_serial++,
        surface->surface,
        wl_fixed_from_int(pointer_x.load()),
        wl_fixed_from_int(pointer_y.load()));
    pointer_entered = true;
  }
  if (request.delta_y != 0.0F) {
    wl_pointer_send_axis(
        pointer_resource,
        pointer_time,
        WL_POINTER_AXIS_VERTICAL_SCROLL,
        wl_fixed_from_double(static_cast<double>(request.delta_y)));
  }
  if (request.delta_x != 0.0F) {
    wl_pointer_send_axis(
        pointer_resource,
        pointer_time,
        WL_POINTER_AXIS_HORIZONTAL_SCROLL,
        wl_fixed_from_double(static_cast<double>(request.delta_x)));
  }
  if (wl_resource_get_version(pointer_resource) >= WL_POINTER_FRAME_SINCE_VERSION) {
    wl_pointer_send_frame(pointer_resource);
  }
  ++pointer_time;
  wl_display_flush_clients(display);
  pointer_scroll_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_drag_enter() {
  if (!drag_enter_pending.exchange(false)) {
    return;
  }

  const SurfaceState* surface = first_pointer_surface();
  if (data_device_resource == nullptr || surface == nullptr) {
    drag_enter_pending.store(true);
    return;
  }

  wl_resource* offer = nullptr;
  std::vector<WaylandMimePayload> payloads;
  {
    std::lock_guard lock(drag_payload_mutex);
    payloads = drag_payloads;
  }
  if (!payloads.empty()) {
    wl_client* client = wl_resource_get_client(data_device_resource);
    offer = wl_resource_create(
        client,
        &wl_data_offer_interface,
        std::min<std::uint32_t>(wl_resource_get_version(data_device_resource), 3),
        0);
    if (offer == nullptr) {
      drag_enter_pending.store(true);
      return;
    }

    drag_offer_resource = offer;
    wl_resource_set_implementation(
        offer,
        &data_offer_implementation,
        this,
        &State::handle_drag_offer_destroyed);
    wl_data_device_send_data_offer(data_device_resource, offer);
    for (const auto& payload : payloads) {
      wl_data_offer_send_offer(offer, payload.mime_type.c_str());
    }
    wl_data_offer_send_source_actions(offer, drag_source_actions.load());
    wl_data_offer_send_action(offer, drag_selected_action.load());
  }

  wl_data_device_send_enter(
      data_device_resource,
      next_drag_serial++,
      surface->surface,
      wl_fixed_from_int(drag_x.load()),
      wl_fixed_from_int(drag_y.load()),
      offer);
  wl_display_flush_clients(display);
  drag_entered = true;
  drag_enter_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_clipboard_selection() {
  if (!clipboard_selection_pending.exchange(false)) {
    return;
  }

  if (data_device_resource == nullptr) {
    clipboard_selection_pending.store(true);
    return;
  }

  std::vector<WaylandMimePayload> payloads;
  {
    std::lock_guard lock(clipboard_selection_mutex);
    payloads = clipboard_selection_payloads;
  }
  if (payloads.empty()) {
    wl_data_device_send_selection(data_device_resource, nullptr);
    wl_display_flush_clients(display);
    clipboard_selection_sent.store(true);
    return;
  }

  wl_client* client = wl_resource_get_client(data_device_resource);
  auto* offer = wl_resource_create(
      client,
      &wl_data_offer_interface,
      std::min<std::uint32_t>(wl_resource_get_version(data_device_resource), 3),
      0);
  if (offer == nullptr) {
    clipboard_selection_pending.store(true);
    return;
  }

  clipboard_offer_resource = offer;
  wl_resource_set_implementation(
      offer,
      &data_offer_implementation,
      this,
      &State::handle_clipboard_offer_destroyed);
  wl_data_device_send_data_offer(data_device_resource, offer);
  for (const auto& payload : payloads) {
    wl_data_offer_send_offer(offer, payload.mime_type.c_str());
  }
  wl_data_device_send_selection(data_device_resource, offer);
  wl_display_flush_clients(display);
  clipboard_selection_sent.store(true);
}

void WaylandTestCompositor::State::
    dispatch_pending_clipboard_client_selection_request() {
  if (!clipboard_client_selection_request_pending.exchange(false)) {
    return;
  }

  std::string mime_type;
  {
    std::lock_guard lock(clipboard_client_selection_request_mutex);
    mime_type = pending_clipboard_client_selection_mime;
  }

  ClientDataSource* source = nullptr;
  {
    std::lock_guard lock(clipboard_client_selection_mutex);
    source = clipboard_client_selection_source;
  }
  if (source == nullptr || source->resource == nullptr || mime_type.empty()) {
    clipboard_client_selection_request_pending.store(true);
    return;
  }

  int pipe_fds[2] = {-1, -1};
  if (pipe2(pipe_fds, O_CLOEXEC) == -1) {
    return;
  }

  wl_data_source_send_send(source->resource, mime_type.c_str(), pipe_fds[1]);
  wl_display_flush_clients(display);
  close(pipe_fds[1]);
  pipe_fds[1] = -1;

  std::string payload;
  bool completed = false;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (std::chrono::steady_clock::now() < deadline) {
    pollfd descriptor{
        .fd = pipe_fds[0],
        .events = POLLIN | POLLHUP,
        .revents = 0,
    };
    const int ready = poll(&descriptor, 1, 50);
    if (ready == -1 && errno == EINTR) {
      continue;
    }
    if (ready <= 0) {
      continue;
    }

    char buffer[4096];
    const auto bytes_read = read(pipe_fds[0], buffer, sizeof(buffer));
    if (bytes_read > 0) {
      payload.append(buffer, static_cast<std::size_t>(bytes_read));
      continue;
    }
    completed = bytes_read == 0;
    break;
  }
  close(pipe_fds[0]);

  if (!completed) {
    return;
  }

  {
    std::lock_guard lock(clipboard_client_selection_payload_mutex);
    last_clipboard_client_selection_mime = std::move(mime_type);
    last_clipboard_client_selection_payload_value = std::move(payload);
  }
  clipboard_client_selection_payload_received.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_drag_motion() {
  if (!drag_motion_pending.exchange(false)) {
    return;
  }

  if (data_device_resource == nullptr || !drag_entered) {
    drag_motion_pending.store(true);
    return;
  }

  wl_data_device_send_motion(
      data_device_resource,
      drag_time++,
      wl_fixed_from_int(drag_x.load()),
      wl_fixed_from_int(drag_y.load()));
  wl_display_flush_clients(display);
  drag_motion_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_drag_drop() {
  if (!drag_drop_pending.exchange(false)) {
    return;
  }

  if (data_device_resource == nullptr || !drag_entered) {
    drag_drop_pending.store(true);
    return;
  }

  wl_data_device_send_drop(data_device_resource);
  wl_display_flush_clients(display);
  drag_drop_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_drag_leave() {
  if (!drag_leave_pending.exchange(false)) {
    return;
  }

  if (data_device_resource == nullptr || !drag_entered) {
    drag_leave_pending.store(true);
    return;
  }

  wl_data_device_send_leave(data_device_resource);
  wl_display_flush_clients(display);
  drag_entered = false;
  drag_leave_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_keyboard_modifiers() {
  if (!keyboard_modifiers_pending.exchange(false)) {
    return;
  }

  if (keyboard_resource == nullptr) {
    keyboard_modifiers_pending.store(true);
    return;
  }

  KeyboardModifiersRequest request{};
  {
    std::lock_guard lock(keyboard_modifiers_mutex);
    request = keyboard_modifiers;
  }

  std::uint32_t depressed = 0;
  if (request.shift) {
    depressed |= 1U;
  }
  if (request.control) {
    depressed |= 4U;
  }
  if (request.alt) {
    depressed |= 8U;
  }
  if (request.super) {
    depressed |= 64U;
  }

  wl_keyboard_send_modifiers(
      keyboard_resource,
      next_keyboard_serial++,
      depressed,
      0,
      0,
      0);
  wl_display_flush_clients(display);
  keyboard_modifiers_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_keyboard_key() {
  if (!keyboard_key_pending.exchange(false)) {
    return;
  }

  KeyboardKeyRequest request{};
  {
    std::lock_guard lock(keyboard_key_mutex);
    if (keyboard_keys.empty()) {
      return;
    }
    request = keyboard_keys.front();
    keyboard_keys.pop_front();
    if (!keyboard_keys.empty()) {
      keyboard_key_pending.store(true);
    }
  }

  if (keyboard_resource == nullptr) {
    {
      std::lock_guard lock(keyboard_key_mutex);
      keyboard_keys.push_front(request);
    }
    keyboard_key_pending.store(true);
    return;
  }

  const SurfaceState* surface = first_keyboard_surface();
  if (surface == nullptr) {
    {
      std::lock_guard lock(keyboard_key_mutex);
      keyboard_keys.push_front(request);
    }
    keyboard_key_pending.store(true);
    return;
  }

  if (!keyboard_entered) {
    wl_array keys{};
    wl_array_init(&keys);
    wl_keyboard_send_enter(
        keyboard_resource,
        next_keyboard_serial++,
        surface->surface,
        &keys);
    wl_array_release(&keys);
    keyboard_entered = true;
  }
  wl_keyboard_send_key(
      keyboard_resource,
      next_keyboard_serial++,
      keyboard_time++,
      request.key,
      request.pressed ? WL_KEYBOARD_KEY_STATE_PRESSED
                      : WL_KEYBOARD_KEY_STATE_RELEASED);
  wl_display_flush_clients(display);
  keyboard_key_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_keyboard_leave() {
  if (!keyboard_leave_pending.exchange(false)) {
    return;
  }

  if (keyboard_resource == nullptr) {
    keyboard_leave_pending.store(true);
    return;
  }

  const SurfaceState* surface = first_keyboard_surface();
  if (surface == nullptr || !keyboard_entered) {
    keyboard_leave_pending.store(true);
    return;
  }

  wl_keyboard_send_leave(
      keyboard_resource,
      next_keyboard_serial++,
      surface->surface);
  keyboard_entered = false;
  wl_display_flush_clients(display);
  keyboard_leave_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_text_input_enter() {
  if (!text_input_enter_pending.exchange(false)) {
    return;
  }

  if (text_input_resource == nullptr) {
    text_input_enter_pending.store(true);
    return;
  }

  const SurfaceState* surface = first_keyboard_surface();
  if (surface == nullptr) {
    text_input_enter_pending.store(true);
    return;
  }

  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_enter,
      surface->surface);
  text_input_entered = true;
  wl_display_flush_clients(display);
  text_input_enter_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_text_input_preedit() {
  if (!text_input_preedit_pending.exchange(false)) {
    return;
  }

  if (text_input_resource == nullptr || !text_input_entered) {
    text_input_preedit_pending.store(true);
    return;
  }

  TextInputPreeditRequest request;
  {
    std::lock_guard lock(text_input_event_mutex);
    request = pending_text_input_preedit;
  }
  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_preedit_string,
      request.text.c_str(),
      request.cursor_begin,
      request.cursor_end);
  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_done,
      consume_text_input_serial(request.serial));
  wl_display_flush_clients(display);
  text_input_preedit_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_text_input_delete_surrounding() {
  if (!text_input_delete_surrounding_pending.exchange(false)) {
    return;
  }

  if (text_input_resource == nullptr || !text_input_entered) {
    text_input_delete_surrounding_pending.store(true);
    return;
  }

  TextInputDeleteSurroundingRequest request{};
  {
    std::lock_guard lock(text_input_event_mutex);
    request = pending_text_input_delete_surrounding;
  }
  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_delete_surrounding_text,
      request.before_length,
      request.after_length);
  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_done,
      consume_text_input_serial(request.serial));
  wl_display_flush_clients(display);
  text_input_delete_surrounding_sent.store(true);
}

void WaylandTestCompositor::State::dispatch_pending_text_input_commit() {
  if (!text_input_commit_pending.exchange(false)) {
    return;
  }

  if (text_input_resource == nullptr || !text_input_entered) {
    text_input_commit_pending.store(true);
    return;
  }

  TextInputCommitRequest request;
  {
    std::lock_guard lock(text_input_event_mutex);
    request = pending_text_input_commit;
  }
  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_commit_string,
      request.text.c_str());
  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_done,
      consume_text_input_serial(request.serial));
  wl_display_flush_clients(display);
  text_input_commit_sent.store(true);
}

std::uint32_t WaylandTestCompositor::State::consume_text_input_serial(
    std::optional<std::uint32_t> serial) {
  if (!serial.has_value()) {
    return next_text_input_serial++;
  }

  const std::uint32_t value = *serial;
  if (static_cast<std::int32_t>(value - next_text_input_serial) >= 0) {
    next_text_input_serial = value + 1;
  }
  return value;
}

void WaylandTestCompositor::State::dispatch_pending_text_input_leave() {
  if (!text_input_leave_pending.exchange(false)) {
    return;
  }

  if (text_input_resource == nullptr || !text_input_entered) {
    text_input_leave_pending.store(true);
    return;
  }

  const SurfaceState* surface = first_keyboard_surface();
  if (surface == nullptr) {
    text_input_leave_pending.store(true);
    return;
  }

  wl_resource_post_event(
      text_input_resource,
      zwp_text_input_v3_leave,
      surface->surface);
  text_input_entered = false;
  wl_display_flush_clients(display);
  text_input_leave_sent.store(true);
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
    .set_buffer_scale = &WaylandTestCompositor::State::surface_set_buffer_scale,
    .damage_buffer = noop_surface_damage,
    .offset = noop_surface_offset,
    .get_release = noop_surface_release,
};

const struct wl_output_interface WaylandTestCompositor::State::output_implementation{
    .release = destroy_resource,
};

const struct wl_seat_interface WaylandTestCompositor::State::seat_implementation{
    .get_pointer = &WaylandTestCompositor::State::seat_get_pointer,
    .get_keyboard = &WaylandTestCompositor::State::seat_get_keyboard,
    .get_touch = [](wl_client*, wl_resource*, std::uint32_t) {},
    .release = noop_resource,
};

const struct wl_pointer_interface WaylandTestCompositor::State::pointer_implementation{
    .set_cursor = &WaylandTestCompositor::State::pointer_set_cursor,
    .release = destroy_resource,
};

const struct wl_keyboard_interface WaylandTestCompositor::State::keyboard_implementation{
    .release = destroy_resource,
};

const struct wl_data_device_manager_interface
    WaylandTestCompositor::State::data_device_manager_implementation{
        .create_data_source =
            &WaylandTestCompositor::State::data_device_manager_create_data_source,
        .get_data_device =
            &WaylandTestCompositor::State::data_device_manager_get_data_device,
};

const struct wl_data_device_interface
    WaylandTestCompositor::State::data_device_implementation{
        .start_drag = [](
            wl_client*,
            wl_resource*,
            wl_resource*,
            wl_resource*,
            wl_resource*,
            std::uint32_t) {},
        .set_selection = [](
            wl_client* client,
            wl_resource* resource,
            wl_resource* source,
            std::uint32_t serial) {
          WaylandTestCompositor::State::data_device_set_selection(
              client,
              resource,
              source,
              serial);
        },
        .release = noop_resource,
};

const struct wl_data_offer_interface
    WaylandTestCompositor::State::data_offer_implementation{
        .accept = &WaylandTestCompositor::State::data_offer_accept,
        .receive = &WaylandTestCompositor::State::data_offer_receive,
        .destroy = destroy_resource,
        .finish = &WaylandTestCompositor::State::data_offer_finish,
        .set_actions = &WaylandTestCompositor::State::data_offer_set_actions,
};

const struct wl_data_source_interface
    WaylandTestCompositor::State::data_source_implementation{
        .offer = &WaylandTestCompositor::State::data_source_offer,
        .destroy = &WaylandTestCompositor::State::data_source_destroy,
        .set_actions = &WaylandTestCompositor::State::data_source_set_actions,
};

const WaylandTestCompositor::State::ZwpTextInputManagerV3Implementation
    WaylandTestCompositor::State::text_input_manager_implementation{
        .destroy = destroy_resource,
        .get_text_input =
            &WaylandTestCompositor::State::text_input_manager_get_text_input,
};

const WaylandTestCompositor::State::ZwpTextInputV3Implementation
    WaylandTestCompositor::State::text_input_implementation{
        .destroy = destroy_resource,
        .enable = &WaylandTestCompositor::State::text_input_enable,
        .disable = &WaylandTestCompositor::State::text_input_disable,
        .set_surrounding_text =
            &WaylandTestCompositor::State::text_input_set_surrounding_text,
        .set_text_change_cause =
            &WaylandTestCompositor::State::text_input_set_text_change_cause,
        .set_content_type =
            &WaylandTestCompositor::State::text_input_set_content_type,
        .set_cursor_rectangle =
            &WaylandTestCompositor::State::text_input_set_cursor_rectangle,
        .commit = &WaylandTestCompositor::State::text_input_commit,
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
        .set_parent = [](wl_client*, wl_resource* resource, wl_resource* parent) {
          auto* surface =
              static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
          if (surface != nullptr && parent != nullptr) {
            surface->compositor->parent_requested.store(true);
          }
        },
        .set_title = [](wl_client*, wl_resource*, const char*) {},
        .set_app_id = [](wl_client*, wl_resource*, const char*) {},
        .show_window_menu = [](
            wl_client*, wl_resource*, wl_resource*, std::uint32_t,
            std::int32_t, std::int32_t) {},
        .move = [](wl_client*, wl_resource*, wl_resource*, std::uint32_t) {},
        .resize = [](
            wl_client*, wl_resource*, wl_resource*, std::uint32_t,
            std::uint32_t) {},
        .set_max_size = [](
            wl_client*, wl_resource*, std::int32_t, std::int32_t) {},
        .set_min_size = [](
            wl_client*, wl_resource*, std::int32_t, std::int32_t) {},
        .set_maximized = [](wl_client*, wl_resource* resource) {
          auto* surface =
              static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
          if (surface != nullptr) {
            surface->compositor->maximize_requested.store(true);
          }
        },
        .unset_maximized = [](wl_client*, wl_resource* resource) {
          auto* surface =
              static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
          if (surface != nullptr) {
            surface->compositor->unmaximize_requested.store(true);
          }
        },
        .set_fullscreen = [](
            wl_client*, wl_resource* resource, wl_resource*) {
          auto* surface =
              static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
          if (surface != nullptr) {
            surface->compositor->fullscreen_requested.store(true);
          }
        },
        .unset_fullscreen = [](wl_client*, wl_resource* resource) {
          auto* surface =
              static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
          if (surface != nullptr) {
            surface->compositor->unfullscreen_requested.store(true);
          }
        },
        .set_minimized = [](wl_client*, wl_resource* resource) {
          auto* surface =
              static_cast<SurfaceState*>(wl_resource_get_user_data(resource));
          if (surface != nullptr) {
            surface->compositor->minimize_requested.store(true);
          }
        },
};

const WaylandTestCompositor::State::ZxdgDecorationManagerImplementation
    WaylandTestCompositor::State::decoration_manager_implementation{
        .destroy = destroy_resource,
        .get_toplevel_decoration =
            &WaylandTestCompositor::State::get_toplevel_decoration,
};

const WaylandTestCompositor::State::ZxdgToplevelDecorationImplementation
    WaylandTestCompositor::State::toplevel_decoration_implementation{
        .destroy = destroy_resource,
        .set_mode = &WaylandTestCompositor::State::set_decoration_mode,
        .unset_mode = [](wl_client*, wl_resource*) {},
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

void WaylandTestCompositor::request_resize_configure_state(
    std::int32_t width,
    std::int32_t height,
    bool activated,
    bool maximized,
    bool fullscreen) {
  state_->request_resize_configure_state(
      width,
      height,
      activated,
      maximized,
      fullscreen);
}

void WaylandTestCompositor::request_output_scale(std::int32_t scale) {
  state_->request_output_scale(scale);
}

void WaylandTestCompositor::request_seat_capabilities(
    std::uint32_t capabilities) {
  state_->request_seat_capabilities(capabilities);
}

void WaylandTestCompositor::request_pointer_move(std::int32_t x, std::int32_t y) {
  state_->request_pointer_move(x, y);
}

void WaylandTestCompositor::request_pointer_button(
    std::uint32_t button,
    bool pressed) {
  state_->request_pointer_button(button, pressed);
}

void WaylandTestCompositor::request_pointer_scroll(float delta_x, float delta_y) {
  state_->request_pointer_scroll(delta_x, delta_y);
}

void WaylandTestCompositor::request_drag_enter(std::int32_t x, std::int32_t y) {
  state_->request_drag_enter(x, y);
}

void WaylandTestCompositor::request_drag_motion(std::int32_t x, std::int32_t y) {
  state_->request_drag_motion(x, y);
}

void WaylandTestCompositor::request_drag_drop() {
  state_->request_drag_drop();
}

void WaylandTestCompositor::request_drag_leave() {
  state_->request_drag_leave();
}

void WaylandTestCompositor::set_drag_payloads(
    std::vector<WaylandMimePayload> payloads) {
  state_->set_drag_payloads(std::move(payloads));
}

void WaylandTestCompositor::set_drag_source_actions(
    std::uint32_t source_actions,
    std::uint32_t selected_action) {
  state_->set_drag_source_actions(source_actions, selected_action);
}

void WaylandTestCompositor::request_keyboard_key(
    std::uint32_t key,
    bool pressed) {
  state_->request_keyboard_key(key, pressed);
}

void WaylandTestCompositor::request_keyboard_modifiers(
    bool shift,
    bool control,
    bool alt,
    bool super) {
  state_->request_keyboard_modifiers(shift, control, alt, super);
}

void WaylandTestCompositor::request_keyboard_leave() {
  state_->request_keyboard_leave();
}

void WaylandTestCompositor::request_text_input_enter() {
  state_->request_text_input_enter();
}

void WaylandTestCompositor::request_text_input_preedit(std::string text) {
  state_->request_text_input_preedit(std::move(text));
}

void WaylandTestCompositor::request_text_input_preedit(
    std::string text,
    std::int32_t cursor_begin,
    std::int32_t cursor_end) {
  state_->request_text_input_preedit(
      std::move(text),
      cursor_begin,
      cursor_end);
}

void WaylandTestCompositor::request_text_input_preedit_with_serial(
    std::string text,
    std::int32_t cursor_begin,
    std::int32_t cursor_end,
    std::uint32_t serial) {
  state_->request_text_input_preedit_with_serial(
      std::move(text),
      cursor_begin,
      cursor_end,
      serial);
}

void WaylandTestCompositor::request_text_input_delete_surrounding(
    std::uint32_t before_length,
    std::uint32_t after_length) {
  state_->request_text_input_delete_surrounding(before_length, after_length);
}

void WaylandTestCompositor::request_text_input_commit(std::string text) {
  state_->request_text_input_commit(std::move(text));
}

void WaylandTestCompositor::request_text_input_commit_with_serial(
    std::string text,
    std::uint32_t serial) {
  state_->request_text_input_commit_with_serial(std::move(text), serial);
}

void WaylandTestCompositor::request_text_input_leave() {
  state_->request_text_input_leave();
}

void WaylandTestCompositor::set_clipboard_selection(
    std::vector<WaylandMimePayload> payloads) {
  state_->set_clipboard_selection(std::move(payloads));
}

void WaylandTestCompositor::request_clipboard_client_selection(
    std::string_view mime_type) {
  state_->request_clipboard_client_selection(mime_type);
}

bool WaylandTestCompositor::wait_for_close_sent() const {
  return state_->wait_for_flag(state_->close_sent);
}

bool WaylandTestCompositor::wait_for_minimize_requested() const {
  return state_->wait_for_flag(state_->minimize_requested);
}

bool WaylandTestCompositor::wait_for_maximize_requested() const {
  return state_->wait_for_flag(state_->maximize_requested);
}

bool WaylandTestCompositor::wait_for_unmaximize_requested() const {
  return state_->wait_for_flag(state_->unmaximize_requested);
}

bool WaylandTestCompositor::wait_for_fullscreen_requested() const {
  return state_->wait_for_flag(state_->fullscreen_requested);
}

bool WaylandTestCompositor::wait_for_unfullscreen_requested() const {
  return state_->wait_for_flag(state_->unfullscreen_requested);
}

bool WaylandTestCompositor::wait_for_client_side_decoration_requested() const {
  return state_->wait_for_flag(state_->client_side_decoration_requested);
}

bool WaylandTestCompositor::wait_for_server_side_decoration_requested() const {
  return state_->wait_for_flag(state_->server_side_decoration_requested);
}

bool WaylandTestCompositor::wait_for_parent_requested() const {
  return state_->wait_for_flag(state_->parent_requested);
}

bool WaylandTestCompositor::wait_for_resize_configure_sent() const {
  return state_->wait_for_flag(state_->resize_configure_sent);
}

bool WaylandTestCompositor::wait_for_resize_configure_acked() const {
  return state_->wait_for_flag(state_->resize_configure_acked);
}

WaylandConfigureState WaylandTestCompositor::last_resize_configure_state()
    const {
  return state_->last_resize_configure_state();
}

bool WaylandTestCompositor::wait_for_output_scale_sent() const {
  return state_->wait_for_flag(state_->output_scale_sent);
}

std::int32_t WaylandTestCompositor::last_surface_buffer_scale() const {
  return state_->surface_buffer_scale.load();
}

bool WaylandTestCompositor::wait_for_pointer_bound(bool bound) const {
  return state_->wait_for_flag_value(state_->pointer_bound, bound);
}

bool WaylandTestCompositor::wait_for_keyboard_bound(bool bound) const {
  return state_->wait_for_flag_value(state_->keyboard_bound, bound);
}

bool WaylandTestCompositor::wait_for_pointer_move_sent() const {
  return state_->wait_for_flag(state_->pointer_move_sent);
}

bool WaylandTestCompositor::wait_for_pointer_button_sent() const {
  return state_->wait_for_flag(state_->pointer_button_sent);
}

bool WaylandTestCompositor::wait_for_pointer_scroll_sent() const {
  return state_->wait_for_flag(state_->pointer_scroll_sent);
}

bool WaylandTestCompositor::wait_for_drag_enter_sent() const {
  return state_->wait_for_flag(state_->drag_enter_sent);
}

bool WaylandTestCompositor::wait_for_drag_motion_sent() const {
  return state_->wait_for_flag(state_->drag_motion_sent);
}

bool WaylandTestCompositor::wait_for_drag_drop_sent() const {
  return state_->wait_for_flag(state_->drag_drop_sent);
}

bool WaylandTestCompositor::wait_for_drag_leave_sent() const {
  return state_->wait_for_flag(state_->drag_leave_sent);
}

bool WaylandTestCompositor::wait_for_drag_offer_accepted() const {
  return state_->wait_for_flag(state_->drag_offer_accepted);
}

bool WaylandTestCompositor::wait_for_drag_offer_actions_set() const {
  return state_->wait_for_flag(state_->drag_offer_actions_set);
}

bool WaylandTestCompositor::wait_for_drag_offer_finished() const {
  return state_->wait_for_flag(state_->drag_offer_finished);
}

std::string WaylandTestCompositor::last_drag_accept_mime_type() const {
  std::lock_guard lock(state_->drag_accept_mutex);
  return state_->last_drag_accept_mime;
}

std::uint32_t WaylandTestCompositor::last_drag_offer_actions() const {
  return state_->last_drag_offer_actions_value.load();
}

std::uint32_t WaylandTestCompositor::last_drag_preferred_action() const {
  return state_->last_drag_preferred_action_value.load();
}

bool WaylandTestCompositor::wait_for_pointer_cursor_set() const {
  return state_->wait_for_flag(state_->pointer_cursor_set);
}

bool WaylandTestCompositor::wait_for_pointer_cursor_set_count(
    std::uint32_t count) const {
  return state_->wait_for_cursor_count(count);
}

bool WaylandTestCompositor::wait_for_keyboard_modifiers_sent() const {
  return state_->wait_for_flag(state_->keyboard_modifiers_sent);
}

bool WaylandTestCompositor::wait_for_keyboard_key_sent() const {
  return state_->wait_for_flag(state_->keyboard_key_sent);
}

bool WaylandTestCompositor::wait_for_keyboard_leave_sent() const {
  return state_->wait_for_flag(state_->keyboard_leave_sent);
}

bool WaylandTestCompositor::wait_for_text_input_enter_sent() const {
  return state_->wait_for_flag(state_->text_input_enter_sent);
}

bool WaylandTestCompositor::wait_for_text_input_preedit_sent() const {
  return state_->wait_for_flag(state_->text_input_preedit_sent);
}

bool WaylandTestCompositor::wait_for_text_input_delete_surrounding_sent() const {
  return state_->wait_for_flag(state_->text_input_delete_surrounding_sent);
}

bool WaylandTestCompositor::wait_for_text_input_commit_sent() const {
  return state_->wait_for_flag(state_->text_input_commit_sent);
}

bool WaylandTestCompositor::wait_for_text_input_leave_sent() const {
  return state_->wait_for_flag(state_->text_input_leave_sent);
}

bool WaylandTestCompositor::wait_for_text_input_client_state_committed() const {
  return state_->wait_for_flag(state_->text_input_client_state_committed);
}

WaylandTextInputClientState WaylandTestCompositor::text_input_client_state() const {
  std::lock_guard lock(state_->text_input_client_state_mutex);
  return state_->text_input_client_state;
}

bool WaylandTestCompositor::wait_for_clipboard_selection_sent() const {
  return state_->wait_for_flag(state_->clipboard_selection_sent);
}

std::string WaylandTestCompositor::last_clipboard_receive_mime_type() const {
  std::lock_guard lock(state_->clipboard_receive_mutex);
  return state_->last_clipboard_receive_mime;
}

bool WaylandTestCompositor::wait_for_clipboard_client_selection_set() const {
  return state_->wait_for_flag(state_->clipboard_client_selection_set);
}

std::vector<std::string>
WaylandTestCompositor::clipboard_client_selection_mime_types() const {
  return state_->clipboard_client_selection_mime_types();
}

bool WaylandTestCompositor::
    wait_for_clipboard_client_selection_payload_received() const {
  return state_->wait_for_flag(
      state_->clipboard_client_selection_payload_received);
}

std::string WaylandTestCompositor::last_clipboard_client_selection_payload()
    const {
  return state_->last_clipboard_client_selection_payload();
}

} // namespace cgpui::test
