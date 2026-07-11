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
  const std::string header = read_source("src/platform/win32/win32_ole_file_data_object_internal.hpp");
  const std::string source = read_source("src/platform/win32/win32_ole_file_data_object.cpp");
  const std::string behavior = read_source("tests/platform/win32_ole_file_data_object_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&header, &source, &behavior, &xmake, &roadmap,
                                &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32OleFileDataObject final : public IDataObject") ||
      !contains(header, "run_win32_ole_file_drag(")) return 2;
  if (!contains(source, "MB_ERR_INVALID_CHARS") ||
      !contains(source, "CF_HDROP") || !contains(source, "DROPFILES") ||
      !contains(source, "drop->fWide = TRUE") ||
      !contains(source, "SHCreateStdEnumFmtEtc") ||
      !contains(source, "run_win32_ole_drag(data_object")) return 3;
  if (!contains(behavior, "DragQueryFileW") ||
      !contains(behavior, "IID_IDataObject") ||
      !contains(behavior, "empty_path") ||
      !contains(behavior, "embedded_nul")) return 4;
  if (line_count(header) > 60 || line_count(source) > 170 ||
      line_count(behavior) > 100) return 5;
  if (!contains(xmake, "target(\"win32_ole_file_data_object_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_ole_file_payload_structure_test\")")) return 6;
  constexpr const char* completion =
      "Phase F Step 576 adds a strict Win32 OLE file IDataObject, enumerates CF_HDROP through an owned wide DROPFILES HGLOBAL, preserves Unicode multi-file ordering, rejects invalid or empty paths before drag, and composes with the drag runner. Step 577 Wayland non-local URI-list drag policy production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 7;
  if (!contains(ledger_json, "\"phase_f_current_handoff\": \"Step 589 native directory-picker")) return 8;
  return 0;
}
