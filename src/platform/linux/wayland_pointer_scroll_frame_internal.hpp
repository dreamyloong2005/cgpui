#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>
#include <limits>

namespace cgpui {

struct WaylandPointerScrollFrameState {
  Point delta{};
  std::int32_t value120_x = 0;
  std::int32_t value120_y = 0;
  std::int32_t discrete_x = 0;
  std::int32_t discrete_y = 0;
  std::uint32_t source = std::numeric_limits<std::uint32_t>::max();
  std::uint8_t stopped_axes = 0;
  bool pending = false;
};

struct WaylandPointerScrollFrameResult {
  Point delta{};
  bool precise = false;
  bool publish = false;
};

void wayland_pointer_scroll_axis(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis,
    float delta);
void wayland_pointer_scroll_source(
    WaylandPointerScrollFrameState& state,
    std::uint32_t source);
void wayland_pointer_scroll_stop(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis);
void wayland_pointer_scroll_discrete(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis,
    std::int32_t discrete);
void wayland_pointer_scroll_value120(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis,
    std::int32_t value120);
WaylandPointerScrollFrameResult wayland_pointer_scroll_consume(
    WaylandPointerScrollFrameState& state);

} // namespace cgpui
