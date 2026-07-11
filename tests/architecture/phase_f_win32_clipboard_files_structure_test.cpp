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
  const std::string public_header =
      read_source("include/cgpui/platform/clipboard.hpp");
  const std::string defaults = read_source("src/platform/clipboard.cpp");
  const std::string internal =
      read_source("src/platform/clipboard_win32_internal.hpp");
  const std::string files =
      read_source("src/platform/clipboard_win32_files.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_clipboard_files_test.cpp");
  const std::string test_lock =
      read_source("tests/platform/win32_clipboard_test_lock.hpp");
  const std::string platform_structure =
      read_source("tests/architecture/platform_source_structure_test.cpp");
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
      &public_header, &defaults, &internal, &files, &behavior, &test_lock,
      &platform_structure, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(public_header,
                "std::optional<std::vector<std::string>>") ||
      !contains(public_header, "read_files() const;") ||
      !contains(public_header,
                "write_files(std::span<const std::string> paths)")) return 2;
  if (!contains(defaults, "Clipboard::read_files() const") ||
      !contains(defaults, "Clipboard::write_files(") ||
      !contains(defaults, "return std::nullopt;") ||
      !contains(defaults, "return false;")) return 3;
  if (!contains(internal, "read_files() const override;") ||
      !contains(internal, "write_files(")) return 4;
  if (!contains(files, "Win32Clipboard::read_files() const") ||
      !contains(files, "Win32Clipboard::write_files(") ||
      !contains(files, "CF_HDROP") || !contains(files, "DROPFILES") ||
      !contains(files, "fWide = TRUE") ||
      !contains(files, "DragQueryFileW") ||
      !contains(files, "widen_clipboard_text(") ||
      !contains(files, "narrow_clipboard_text(") ||
      !contains(files, "GMEM_ZEROINIT") ||
      !contains(files, "SetClipboardData(CF_HDROP")) return 5;
  if (!contains(behavior, "unicode") ||
      !contains(behavior, "\\xF0\\x9F\\x98\\x80") ||
      !contains(behavior, "invalid_utf8") ||
      !contains(behavior, "embedded_nul") ||
      !contains(behavior, "L\"baseline\"")) return 6;
  if (!contains(test_lock, "Win32ClipboardTestLock") ||
      !contains(test_lock, "CreateMutexW") ||
      !contains(test_lock, "WaitForSingleObject") ||
      !contains(behavior, "Win32ClipboardTestLock clipboard_lock")) return 12;
  if (!contains(platform_structure,
                "\"src/platform/clipboard_win32_files.cpp\"") ||
      !contains(xmake, "target(\"win32_clipboard_files_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_clipboard_files_structure_test\")") ||
      !contains(xmake,
                "add_syslinks(\"shell32\", \"user32\", {public = true})")) {
    return 7;
  }
  if (line_count(public_header) > 110 || line_count(defaults) > 80 ||
      line_count(internal) > 55 || line_count(files) > 130 ||
      line_count(behavior) > 140 || line_count(test_lock) > 50) return 8;
  constexpr const char* completion =
      "Phase F Step 564 adds Win32 CF_HDROP file clipboard read and write "
      "with strict UTF-8 paths, wide DROPFILES payloads, multi-file ordering, "
      "and rejection-safe system content preservation. Step 565 Wayland "
      "selection ownership and write production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_564_remaining_gap\": \"Step 565 Wayland")) {
    return 10;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 595 multi-window event-loop ownership")) {
    return 11;
  }
  return 0;
}
