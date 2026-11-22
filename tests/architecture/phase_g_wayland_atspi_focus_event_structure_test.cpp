#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string event_header = read_source(
      "src/platform/linux/wayland_atspi_events_internal.hpp");
  const std::string event_source = read_source(
      "src/platform/linux/wayland_atspi_events.cpp");
  const std::string helper = read_source(
      "tests/platform/wayland_atspi_event_test_support.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_focus_event_test.cpp");
  const std::string text_value_behavior = read_source(
      "tests/platform/wayland_atspi_text_value_event_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &event_header, &event_source, &helper, &behavior, &text_value_behavior,
      &manifests};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(event_header, "focus_event_count") ||
      contains(event_header, "deferred_focus_count")) {
    return 2;
  }
  if (!contains(event_source, "PlatformAccessibilityLiveUpdateKind::focus_changed") ||
      !contains(event_source, "\"StateChanged\"") ||
      !contains(event_source, "\"focused\"") ||
      !contains(event_source, "update.focused ? 1 : 0") ||
      !contains(event_source, "diagnostics_.focus_event_count += 1")) {
    return 3;
  }
  if (!contains(helper, "struct AtspiEventPayload") ||
      !contains(helper, "read_atspi_event_payload(DBusMessage* message)") ||
      !contains(helper, "payload.source = read_object_reference(&root)")) {
    return 4;
  }
  if (!contains(behavior, "dbus_message_get_member(message)) != \"StateChanged\"") ||
      !contains(behavior, "dbus_message_get_signature(message)) != \"siiv(so)\"") ||
      !contains(behavior, "payload.detail != \"focused\"") ||
      !contains(behavior, "payload.detail1 != (index == 0 ? 1 : 0)") ||
      !contains(behavior, "diagnostics.focus_event_count != 2")) {
    return 5;
  }
  if (!contains(text_value_behavior, "read_atspi_event_payload(value_event)") ||
      !contains(text_value_behavior, "read_atspi_event_payload(text_event)")) {
    return 6;
  }
  if (!contains(manifests, "\"wayland_atspi_focus_event_test.cpp\"") ||
      !contains(
          manifests,
          "\"phase_g_wayland_atspi_focus_event_structure_test.cpp\"")) {
    return 7;
  }
  if (line_count(event_header) > 50 || line_count(event_source) > 150 ||
      line_count(helper) > 60 || line_count(behavior) > 120 ||
      line_count(text_value_behavior) > 170) {
    return 8;
  }

  return 0;
}
