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
  const std::string internal = read_source(
      "src/platform/clipboard_win32_internal.hpp");
  const std::string text = read_source(
      "src/platform/clipboard_win32_text.cpp");
  const std::string read = read_source(
      "src/platform/clipboard_win32_read.cpp");
  const std::string write = read_source(
      "src/platform/clipboard_win32_write.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_clipboard_unicode_test.cpp");
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
      &internal, &text, &read, &write, &behavior, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(internal, "std::optional<std::wstring> widen_clipboard_text") ||
      !contains(internal, "std::optional<std::string> narrow_clipboard_text")) {
    return 2;
  }
  if (!contains(text, "MB_ERR_INVALID_CHARS") ||
      !contains(text, "WC_ERR_INVALID_CHARS") ||
      !contains(text, "value.find('\\0')") ||
      !contains(text, "value.find(L'\\0')") ||
      !contains(text, "value.size() > INT_MAX") ||
      !contains(text, "written == required")) return 3;
  if (!contains(write, "if (!wide_text) return false;") ||
      !contains(read, "narrow_clipboard_text(")) return 4;
  if (!contains(behavior, "\\xF0\\x9F\\x98\\x80") ||
      !contains(behavior, "line one\\r\\nemoji") ||
      !contains(behavior, "invalid_utf8") ||
      !contains(behavior, "embedded_nul") ||
      !contains(behavior, "L\"baseline\"")) return 5;
  if (!contains(xmake, "target(\"win32_clipboard_unicode_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_clipboard_unicode_structure_test\")")) {
    return 6;
  }
  if (line_count(internal) > 50 || line_count(text) > 90 ||
      line_count(read) > 70 || line_count(write) > 80 ||
      line_count(behavior) > 130) return 7;
  constexpr const char* completion =
      "Phase F Step 563 makes Win32 Unicode clipboard conversion strict, "
      "preserves emoji, CRLF, and empty text, rejects invalid UTF-8 and "
      "embedded NUL, and leaves existing system content intact on rejection. "
      "Step 564 Win32 file clipboard production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_563_remaining_gap\": \"Step 564 Win32")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 581 native menu")) {
    return 10;
  }
  return 0;
}
