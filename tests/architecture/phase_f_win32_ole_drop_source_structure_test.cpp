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
} // namespace
int main() {
  const std::string header = read_source("src/platform/win32/win32_drag_drop_internal.hpp");
  const std::string source = read_source("src/platform/win32/win32_ole_drop_source.cpp");
  const std::string behavior = read_source("tests/platform/win32_ole_drop_source_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&header, &source, &behavior, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32OleDropSource final : public IDropSource") ||
      !contains(header, "run_win32_ole_drag(")) return 2;
  if (!contains(source, "DRAGDROP_S_CANCEL") || !contains(source, "DRAGDROP_S_DROP") ||
      !contains(source, "DRAGDROP_S_USEDEFAULTCURSORS") || !contains(source, "runner(&data_object")) return 3;
  if (!contains(behavior, "QueryContinueDrag(TRUE") || !contains(behavior, "observed_allowed") ||
      !contains(xmake, "target(\"win32_ole_drop_source_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_ole_drop_source_structure_test\")")) return 4;
  constexpr const char* completion =
      "Phase F Step 572 adds a Win32 OLE IDropSource and injectable DoDragDrop runner, supports escape cancellation, button-release drop, default cursor feedback, allowed-effect propagation, and final effect reporting. Step 573 Wayland data-device accept and action negotiation production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 5;
  if (!contains(ledger_json, "\"phase_f_current_handoff\": \"Step 613 cross-platform test execution audit")) return 6;
  return 0;
}
