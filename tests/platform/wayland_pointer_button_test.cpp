#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <string_view>
#include <thread>
#include <variant>
#include <vector>

namespace {

constexpr cgpui::Point expected_position{42.0F, 24.0F};
constexpr std::uint32_t left_button = 0x110;

bool point_equals(cgpui::Point lhs, cgpui::Point rhs) {
  return std::fabs(lhs.x - rhs.x) < 0.01F && std::fabs(lhs.y - rhs.y) < 0.01F;
}

bool wait_for_run_finished(const std::atomic_bool& run_finished) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return run_finished.load();
}

struct ExpectedDragPayload {
  cgpui::DragDropPayloadKind kind = cgpui::DragDropPayloadKind::none;
  std::string_view text;
  std::vector<std::string> files;
};

bool payload_matches(
    const cgpui::DragDropPayload& payload,
    const ExpectedDragPayload& expected) {
  if (payload.kind != expected.kind) {
    return false;
  }
  if (payload.text != expected.text) {
    return false;
  }
  return payload.files == expected.files;
}

int run_drag_payload_case(
    std::string_view name,
    std::vector<cgpui::test::WaylandMimePayload> payloads,
    ExpectedDragPayload expected,
    int failure_base) {
  cgpui::test::WaylandTestCompositor compositor{std::string(name)};
  compositor.set_drag_payloads(std::move(payloads));
  if (!compositor.start()) {
    return failure_base + 2;
  }

  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return failure_base + 3;
  }

  bool moved = false;
  bool cursor_requested = false;
  bool pressed = false;
  bool released = false;
  bool drag_entered = false;
  bool drag_updated = false;
  bool drag_dropped = false;
  bool drag_exited = false;
  cgpui::PlatformWindow* platform_window = nullptr;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Pointer Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* move = std::get_if<cgpui::PointerMoved>(&event);
            move != nullptr && point_equals(move->position, expected_position)) {
          moved = true;
          if (!cursor_requested && platform_window != nullptr) {
            platform_window->set_cursor(cgpui::CursorShape::text);
            cursor_requested = true;
          }
        }
        if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
            button != nullptr && button->button == cgpui::MouseButton::left &&
            point_equals(button->position, expected_position)) {
          pressed = pressed || button->pressed;
          released = released || !button->pressed;
        }
        if (const auto* drag = std::get_if<cgpui::DragEntered>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_entered = payload_matches(drag->payload, expected);
        }
        if (const auto* drag = std::get_if<cgpui::DragUpdated>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_updated = payload_matches(drag->payload, expected);
        }
        if (const auto* drag = std::get_if<cgpui::DragDropped>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_dropped = payload_matches(drag->payload, expected);
        }
        if (const auto* drag = std::get_if<cgpui::DragExited>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_exited =
              drag->payload.kind == cgpui::DragDropPayloadKind::none &&
              drag->payload.text.empty() && drag->payload.files.empty();
        }
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
        if (moved && pressed && released && drag_entered && drag_updated &&
            drag_dropped && drag_exited) {
          (*app)->quit();
        }
      });
  if (!window) {
    return failure_base + 4;
  }
  platform_window = window->get();

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });

  compositor.request_pointer_move(
      static_cast<std::int32_t>(expected_position.x),
      static_cast<std::int32_t>(expected_position.y));
  compositor.request_pointer_button(left_button, true);
  compositor.request_pointer_button(left_button, false);
  compositor.request_drag_enter(
      static_cast<std::int32_t>(expected_position.x),
      static_cast<std::int32_t>(expected_position.y));
  compositor.request_drag_motion(
      static_cast<std::int32_t>(expected_position.x),
      static_cast<std::int32_t>(expected_position.y));
  compositor.request_drag_drop();
  compositor.request_drag_leave();

  if (!wait_for_run_finished(run_finished)) {
    compositor.request_close();
    wait_for_run_finished(run_finished);
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return failure_base + 9;
  }

  if (client_thread.joinable()) {
    client_thread.join();
  }
  compositor.stop();

  if (run_result != 0) {
    return failure_base + 5;
  }
  if (!compositor.wait_for_pointer_move_sent()) {
    return failure_base + 6;
  }
  if (!compositor.wait_for_pointer_cursor_set_count(2)) {
    return failure_base + 12;
  }
  if (!compositor.wait_for_pointer_button_sent()) {
    return failure_base + 7;
  }
  if (!compositor.wait_for_drag_enter_sent()) {
    return failure_base + 13;
  }
  if (!compositor.wait_for_drag_motion_sent()) {
    return failure_base + 14;
  }
  if (!compositor.wait_for_drag_drop_sent()) {
    return failure_base + 15;
  }
  if (!compositor.wait_for_drag_leave_sent()) {
    return failure_base + 16;
  }
  if (!moved) {
    return failure_base + 8;
  }
  if (!cursor_requested) {
    return failure_base + 11;
  }
  if (!pressed || !released) {
    return failure_base + 10;
  }
  if (!drag_entered || !drag_updated || !drag_dropped || !drag_exited) {
    return failure_base + 17;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = run_drag_payload_case(
          "pointer-button-none",
          {},
          ExpectedDragPayload{},
          0);
      result != 0) {
    return result;
  }
  if (const int result = run_drag_payload_case(
          "pointer-button-text",
          {
              cgpui::test::WaylandMimePayload{
                  .mime_type = "text/plain",
                  .payload = "drag text \xE4\xB8\xAD",
              },
          },
          ExpectedDragPayload{
              .kind = cgpui::DragDropPayloadKind::text,
              .text = "drag text \xE4\xB8\xAD",
          },
          100);
      result != 0) {
    return result;
  }
  if (const int result = run_drag_payload_case(
          "pointer-button-uri-list",
          {
              cgpui::test::WaylandMimePayload{
                  .mime_type = "text/uri-list",
                  .payload =
                      "file:///tmp/cgpui-one.txt\r\n"
                      "# ignored comment\r\n"
                      "file:///home/test/two%20words.txt\r\n",
              },
          },
          ExpectedDragPayload{
              .kind = cgpui::DragDropPayloadKind::files,
              .files = {
                  "/tmp/cgpui-one.txt",
                  "/home/test/two words.txt",
              },
          },
          200);
      result != 0) {
    return result;
  }

  return 0;
}
