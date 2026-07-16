#include "x11_scale_internal.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace cgpui {
namespace {

float normalized_scale(float dpi) {
  const float raw = std::clamp(dpi / 96.0F, 0.5F, 4.0F);
  return std::round(raw * 4.0F) / 4.0F;
}

}  // namespace

std::optional<float> x11_parse_xft_dpi(std::string_view resources) {
  constexpr std::string_view key = "Xft.dpi:";
  std::size_t offset = resources.find(key);
  if (offset == std::string_view::npos) return std::nullopt;
  offset += key.size();
  while (offset < resources.size() &&
         (resources[offset] == ' ' || resources[offset] == '\t')) {
    ++offset;
  }
  std::size_t end = offset;
  while (end < resources.size() && resources[end] != '\n' &&
         resources[end] != '\r') {
    ++end;
  }
  const std::string value{resources.substr(offset, end - offset)};
  char* parsed_end = nullptr;
  const float dpi = std::strtof(value.c_str(), &parsed_end);
  if (parsed_end == value.c_str() || !std::isfinite(dpi) || dpi <= 0.0F) {
    return std::nullopt;
  }
  return dpi;
}

DpiScale x11_display_scale(
    xcb_connection_t* connection,
    xcb_screen_t* screen) {
  constexpr char resource_manager[] = "RESOURCE_MANAGER";
  const auto atom_cookie = xcb_intern_atom(
      connection, true, sizeof(resource_manager) - 1, resource_manager);
  xcb_intern_atom_reply_t* atom_reply =
      xcb_intern_atom_reply(connection, atom_cookie, nullptr);
  if (atom_reply != nullptr && atom_reply->atom != XCB_ATOM_NONE) {
    const auto property_cookie = xcb_get_property(
        connection, false, screen->root, atom_reply->atom,
        XCB_GET_PROPERTY_TYPE_ANY, 0, 256 * 1024);
    xcb_get_property_reply_t* property =
        xcb_get_property_reply(connection, property_cookie, nullptr);
    std::free(atom_reply);
    if (property != nullptr) {
      const auto* bytes = static_cast<const char*>(xcb_get_property_value(property));
      const std::string_view resources{
          bytes, static_cast<std::size_t>(xcb_get_property_value_length(property))};
      const auto dpi = x11_parse_xft_dpi(resources);
      std::free(property);
      if (dpi) return DpiScale{normalized_scale(*dpi)};
    }
  } else {
    std::free(atom_reply);
  }
  if (screen->width_in_millimeters != 0) {
    const float dpi = static_cast<float>(screen->width_in_pixels) * 25.4F /
        static_cast<float>(screen->width_in_millimeters);
    return DpiScale{normalized_scale(dpi)};
  }
  return DpiScale{1.0F};
}

}  // namespace cgpui
