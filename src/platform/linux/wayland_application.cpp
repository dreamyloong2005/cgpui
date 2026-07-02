#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
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
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;

namespace {

extern const wl_interface xdg_positioner_interface;
extern const wl_interface xdg_wm_base_interface;
extern const wl_interface xdg_surface_interface;
extern const wl_interface xdg_toplevel_interface;
extern const wl_interface zwp_text_input_manager_v3_interface;
extern const wl_interface zwp_text_input_v3_interface;

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

struct zwp_text_input_v3_listener {
  void (*enter)(void* data, zwp_text_input_v3* text_input, wl_surface* surface);
  void (*leave)(void* data, zwp_text_input_v3* text_input, wl_surface* surface);
  void (*preedit_string)(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end);
  void (*commit_string)(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text);
  void (*delete_surrounding_text)(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t before_length,
      std::uint32_t after_length);
  void (*done)(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t serial);
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

void zwp_text_input_manager_v3_destroy(zwp_text_input_manager_v3* manager) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)),
      WL_MARSHAL_FLAG_DESTROY);
}

zwp_text_input_v3* zwp_text_input_manager_v3_get_text_input(
    zwp_text_input_manager_v3* manager,
    wl_seat* seat) {
  wl_proxy* proxy = wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager),
      1,
      &zwp_text_input_v3_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)),
      0,
      nullptr,
      seat);
  return reinterpret_cast<zwp_text_input_v3*>(proxy);
}

int zwp_text_input_v3_add_listener(
    zwp_text_input_v3* text_input,
    const zwp_text_input_v3_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(text_input),
      reinterpret_cast<void (**)(void)>(
          const_cast<zwp_text_input_v3_listener*>(listener)),
      data);
}

void zwp_text_input_v3_destroy(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      WL_MARSHAL_FLAG_DESTROY);
}

void zwp_text_input_v3_enable(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      1,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0);
}

void zwp_text_input_v3_disable(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      2,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0);
}

void zwp_text_input_v3_set_surrounding_text(
    zwp_text_input_v3* text_input,
    const char* text,
    std::int32_t cursor,
    std::int32_t anchor) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      3,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0,
      text,
      cursor,
      anchor);
}

void zwp_text_input_v3_set_content_type(
    zwp_text_input_v3* text_input,
    std::uint32_t hint,
    std::uint32_t purpose) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      5,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0,
      hint,
      purpose);
}

void zwp_text_input_v3_set_cursor_rectangle(
    zwp_text_input_v3* text_input,
    std::int32_t x,
    std::int32_t y,
    std::int32_t width,
    std::int32_t height) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      6,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0,
      x,
      y,
      width,
      height);
}

void zwp_text_input_v3_commit(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      7,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0);
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

std::optional<int> hex_digit(char value) {
  if (value >= '0' && value <= '9') {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f') {
    return value - 'a' + 10;
  }
  if (value >= 'A' && value <= 'F') {
    return value - 'A' + 10;
  }
  return std::nullopt;
}

std::string percent_decode(std::string_view value) {
  std::string result;
  result.reserve(value.size());
  for (std::size_t index = 0; index < value.size(); ++index) {
    if (value[index] == '%' && index + 2 < value.size()) {
      const auto high = hex_digit(value[index + 1]);
      const auto low = hex_digit(value[index + 2]);
      if (high.has_value() && low.has_value()) {
        result.push_back(static_cast<char>((*high << 4) | *low));
        index += 2;
        continue;
      }
    }
    result.push_back(value[index]);
  }
  return result;
}

std::optional<std::string> path_from_file_uri(std::string_view uri) {
  constexpr std::string_view prefix = "file://";
  if (!uri.starts_with(prefix)) {
    return std::nullopt;
  }

  std::string_view path = uri.substr(prefix.size());
  constexpr std::string_view localhost = "localhost/";
  if (path.starts_with(localhost)) {
    path.remove_prefix(localhost.size() - 1U);
  }
  if (!path.starts_with('/')) {
    return std::nullopt;
  }
  return percent_decode(path);
}

