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
  const std::string actions = read_source("src/platform/linux/wayland_data_device_drag_actions.cpp");
  const std::string state = read_source("tests/platform/wayland_test_drag_offer_state.hpp");
  const std::string compositor = read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string behavior = read_source("tests/platform/wayland_pointer_button_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&actions, &state, &compositor, &behavior, &xmake,
                                &roadmap, &ledger_md, &ledger_json, &task_plan,
                                &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  const auto finish = actions.find("wl_data_offer_finish(active_offer_->offer)");
  const auto destroy = actions.find("clear_active_offer()", finish);
  if (finish == std::string::npos || destroy == std::string::npos ||
      destroy < finish) return 2;
  if (!contains(actions, "active_offer_->accepted &&") ||
      !contains(actions, "data_offer_version(active_offer_->offer) >= 3") ||
      !contains(state, "class WaylandTestDragOfferState") ||
      !contains(state, "record_destroyed(") ||
      !contains(state, "std::chrono::milliseconds{500}")) return 3;
  if (!contains(compositor, "drag_offer.record_destroyed(resource)") ||
      !contains(behavior, "wait_for_drag_offer_destroyed()") ||
      !contains(behavior, "destroy_before_leave")) return 4;
  if (line_count(actions) > 115 || line_count(state) > 110 ||
      line_count(compositor) > 3700 || line_count(behavior) > 370) return 5;
  if (!contains(xmake, "target(\"phase_f_wayland_drag_finish_structure_test\")")) return 6;
  constexpr const char* completion =
      "Phase F Step 574 completes Wayland drop offers with exactly-once finish followed by immediate destroy, destroys rejected and pre-v3 offers without finish, and observes lifecycle completion before leave in focused test state. Step 575 Win32 OLE text drag payload production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 7;
  if (!contains(ledger_json, "\"phase_f_current_handoff\": \"Step 610 platform diagnostics and stress closeout audit")) return 8;
  return 0;
}
