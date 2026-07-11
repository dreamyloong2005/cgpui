#include "wayland_cursor_theme_internal.hpp"

#include <wayland-client.h>
#include <wayland-cursor.h>

#include <algorithm>
#include <charconv>
#include <climits>
#include <cstdlib>

namespace cgpui {
namespace {

int cursor_base_size() {
  constexpr int default_size = 24;
  const char* value = std::getenv("XCURSOR_SIZE");
  if (value == nullptr) return default_size;
  int size = 0;
  const char* end = value;
  while (*end != '\0') ++end;
  const auto parsed = std::from_chars(value, end, size);
  return parsed.ec == std::errc{} && parsed.ptr == end
      ? std::clamp(size, 1, 256)
      : default_size;
}

std::string cursor_theme_name() {
  const char* value = std::getenv("XCURSOR_THEME");
  return value == nullptr ? std::string{} : std::string{value};
}

} // namespace

class WaylandCursorThemeResources {
 public:
  WaylandCursorThemeResources(wl_compositor* compositor, wl_shm* shm)
      : shm_(shm), surface_(compositor == nullptr
            ? nullptr
            : wl_compositor_create_surface(compositor)),
        theme_name_(cursor_theme_name()), base_size_(cursor_base_size()) {}

  ~WaylandCursorThemeResources() {
    if (theme_ != nullptr) wl_cursor_theme_destroy(theme_);
    if (surface_ != nullptr) wl_surface_destroy(surface_);
  }

  bool load(std::int32_t scale) {
    scale = std::max(scale, 1);
    if (theme_ != nullptr && loaded_scale_ == scale) return true;
    if (theme_ != nullptr) wl_cursor_theme_destroy(theme_);
    theme_ = wl_cursor_theme_load(
        theme_name_.empty() ? nullptr : theme_name_.c_str(),
        base_size_ * scale,
        shm_);
    loaded_scale_ = theme_ == nullptr ? 0 : scale;
    return theme_ != nullptr;
  }

  wl_shm* shm_ = nullptr;
  wl_surface* surface_ = nullptr;
  wl_cursor_theme* theme_ = nullptr;
  std::string theme_name_;
  int base_size_ = 24;
  std::int32_t loaded_scale_ = 0;
};

void WaylandCursorThemeResourcesDeleter::operator()(
    WaylandCursorThemeResources* resources) const {
  delete resources;
}

WaylandCursorThemeResourcesPtr create_wayland_cursor_theme(
    wl_compositor* compositor,
    wl_shm* shm) {
  if (compositor == nullptr || shm == nullptr) return {};
  auto resources = WaylandCursorThemeResourcesPtr(
      new WaylandCursorThemeResources(compositor, shm));
  return resources->surface_ == nullptr ? WaylandCursorThemeResourcesPtr{}
                                        : std::move(resources);
}

WaylandCursorThemeApplyResult apply_wayland_cursor_theme(
    WaylandCursorThemeResources& resources,
    wl_pointer* pointer,
    std::uint32_t serial,
    std::string_view cursor_name,
    std::int32_t buffer_scale) {
  WaylandCursorThemeApplyResult result{
      .buffer_scale = std::max(buffer_scale, 1)};
  if (pointer == nullptr || resources.surface_ == nullptr ||
      !resources.load(result.buffer_scale)) {
    result.unavailable_reason = "cursor theme unavailable";
    return result;
  }
  result.theme_loaded = true;
  std::string name(cursor_name);
  wl_cursor* cursor = wl_cursor_theme_get_cursor(
      resources.theme_, name.c_str());
  if (cursor == nullptr && name != "left_ptr") {
    cursor = wl_cursor_theme_get_cursor(resources.theme_, "left_ptr");
  }
  if (cursor == nullptr || cursor->image_count == 0) {
    result.unavailable_reason = "cursor theme image unavailable";
    return result;
  }
  wl_cursor_image* image = cursor->images[0];
  wl_buffer* buffer = wl_cursor_image_get_buffer(image);
  if (buffer == nullptr) {
    result.unavailable_reason = "cursor image buffer unavailable";
    return result;
  }
  result.image_ready = true;
  result.hotspot_x = static_cast<std::int32_t>(image->hotspot_x) /
      result.buffer_scale;
  result.hotspot_y = static_cast<std::int32_t>(image->hotspot_y) /
      result.buffer_scale;
  wl_pointer_set_cursor(
      pointer, serial, resources.surface_, result.hotspot_x, result.hotspot_y);
  wl_surface_set_buffer_scale(resources.surface_, result.buffer_scale);
  wl_surface_attach(resources.surface_, buffer, 0, 0);
  wl_surface_damage_buffer(
      resources.surface_, 0, 0,
      static_cast<std::int32_t>(image->width),
      static_cast<std::int32_t>(image->height));
  wl_surface_commit(resources.surface_);
  result.applied = true;
  return result;
}

} // namespace cgpui
