#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <variant>

namespace {

bool point_equals(cgpui::Point lhs, cgpui::Point rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

} // namespace

int main() {
  constexpr cgpui::Point initial_position{160.0F, 120.0F};
  constexpr cgpui::Point requested_position{240.0F, 180.0F};
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  bool moved_state_matched = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Win32 Window Position Test",
          .size = cgpui::Size{420.0F, 280.0F},
          .position = initial_position},
      [&](const cgpui::PlatformEvent& event) {
        const auto* moved = std::get_if<cgpui::WindowMoved>(&event);
        if (moved == nullptr || observed_window == nullptr) {
          return;
        }
        const auto state = observed_window->position_state();
        moved_state_matched = state.supported && state.position.has_value() &&
            point_equals(*state.position, moved->position);
      });
  if (!window) {
    return 2;
  }
  observed_window = window->get();

  const auto initial = (*window)->position_state();
  if (!initial.supported || !initial.position.has_value() ||
      !point_equals(*initial.position, initial_position)) {
    return 3;
  }
  if (!(*window)->request_position(requested_position)) {
    return 4;
  }
  const auto moved = (*window)->position_state();
  if (!moved.supported || !moved.position.has_value() ||
      !point_equals(*moved.position, requested_position) ||
      !moved_state_matched) {
    return 5;
  }
  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::fullscreen) ||
      (*window)->request_position(initial_position)) {
    return 6;
  }
  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::normal)) {
    return 7;
  }
  return 0;
}