std::vector<std::string> parse_uri_list(std::string_view payload) {
  std::vector<std::string> files;
  while (!payload.empty()) {
    auto line_end = payload.find('\n');
    std::string_view line =
        line_end == std::string_view::npos ? payload : payload.substr(0, line_end);
    if (!line.empty() && line.back() == '\r') {
      line.remove_suffix(1);
    }
    if (!line.empty() && line.front() != '#') {
      if (auto path = path_from_file_uri(line); path.has_value()) {
        files.push_back(std::move(*path));
      }
    }
    if (line_end == std::string_view::npos) {
      break;
    }
    payload.remove_prefix(line_end + 1U);
  }
  return files;
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

class WaylandTextInputState {
 public:
  void set_available(bool available) {
    available_ = available;
  }

  [[nodiscard]] ImeTextInputSupport support() const {
    return available_ ? ImeTextInputSupport::available
                      : ImeTextInputSupport::unsupported;
  }

  void set_placement(std::optional<ImeTextInputPlacement> placement) {
    placement_ = placement;
  }

  [[nodiscard]] std::optional<ImeTextInputPlacement> placement() const {
    return placement_;
  }

  void enter() {
    entered_ = true;
  }

  std::optional<ImeComposition> leave(KeyboardModifiers modifiers) {
    entered_ = false;
    if (!preedit_text_.empty()) {
      preedit_text_.clear();
      return ImeComposition{
          .phase = ImeCompositionPhase::cancel,
          .text = {},
          .modifiers = modifiers};
    }
    return std::nullopt;
  }

  void set_surrounding_text(
      std::string surrounding_text,
      std::int32_t cursor,
      std::int32_t anchor) {
    surrounding_text_ = std::move(surrounding_text);
    surrounding_text_cursor_ = cursor;
    surrounding_text_anchor_ = anchor;
  }

  void set_content_type(std::uint32_t hint, std::uint32_t purpose) {
    content_type_hint_ = hint;
    content_type_purpose_ = purpose;
  }

  [[nodiscard]] ImeComposition preedit(
      std::string text,
      KeyboardModifiers modifiers) {
    preedit_text_ = std::move(text);
    return ImeComposition{
        .phase = ImeCompositionPhase::update,
        .text = preedit_text_,
        .modifiers = modifiers};
  }

  [[nodiscard]] ImeComposition commit(
      std::string text,
      KeyboardModifiers modifiers) {
    preedit_text_.clear();
    committed_text_ = std::move(text);
    return ImeComposition{
        .phase = ImeCompositionPhase::commit,
        .text = committed_text_,
        .modifiers = modifiers};
  }

 private:
  bool available_ = false;
  bool entered_ = false;
  std::optional<ImeTextInputPlacement> placement_;
  std::string surrounding_text_;
  std::int32_t surrounding_text_cursor_ = 0;
  std::int32_t surrounding_text_anchor_ = 0;
  std::uint32_t content_type_hint_ = 0;
  std::uint32_t content_type_purpose_ = 0;
  std::string preedit_text_;
  std::string committed_text_;
};

class WaylandWindow final : public PlatformWindow {
 public:
  static Result<std::unique_ptr<WaylandWindow>> create(
      wl_display* display,
      wl_compositor* compositor,
      xdg_wm_base* shell,
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback,
      bool text_input_available) {
    auto window = std::unique_ptr<WaylandWindow>(
        new WaylandWindow(display, std::move(callback), WindowState{
            .framebuffer_size = descriptor.size,
            .scale = DpiScale{1.0F},
            .close_requested = false,
        }));
    window->set_text_input_available(text_input_available);

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
    text_input_state_.set_placement(placement);
    sync_text_input_state();
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

  void text_input_entered() {
    text_input_state_.enter();
  }

  void text_input_left(KeyboardModifiers modifiers) {
    if (auto event = text_input_state_.leave(modifiers); event.has_value()) {
      callback_(*event);
    }
  }

  void text_input_preedit(std::string text, KeyboardModifiers modifiers) {
    callback_(text_input_state_.preedit(std::move(text), modifiers));
  }

  void text_input_commit(std::string text, KeyboardModifiers modifiers) {
    callback_(text_input_state_.commit(std::move(text), modifiers));
  }

  void text_input_surrounding_text(
      std::string text,
      std::int32_t cursor,
      std::int32_t anchor) {
    text_input_state_.set_surrounding_text(std::move(text), cursor, anchor);
  }

  void text_input_content_type(std::uint32_t hint, std::uint32_t purpose) {
    text_input_state_.set_content_type(hint, purpose);
  }

  void set_text_input_available(bool available) {
    text_input_state_.set_available(available);
    sync_text_input_state();
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

  void sync_text_input_state() {
    state_.ime_text_input_support = text_input_state_.support();
    state_.ime_text_input_placement = text_input_state_.placement();
  }

  wl_display* display_ = nullptr;
  wl_surface* surface_ = nullptr;
  xdg_surface* xdg_surface_ = nullptr;
  xdg_toplevel* toplevel_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
  WaylandTextInputState text_input_state_;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  bool configured_ = false;
  bool resize_pending_surface_configure_ = false;
};

class WaylandTextInput {
 public:
  using WindowLookup = std::function<WaylandWindow*(wl_surface*)>;
  using ModifiersProvider = std::function<KeyboardModifiers()>;

  void set_display(wl_display* display) {
    display_ = display;
  }

  void set_manager(zwp_text_input_manager_v3* manager) {
    manager_ = manager;
  }

  void set_window_lookup(WindowLookup lookup) {
    find_window_ = std::move(lookup);
  }

  void set_modifiers_provider(ModifiersProvider provider) {
    modifiers_ = std::move(provider);
  }

  void bind_to_seat(wl_seat* seat) {
    if (manager_ == nullptr || seat == nullptr || text_input_ != nullptr) {
      return;
    }

    text_input_ = zwp_text_input_manager_v3_get_text_input(manager_, seat);
    if (text_input_ == nullptr) {
      return;
    }

    static const zwp_text_input_v3_listener listener{
        .enter = &WaylandTextInput::handle_enter,
        .leave = &WaylandTextInput::handle_leave,
        .preedit_string = &WaylandTextInput::handle_preedit_string,
        .commit_string = &WaylandTextInput::handle_commit_string,
        .delete_surrounding_text =
            &WaylandTextInput::handle_delete_surrounding_text,
        .done = &WaylandTextInput::handle_done,
    };
    zwp_text_input_v3_add_listener(text_input_, &listener, this);
  }

  void reset_text_input() {
    active_window_ = nullptr;
    pending_preedit_.reset();
    pending_commit_.reset();
    if (text_input_ != nullptr) {
      zwp_text_input_v3_destroy(text_input_);
      text_input_ = nullptr;
    }
  }

  [[nodiscard]] bool available() const {
    return text_input_ != nullptr;
  }

  void apply_placement(WaylandWindow& window) {
    const auto placement = window.state().ime_text_input_placement;
    if (placement.has_value()) {
      const std::int32_t cursor =
          static_cast<std::int32_t>(placement->byte_offset);
      window.text_input_surrounding_text({}, cursor, cursor);
      window.text_input_content_type(0, 0);
    }
    if (text_input_ == nullptr) {
      return;
    }

    if (placement.has_value()) {
      const auto& rect = placement->rect;
      const std::int32_t cursor =
          static_cast<std::int32_t>(placement->byte_offset);
      const std::string surrounding_text;
      zwp_text_input_v3_enable(text_input_);
      zwp_text_input_v3_set_surrounding_text(
          text_input_,
          surrounding_text.c_str(),
          cursor,
          cursor);
      zwp_text_input_v3_set_content_type(text_input_, 0, 0);
      zwp_text_input_v3_set_cursor_rectangle(
          text_input_,
          static_cast<std::int32_t>(rect.origin.x),
          static_cast<std::int32_t>(rect.origin.y),
          static_cast<std::int32_t>(rect.size.width),
          static_cast<std::int32_t>(rect.size.height));
    } else {
      zwp_text_input_v3_disable(text_input_);
    }
    zwp_text_input_v3_commit(text_input_);
    if (display_ != nullptr) {
      (void)wl_display_flush(display_);
    }
  }

 private:
  static void handle_enter(
      void* data,
      zwp_text_input_v3* text_input,
      wl_surface* surface) {
    (void)text_input;
    auto* self = static_cast<WaylandTextInput*>(data);
    self->active_window_ = self->find_window_ ? self->find_window_(surface) : nullptr;
    if (self->active_window_ != nullptr) {
      self->active_window_->text_input_entered();
    }
  }

  static void handle_leave(
      void* data,
      zwp_text_input_v3* text_input,
      wl_surface* surface) {
    (void)text_input;
    auto* self = static_cast<WaylandTextInput*>(data);
    WaylandWindow* window =
        self->find_window_ ? self->find_window_(surface) : self->active_window_;
    if (window != nullptr) {
      window->text_input_left(self->current_modifiers());
    }
    if (window == self->active_window_) {
      self->active_window_ = nullptr;
    }
    self->pending_preedit_.reset();
    self->pending_commit_.reset();
  }

  static void handle_preedit_string(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end) {
    (void)text_input;
    (void)cursor_begin;
    (void)cursor_end;
    auto* self = static_cast<WaylandTextInput*>(data);
    self->pending_preedit_ = text == nullptr ? std::string{} : std::string(text);
  }

  static void handle_commit_string(
      void* data,
      zwp_text_input_v3* text_input,
      const char* text) {
    (void)text_input;
    auto* self = static_cast<WaylandTextInput*>(data);
    self->pending_commit_ = text == nullptr ? std::string{} : std::string(text);
  }

  static void handle_delete_surrounding_text(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t before_length,
      std::uint32_t after_length) {
    (void)data;
    (void)text_input;
    (void)before_length;
    (void)after_length;
  }

  static void handle_done(
      void* data,
      zwp_text_input_v3* text_input,
      std::uint32_t serial) {
    (void)text_input;
    (void)serial;
    auto* self = static_cast<WaylandTextInput*>(data);
    if (self->active_window_ == nullptr) {
      self->pending_preedit_.reset();
      self->pending_commit_.reset();
      return;
    }

    const KeyboardModifiers modifiers = self->current_modifiers();
    if (self->pending_preedit_.has_value()) {
      self->active_window_->text_input_preedit(
          std::move(*self->pending_preedit_),
          modifiers);
    }
    if (self->pending_commit_.has_value()) {
      self->active_window_->text_input_commit(
          std::move(*self->pending_commit_),
          modifiers);
    }
    self->pending_preedit_.reset();
    self->pending_commit_.reset();
  }

  [[nodiscard]] KeyboardModifiers current_modifiers() const {
    return modifiers_ ? modifiers_() : KeyboardModifiers{};
  }

  wl_display* display_ = nullptr;
  zwp_text_input_manager_v3* manager_ = nullptr;
  zwp_text_input_v3* text_input_ = nullptr;
  WaylandWindow* active_window_ = nullptr;
  WindowLookup find_window_;
  ModifiersProvider modifiers_;
  std::optional<std::string> pending_preedit_;
  std::optional<std::string> pending_commit_;
};

class WaylandDataDevice {
 public:
  using WindowLookup = std::function<WaylandWindow*(wl_surface*)>;

  void set_display(wl_display* display) {
    display_ = display;
  }

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
    clear_offer(pending_offer_);
    drag_window_ = nullptr;
    last_drag_position_ = {};
    if (data_device_ != nullptr) {
      wl_data_device_destroy(data_device_);
      data_device_ = nullptr;
    }
  }

 private:
  struct Offer {
    wl_data_offer* offer = nullptr;
    std::vector<std::string> mime_types;
  };

  static void handle_data_offer(
      void* data,
      wl_data_device* data_device,
      wl_data_offer* offer) {
    (void)data_device;
    auto* self = static_cast<WaylandDataDevice*>(data);
    self->clear_offer(self->pending_offer_);
    self->pending_offer_ = std::make_unique<Offer>();
    self->pending_offer_->offer = offer;
    static const wl_data_offer_listener offer_listener{
        .offer = &WaylandDataDevice::handle_offer_mime_type,
        .source_actions = &WaylandDataDevice::handle_offer_source_actions,
        .action = &WaylandDataDevice::handle_offer_action,
    };
    wl_data_offer_add_listener(
        offer,
        &offer_listener,
        self->pending_offer_.get());
  }

  static void handle_offer_mime_type(
      void* data,
      wl_data_offer*,
      const char* mime_type) {
    auto* offer = static_cast<Offer*>(data);
    if (offer != nullptr && mime_type != nullptr) {
      offer->mime_types.emplace_back(mime_type);
    }
  }

  static void handle_offer_source_actions(
      void*,
      wl_data_offer*,
      std::uint32_t) {}

  static void handle_offer_action(void*, wl_data_offer*, std::uint32_t) {}

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
      if (self->pending_offer_ != nullptr &&
          self->pending_offer_->offer == offer) {
        self->clear_offer(self->pending_offer_);
      } else {
        wl_data_offer_destroy(offer);
      }
    }
  }

  void replace_active_offer(wl_data_offer* offer) {
    if (active_offer_ != nullptr && active_offer_->offer != offer) {
      clear_offer(active_offer_);
    }
    if (offer == nullptr) {
      return;
    }
    if (pending_offer_ != nullptr && pending_offer_->offer == offer) {
      active_offer_ = std::move(pending_offer_);
    } else if (active_offer_ == nullptr) {
      active_offer_ = std::make_unique<Offer>();
      active_offer_->offer = offer;
    }
  }

  void clear_active_offer() {
    clear_offer(active_offer_);
  }

  void clear_offer(std::unique_ptr<Offer>& offer) {
    if (offer != nullptr && offer->offer != nullptr) {
      wl_data_offer_destroy(offer->offer);
    }
    offer.reset();
  }

  [[nodiscard]] std::optional<std::string> read_offer_payload(
      const std::string& mime_type) {
    if (active_offer_ == nullptr || active_offer_->offer == nullptr ||
        display_ == nullptr) {
      return std::nullopt;
    }

    int pipe_fds[2] = {-1, -1};
    if (pipe2(pipe_fds, O_CLOEXEC) == -1) {
      return std::nullopt;
    }

    wl_data_offer_receive(active_offer_->offer, mime_type.c_str(), pipe_fds[1]);
    if (wl_display_flush(display_) == -1) {
      close(pipe_fds[0]);
      close(pipe_fds[1]);
      return std::nullopt;
    }

    close(pipe_fds[1]);
    pipe_fds[1] = -1;

    std::string payload;
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
      close(pipe_fds[0]);
      return bytes_read == 0 ? std::optional<std::string>{std::move(payload)}
                             : std::nullopt;
    }

    close(pipe_fds[0]);
    return std::nullopt;
  }

  [[nodiscard]] bool active_offer_has_mime(std::string_view mime_type) const {
    if (active_offer_ == nullptr) {
      return false;
    }
    return std::ranges::find_if(
               active_offer_->mime_types,
               [mime_type](const std::string& candidate) {
                 return candidate == mime_type;
               }) != active_offer_->mime_types.end();
  }

  [[nodiscard]] DragDropPayload payload_from_active_offer() {
    if (active_offer_has_mime("text/plain;charset=utf-8")) {
      if (auto text = read_offer_payload("text/plain;charset=utf-8");
          text.has_value()) {
        return DragDropPayload{
            .kind = DragDropPayloadKind::text,
            .text = std::move(*text),
        };
      }
    }
    if (active_offer_has_mime("text/plain")) {
      if (auto text = read_offer_payload("text/plain"); text.has_value()) {
        return DragDropPayload{
            .kind = DragDropPayloadKind::text,
            .text = std::move(*text),
        };
      }
    }
    if (active_offer_has_mime("text/uri-list")) {
      if (auto uri_list = read_offer_payload("text/uri-list");
          uri_list.has_value()) {
        auto files = parse_uri_list(*uri_list);
        if (!files.empty()) {
          return DragDropPayload{
              .kind = DragDropPayloadKind::files,
              .files = std::move(files),
          };
        }
      }
    }
    return {};
  }

  wl_display* display_ = nullptr;
  wl_data_device_manager* manager_ = nullptr;
  wl_data_device* data_device_ = nullptr;
  std::unique_ptr<Offer> pending_offer_;
  std::unique_ptr<Offer> active_offer_;
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
    data_device_.set_display(display_);
    text_input_.set_display(display_);
    if (pipe2(wakeup_pipe_, O_NONBLOCK | O_CLOEXEC) == -1) {
      initialization_error_ = "pipe2 failed while creating event-loop wakeup pipe";
      return;
    }
    configure_data_device_lookup();
    configure_text_input_lookup();

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
      text_input_.bind_to_seat(seat_);
      if (wl_display_roundtrip(display_) == -1) {
        initialization_error_ = "wl_display_roundtrip failed while waiting for seat";
      }
    }
  }

  ~WaylandApplication() override {
    text_input_.reset_text_input();
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
    if (text_input_manager_ != nullptr) {
      zwp_text_input_manager_v3_destroy(text_input_manager_);
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
        display_,
        compositor_,
        shell_,
        descriptor,
        std::move(callback),
        text_input_.available());
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

  void configure_text_input_lookup() {
    text_input_.set_window_lookup(
        [this](wl_surface* surface) { return find_window(surface); });
    text_input_.set_modifiers_provider([this] { return keyboard_modifiers(); });
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
    if (interface_name == zwp_text_input_manager_v3_interface.name) {
      app->text_input_manager_ =
          static_cast<zwp_text_input_manager_v3*>(wl_registry_bind(
              registry,
              name,
              &zwp_text_input_manager_v3_interface,
              std::min<std::uint32_t>(version, 1)));
      app->text_input_.set_manager(app->text_input_manager_);
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
      app->text_input_.bind_to_seat(seat);
    } else {
      app->data_device_.reset_device();
      app->text_input_.reset_text_input();
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

  void set_window_ime_text_input_placement(
      WaylandWindow& window,
      std::optional<ImeTextInputPlacement> placement) {
    window.set_ime_text_input_placement(placement);
    text_input_.apply_placement(window);
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
      app_.set_window_ime_text_input_placement(*window_, placement);
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
  zwp_text_input_manager_v3* text_input_manager_ = nullptr;
  wl_pointer* pointer_ = nullptr;
  wl_keyboard* keyboard_ = nullptr;
  WaylandDataDevice data_device_;
  WaylandTextInput text_input_;
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
