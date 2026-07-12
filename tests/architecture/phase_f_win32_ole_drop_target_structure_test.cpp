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
  const std::string header =
      read_source("src/platform/win32/win32_drag_drop_internal.hpp");
  const std::string target =
      read_source("src/platform/win32/win32_ole_drop_target.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_ole_drop_target_test.cpp");
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
      &header, &target, &behavior, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32OleDropTarget final : public IDropTarget") ||
      !contains(target, "Win32OleDropTarget::QueryInterface(") ||
      !contains(target, "Win32OleDropTarget::AddRef()") ||
      !contains(target, "Win32OleDropTarget::Release()")) return 2;
  if (!contains(target, "if (effect == nullptr) return E_INVALIDARG") ||
      !contains(target, "if (data_object == nullptr)") ||
      !contains(target, "*effect = DROPEFFECT_NONE") ||
      !contains(target, "owner_->ole_drag_entered(") ||
      !contains(target, "owner_->ole_drag_dropped(")) return 3;
  if (!contains(behavior, "IID_IClassFactory") ||
      !contains(behavior, "target.Release() != 1") ||
      !contains(behavior, "E_INVALIDARG") ||
      !contains(behavior, "DROPEFFECT_NONE") ||
      !contains(behavior, "owner.updated != 1")) return 4;
  if (!contains(xmake, "target(\"win32_ole_drop_target_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_ole_drop_target_structure_test\")")) {
    return 5;
  }
  if (line_count(header) > 120 || line_count(target) > 100 ||
      line_count(behavior) > 90) return 6;
  constexpr const char* completion =
      "Phase F Step 571 validates Win32 OLE drop target COM inputs, returns "
      "E_INVALIDARG for missing data/effect pointers, clears rejected effects, "
      "and preserves QueryInterface, reference counting, and valid owner "
      "forwarding. Step 572 Win32 OLE drop source production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_571_remaining_gap\": \"Step 572 Win32")) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 610 platform diagnostics and stress closeout audit")) return 9;
  return 0;
}
