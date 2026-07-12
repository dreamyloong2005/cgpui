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
  const std::string header = read_source("src/platform/linux/wayland_data_device_internal.hpp");
  const std::string offer = read_source("src/platform/linux/wayland_data_device_offer.cpp");
  const std::string actions = read_source("src/platform/linux/wayland_data_device_drag_actions.cpp");
  const std::string events = read_source("src/platform/linux/wayland_data_device_drag_events.cpp");
  const std::string behavior = read_source("tests/platform/wayland_pointer_button_test.cpp");
  const std::string compositor = read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&header, &offer, &actions, &events, &behavior,
                                &compositor, &xmake, &roadmap, &ledger_md,
                                &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "WaylandDataDevice* owner = nullptr") ||
      !contains(header, "bool accepted = false") ||
      !contains(offer, "active_offer_negotiation_changed(*offer)")) return 2;
  if (!contains(actions, "return drag_supported_actions()") ||
      !contains(actions, "active_offer_->accepted = mime_type.has_value()") ||
      !contains(actions, "active_offer_->accepted &&") ||
      !contains(actions, "current_drag_action() != DragDropAction::none")) return 3;
  if (!contains(events, "active_offer_->enter_serial = serial") ||
      !contains(events, "active_offer_->entered = true") ||
      !contains(behavior, "allow_pending_enter_action") ||
      !contains(behavior, "pointer-button-unsupported") ||
      !contains(compositor, "WL_DATA_OFFER_ERROR_INVALID_FINISH")) return 4;
  const auto enter = compositor.find("wl_data_device_send_enter(");
  const auto source_actions = compositor.find("wl_data_offer_send_source_actions(");
  if (enter == std::string::npos || source_actions == std::string::npos ||
      source_actions < enter) return 5;
  if (line_count(header) > 110 || line_count(offer) > 120 ||
      line_count(actions) > 115 || line_count(events) > 90 ||
      line_count(behavior) > 360 || line_count(compositor) > 3700) return 6;
  if (!contains(xmake, "target(\"phase_f_wayland_drag_negotiation_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 573 makes Wayland drag negotiation order-safe, advertises destination copy/move capabilities independently of source actions, renegotiates late offer events, preserves pending enter actions, and rejects invalid finish requests. Step 574 Wayland data-device finish negotiation production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json, "\"phase_f_current_handoff\": \"Step 640 async timer integration production behavior")) return 9;
  return 0;
}
