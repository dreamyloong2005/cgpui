#include "wayland_pointer_scroll_frame_internal.hpp"

#include <wayland-client-protocol.h>

namespace cgpui {
namespace {

void add_axis_value(
    std::int32_t& x,
    std::int32_t& y,
    std::uint32_t axis,
    std::int32_t value) {
  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
    y += value;
  } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
    x += value;
  }
}

bool high_resolution_wheel(const WaylandPointerScrollFrameState& state) {
  const std::int32_t x = state.value120_x != 0
      ? state.value120_x
      : state.discrete_x * 120;
  const std::int32_t y = state.value120_y != 0
      ? state.value120_y
      : state.discrete_y * 120;
  return (x != 0 && x % 120 != 0) || (y != 0 && y % 120 != 0);
}

} // namespace

void wayland_pointer_scroll_axis(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis,
    float delta) {
  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
    state.delta.y += delta;
  } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
    state.delta.x += delta;
  } else {
    return;
  }
  state.pending = true;
}

void wayland_pointer_scroll_source(
    WaylandPointerScrollFrameState& state,
    std::uint32_t source) {
  state.source = source;
}

void wayland_pointer_scroll_stop(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis) {
  if (axis <= WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
    state.stopped_axes |= static_cast<std::uint8_t>(1U << axis);
  }
}

void wayland_pointer_scroll_discrete(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis,
    std::int32_t discrete) {
  add_axis_value(state.discrete_x, state.discrete_y, axis, discrete);
}

void wayland_pointer_scroll_value120(
    WaylandPointerScrollFrameState& state,
    std::uint32_t axis,
    std::int32_t value120) {
  add_axis_value(state.value120_x, state.value120_y, axis, value120);
}

WaylandPointerScrollFrameResult wayland_pointer_scroll_consume(
    WaylandPointerScrollFrameState& state) {
  const bool continuous = state.source == WL_POINTER_AXIS_SOURCE_FINGER ||
      state.source == WL_POINTER_AXIS_SOURCE_CONTINUOUS;
  const WaylandPointerScrollFrameResult result{
      .delta = state.delta,
      .precise = continuous || high_resolution_wheel(state),
      .publish = state.pending,
  };
  state = {};
  return result;
}

} // namespace cgpui
