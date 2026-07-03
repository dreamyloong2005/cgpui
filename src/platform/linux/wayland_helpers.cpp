#include "wayland_internal.hpp"

namespace cgpui {
namespace {

constexpr std::uint32_t linux_button_left = 0x110;
constexpr std::uint32_t linux_button_right = 0x111;
constexpr std::uint32_t linux_button_middle = 0x112;

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

} // namespace

Error wayland_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

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

} // namespace cgpui
