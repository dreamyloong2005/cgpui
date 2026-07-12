#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_dbus_test_support.hpp"
#include "wayland_atspi_role_state_internal.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <array>
#include <cstdint>
#include <string>

namespace {

std::uint32_t reply_uint32(DBusMessage* reply) {
  DBusError error;
  dbus_error_init(&error);
  dbus_uint32_t value = 0;
  if (reply == nullptr || !dbus_message_get_args(
          reply, &error, DBUS_TYPE_UINT32, &value, DBUS_TYPE_INVALID)) {
    dbus_error_free(&error);
    return 0;
  }
  return value;
}

std::array<std::uint32_t, 2> reply_state_set(DBusMessage* reply) {
  DBusMessageIter root;
  if (reply == nullptr || !dbus_message_iter_init(reply, &root)) return {};
  DBusMessageIter array;
  dbus_message_iter_recurse(&root, &array);
  std::array<std::uint32_t, 2> words{};
  for (std::uint32_t& word : words) {
    if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_UINT32) return {};
    dbus_message_iter_get_basic(&array, &word);
    dbus_message_iter_next(&array);
  }
  return words;
}

bool state_present(
    const std::array<std::uint32_t, 2>& words, std::uint32_t state) {
  return (words[state / 32] & (1U << (state % 32))) != 0;
}

DBusMessage* role_call(const char* path, const char* method) {
  return cgpui::test::atspi_method_call(
      path, "org.a11y.atspi.Accessible", method);
}

} // namespace

int main() {
  using namespace cgpui;

  test::WaylandAtspiDbusRecorder recorder;
  auto adapter = create_wayland_atspi_accessibility_adapter();
  wayland_atspi_attach_dbus(
      *adapter, recorder.connection(), recorder.operations());

  PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 1;
  update.node_count = 3;
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 1,
      .role = PlatformAccessibilityRole::generic,
      .name = "window",
      .child_count = 2,
  });
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 2,
      .parent_element_id = 1,
      .role = PlatformAccessibilityRole::button,
      .name = "Run",
      .enabled = true,
      .focusable = true,
      .focused = true,
  });
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 3,
      .parent_element_id = 1,
      .role = PlatformAccessibilityRole::checkbox,
      .name = "Remember",
      .patterns = PlatformAccessibilityPatternState{.toggled = true},
      .enabled = false,
      .focusable = true,
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));

  DBusMessage* role = role_call(
      "/org/a11y/atspi/accessible/2", "GetRole");
  if (recorder.call(role, 1) != DBUS_HANDLER_RESULT_HANDLED) return 1;
  dbus_message_unref(role);
  if (reply_uint32(recorder.sent_reply) != 43) return 2;

  DBusMessage* role_name = role_call(
      "/org/a11y/atspi/accessible/2", "GetRoleName");
  if (recorder.call(role_name, 1) != DBUS_HANDLER_RESULT_HANDLED) return 3;
  dbus_message_unref(role_name);
  if (test::reply_string(recorder.sent_reply) != "push button") return 4;

  DBusMessage* state = role_call(
      "/org/a11y/atspi/accessible/2", "GetState");
  if (recorder.call(state, 1) != DBUS_HANDLER_RESULT_HANDLED) return 5;
  dbus_message_unref(state);
  const auto button_states = reply_state_set(recorder.sent_reply);
  if (!state_present(button_states, 1) ||
      !state_present(button_states, 8) ||
      !state_present(button_states, 11) ||
      !state_present(button_states, 12) ||
      !state_present(button_states, 24) ||
      !state_present(button_states, 25) ||
      !state_present(button_states, 30)) {
    return 6;
  }

  DBusMessage* checkbox_role = role_call(
      "/org/a11y/atspi/accessible/3", "GetRole");
  if (recorder.call(checkbox_role, 2) != DBUS_HANDLER_RESULT_HANDLED) return 7;
  dbus_message_unref(checkbox_role);
  if (reply_uint32(recorder.sent_reply) != 7) return 8;

  DBusMessage* checkbox_state = role_call(
      "/org/a11y/atspi/accessible/3", "GetState");
  if (recorder.call(checkbox_state, 2) != DBUS_HANDLER_RESULT_HANDLED) return 9;
  dbus_message_unref(checkbox_state);
  const auto checkbox_states = reply_state_set(recorder.sent_reply);
  if (!state_present(checkbox_states, 4) ||
      !state_present(checkbox_states, 11) ||
      !state_present(checkbox_states, 25) ||
      !state_present(checkbox_states, 30) ||
      !state_present(checkbox_states, 41) ||
      state_present(checkbox_states, 8) ||
      state_present(checkbox_states, 24)) {
    return 10;
  }

  if (wayland_atspi_role_for(PlatformAccessibilityRole::text_input).id != 79 ||
      wayland_atspi_role_for(PlatformAccessibilityRole::slider).id != 51 ||
      wayland_atspi_role_for(PlatformAccessibilityRole::image).id != 27 ||
      wayland_atspi_role_for(PlatformAccessibilityRole::label).id != 29) {
    return 11;
  }

  wayland_atspi_detach_dbus(*adapter);
  return 0;
}
