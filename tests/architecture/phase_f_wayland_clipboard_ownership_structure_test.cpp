#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
  return lines;
}
} // namespace

int main() {
  const std::string internal =
      read_source("src/platform/clipboard_wayland_internal.hpp");
  const std::string source =
      read_source("src/platform/clipboard_wayland_source.cpp");
  const std::string dispatch =
      read_source("src/platform/clipboard_wayland_dispatch.cpp");
  const std::string helper =
      read_source("tests/platform/wayland_test_clipboard_source_state.hpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_clipboard_ownership_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &internal, &source, &dispatch, &helper, &compositor, &behavior,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(internal, "wl_data_source* owned_source_") ||
      !contains(internal, "owned_selection_mutex_") ||
      !contains(dispatch, "dispatch_owned_selection_events()")) return 2;
  const auto stop = source.find("stop_dispatch_thread();");
  const auto previous = source.find("previous_source = owned_source_");
  const auto set = source.find("wl_data_device_set_selection(");
  const auto flush = source.find("wl_display_flush(", set);
  const auto commit = source.find("owned_source_ = source", flush);
  const auto destroy = source.find("wl_data_source_destroy(previous_source)", commit);
  const auto restart = source.find("start_dispatch_thread()", destroy);
  if (stop == std::string::npos || previous < stop || set < previous ||
      flush < set || commit < flush || destroy < commit || restart < destroy) {
    return 3;
  }
  if (!contains(helper, "WaylandTestClipboardSourceState") ||
      !contains(helper, "replacement_continuous_") ||
      !contains(helper, "wait_for_set_count(") ||
      !contains(helper, "take_payload_request()")) return 4;
  if (!contains(compositor,
                "#include \"wayland_test_clipboard_source_state.hpp\"") ||
      !contains(compositor, "clipboard_source.set_selection(") ||
      !contains(compositor, "clipboard_source.record_payload(")) return 5;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "ownership_revision != index + 1") ||
      !contains(behavior, "last_payload") ||
      !contains(behavior,
                "clipboard_client_selection_replacement_was_continuous")) {
    return 6;
  }
  if (!contains(xmake, "target(\"wayland_clipboard_ownership_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_clipboard_ownership_structure_test\")")) {
    return 7;
  }
  if (line_count(internal) > 150 || line_count(source) > 100 ||
      line_count(dispatch) > 90 || line_count(helper) > 160 ||
      line_count(compositor) > 3700 || line_count(behavior) > 90) return 8;
  constexpr const char* completion =
      "Phase F Step 565 makes Wayland selection writes transactional, "
      "prevents display-lock starvation during replacement, preserves "
      "continuous ownership, and serves the newest payload across UTF-8 and "
      "plain-text MIME requests. Step 566 Wayland selection read production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_565_remaining_gap\": \"Step 566 Wayland")) {
    return 10;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 675 action macro production behavior")) {
    return 11;
  }
  return 0;
}
