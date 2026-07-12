#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
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
  const std::string win32_source =
      read_source("src/platform/win32/win32_ole_drop_source.cpp");
  const std::string win32_behavior =
      read_source("tests/platform/win32_ole_drop_source_test.cpp");
  const std::string wayland_events =
      read_source("src/platform/linux/wayland_data_device_drag_events.cpp");
  const std::string wayland_offer =
      read_source("src/platform/linux/wayland_data_device_offer.cpp");
  const std::string wayland_behavior =
      read_source("tests/platform/wayland_drag_cancel_test.cpp");
  const std::string offer_state =
      read_source("tests/platform/wayland_test_drag_offer_state.hpp");
  const std::string step_571 = read_source(
      "tests/architecture/phase_f_win32_ole_drop_target_structure_test.cpp");
  const std::string step_574 = read_source(
      "tests/architecture/phase_f_wayland_drag_finish_structure_test.cpp");
  const std::string step_577 = read_source(
      "tests/architecture/phase_f_wayland_uri_policy_structure_test.cpp");
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
      &win32_source, &win32_behavior, &wayland_events, &wayland_offer,
      &wayland_behavior, &offer_state, &step_571, &step_574, &step_577,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(win32_source, "result == DRAGDROP_S_DROP ? effect") ||
      !contains(win32_source, ": DROPEFFECT_NONE") ||
      !contains(win32_behavior, "cancel_drag(") ||
      !contains(win32_behavior, "cancelled.effect != DROPEFFECT_NONE")) return 2;
  if (!contains(wayland_events, "WaylandDataDevice::handle_leave(") ||
      !contains(wayland_events, "self->drag_window_ = nullptr") ||
      !contains(wayland_events, "self->clear_active_offer()") ||
      !contains(wayland_offer, "wl_data_offer_destroy(offer->offer)")) return 3;
  if (!contains(wayland_behavior, "request_drag_leave()") ||
      !contains(wayland_behavior, "request_drag_drop()") ||
      !contains(wayland_behavior, "exited.load() != 1") ||
      !contains(wayland_behavior, "dropped.load() != 0") ||
      !contains(wayland_behavior, "drag_offer_finished()")) return 4;
  if (!contains(offer_state, "bool finished() const") ||
      !contains(step_571, "Phase F Step 571") ||
      !contains(step_574, "Phase F Step 574") ||
      !contains(step_577, "Phase F Step 577")) return 5;
  if (line_count(win32_source) > 70 || line_count(win32_behavior) > 75 ||
      line_count(wayland_events) > 95 || line_count(wayland_offer) > 125 ||
      line_count(wayland_behavior) > 120 || line_count(offer_state) > 110) return 6;
  if (!contains(xmake, "target(\"wayland_drag_cancel_test\")") ||
      !contains(xmake, "target(\"phase_f_drag_drop_closeout_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 578 normalizes Win32 OLE cancellation to a none effect, proves Wayland leave destroys offers without finish, suppresses duplicate exit and late drop callbacks, and closes the Steps 571-578 drag/drop band. Step 579 native menu tree production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 617 final Windows/WSL gate")) return 9;
  return 0;
}
