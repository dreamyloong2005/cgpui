#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <wayland-client-protocol.h>

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
  cgpui::DragDropAction action = cgpui::DragDropAction::none;
  std::string_view text;
  std::vector<std::string> files;
  std::string_view accepted_mime;
  std::uint32_t source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  std::uint32_t selected_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  std::uint32_t client_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  std::uint32_t preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  bool allow_pending_enter_action = false;
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

bool drag_action_matches(
    cgpui::DragDropAction action,
    const ExpectedDragPayload& expected) {
  return action == expected.action;
}

int run_drag_payload_case(
    std::string_view name,
    std::vector<cgpui::test::WaylandMimePayload> payloads,
    ExpectedDragPayload expected,
    int failure_base) {
  cgpui::test::WaylandTestCompositor compositor{std::string(name)};
  compositor.set_drag_payloads(std::move(payloads));
  compositor.set_drag_source_actions(
      expected.source_actions,
      expected.selected_action);
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
  bool hand_cursor_requested = false;
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
          drag_entered =
              payload_matches(drag->payload, expected) &&
              (drag_action_matches(drag->action, expected) ||
               (expected.allow_pending_enter_action &&
                drag->action == cgpui::DragDropAction::none));
          if (!hand_cursor_requested && platform_window != nullptr) {
            platform_window->set_cursor(cgpui::CursorShape::pointing_hand);
            hand_cursor_requested = true;
          }
        }
        if (const auto* drag = std::get_if<cgpui::DragUpdated>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_updated = payload_matches(drag->payload, expected) &&
                         drag_action_matches(drag->action, expected);
        }
        if (const auto* drag = std::get_if<cgpui::DragDropped>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_dropped = payload_matches(drag->payload, expected) &&
                         drag_action_matches(drag->action, expected);
        }
        if (const auto* drag = std::get_if<cgpui::DragExited>(&event);
            drag != nullptr && point_equals(drag->position, expected_position)) {
          drag_exited =
              drag->payload.kind == cgpui::DragDropPayloadKind::none &&
              drag->payload.text.empty() && drag->payload.files.empty() &&
              drag->action == cgpui::DragDropAction::none;
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
  const bool offer_expected =
      expected.source_actions != WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  const bool finish_before_leave =
      !offer_expected || expected.kind == cgpui::DragDropPayloadKind::none ||
      compositor.wait_for_drag_offer_finished();
  const bool destroy_before_leave =
      !offer_expected || compositor.wait_for_drag_offer_destroyed();
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
  if (!compositor.wait_for_pointer_cursor_set_count(3)) {
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
  if (!cursor_requested || !hand_cursor_requested) {
    return failure_base + 11;
  }
  if (!pressed || !released) {
    return failure_base + 10;
  }
  if (!drag_entered || !drag_updated || !drag_dropped || !drag_exited) {
    return failure_base + 17;
  }
  if (!finish_before_leave) return failure_base + 24;
  if (!destroy_before_leave) return failure_base + 25;
  if (expected.kind != cgpui::DragDropPayloadKind::none) {
    if (!compositor.wait_for_drag_offer_accepted()) {
      return failure_base + 18;
    }
    if (compositor.last_drag_accept_mime_type() != expected.accepted_mime) {
      return failure_base + 19;
    }
    if (!compositor.wait_for_drag_offer_actions_set()) {
      return failure_base + 20;
    }
    if (compositor.last_drag_offer_actions() != expected.client_actions) {
      return failure_base + 21;
    }
    if (compositor.last_drag_preferred_action() != expected.preferred_action) {
      return failure_base + 22;
    }
    if (!compositor.wait_for_drag_offer_finished()) {
      return failure_base + 23;
    }
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
              .action = cgpui::DragDropAction::move,
              .text = "drag text \xE4\xB8\xAD",
              .accepted_mime = "text/plain",
              .source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                                WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
              .selected_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
              .client_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                                WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
              .preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
              .allow_pending_enter_action = true,
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
                      "file://remote.example/share/ignored.txt\r\n"
                      "file:///tmp/ignored%00suffix\r\n"
                      "file:///home/test/two%20words.txt\r\n",
              },
          },
          ExpectedDragPayload{
              .kind = cgpui::DragDropPayloadKind::files,
              .action = cgpui::DragDropAction::copy,
              .files = {
                  "/tmp/cgpui-one.txt",
                  "/home/test/two words.txt",
              },
              .accepted_mime = "text/uri-list",
              .source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
              .selected_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
              .client_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                                WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
              .preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
              .allow_pending_enter_action = true,
          },
          200);
      result != 0) {
    return result;
  }
  if (const int result = run_drag_payload_case(
          "pointer-button-unsupported",
          {
              cgpui::test::WaylandMimePayload{
                  .mime_type = "application/octet-stream",
                  .payload = "unsupported",
              },
          },
          ExpectedDragPayload{
              .kind = cgpui::DragDropPayloadKind::none,
              .action = cgpui::DragDropAction::copy,
              .source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
              .selected_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
              .client_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
                                WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
              .preferred_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
              .allow_pending_enter_action = true,
          },
          300);
      result != 0) {
    return result;
  }

  return 0;
}
