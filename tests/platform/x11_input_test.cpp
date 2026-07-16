#include "cgpui/platform/platform.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <variant>
#include <xcb/xcb.h>

namespace {
bool wait_for(const std::atomic_int& count, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return count.load() >= expected;
}
}  // namespace

int main() {
  if (std::getenv("DISPLAY") == nullptr) return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");
  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  std::atomic_int matched{0};
  auto window = (*app)->create_window(
      {.title = "CGPUI X11 Input", .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* moved = std::get_if<cgpui::PointerMoved>(&event);
            moved && moved->position.x == 21.0F && moved->position.y == 34.0F) {
          matched.fetch_add(1);
        } else if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
                   button && button->button == cgpui::MouseButton::left &&
                   button->pressed) {
          matched.fetch_add(1);
        } else if (const auto* scroll = std::get_if<cgpui::PointerScrolled>(&event);
                   scroll && scroll->delta.y == 1.0F) {
          matched.fetch_add(1);
        } else if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
                   key && key->key_code == 30 &&
                   key->action == cgpui::KeyAction::pressed) {
          matched.fetch_add(1);
        } else if (const auto* text = std::get_if<cgpui::TextInput>(&event);
                   text && text->text == "a") {
          matched.fetch_add(1);
        } else if (std::holds_alternative<cgpui::PointerCaptureChanged>(event)) {
          matched.fetch_add(1);
        }
      });
  if (!window) return 2;
  const cgpui::NativeSurfaceHandle native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::X11SurfaceHandle>(&native);
  if (surface == nullptr) return 3;
  auto* connection = static_cast<xcb_connection_t*>(surface->display);
  const auto id = static_cast<xcb_window_t>(surface->window);

  std::atomic_bool finished{false};
  std::thread loop([&] { (*app)->run(); finished.store(true); });
  (*window)->set_cursor(cgpui::CursorShape::pointing_hand);
  (*window)->set_pointer_capture(true);
  (*window)->set_pointer_capture(false);

  xcb_motion_notify_event_t motion{};
  motion.response_type = XCB_MOTION_NOTIFY;
  motion.event = id;
  motion.event_x = 21;
  motion.event_y = 34;
  xcb_send_event(connection, false, id, XCB_EVENT_MASK_POINTER_MOTION,
                 reinterpret_cast<const char*>(&motion));
  xcb_button_press_event_t button{};
  button.response_type = XCB_BUTTON_PRESS;
  button.event = id;
  button.detail = 1;
  xcb_send_event(connection, false, id, XCB_EVENT_MASK_BUTTON_PRESS,
                 reinterpret_cast<const char*>(&button));
  button.detail = 4;
  xcb_send_event(connection, false, id, XCB_EVENT_MASK_BUTTON_PRESS,
                 reinterpret_cast<const char*>(&button));
  xcb_key_press_event_t key{};
  key.response_type = XCB_KEY_PRESS;
  key.event = id;
  key.detail = 38;
  xcb_send_event(connection, false, id, XCB_EVENT_MASK_KEY_PRESS,
                 reinterpret_cast<const char*>(&key));
  xcb_flush(connection);

  const bool passed = wait_for(matched, 7);
  (*app)->quit();
  loop.join();
  if (!passed || !finished.load()) {
    std::cerr << "X11 input matches: " << matched.load() << "/7\n";
    return 4;
  }
  return 0;
}
