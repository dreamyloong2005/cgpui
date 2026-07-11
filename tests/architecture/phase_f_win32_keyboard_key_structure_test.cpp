#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) {
    lines += value == '\n' ? 1U : 0U;
  }
  return lines;
}

} // namespace

int main() {
  const std::string keyboard_header =
      read_source("include/cgpui/core/event_keyboard.hpp");
  const std::string decoder =
      read_source("src/platform/win32/win32_keyboard_key.cpp");
  const std::string decoder_header = read_source(
      "src/platform/win32/win32_keyboard_key_internal.hpp");
  const std::string internal =
      read_source("src/platform/win32/win32_internal.hpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_keyboard.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_keyboard_key_test.cpp");
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
      &keyboard_header, &decoder, &decoder_header, &internal, &proc, &behavior,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(keyboard_header, "scan_code = 0") ||
      !contains(keyboard_header, "repeat_count = 1") ||
      !contains(keyboard_header, "bool repeated = false") ||
      !contains(keyboard_header, "bool extended = false") ||
      !contains(keyboard_header, "bool system = false") ||
      !contains(decoder_header, "decode_win32_keyboard_key(") ||
      !contains(decoder, "WM_SYSKEYDOWN") ||
      !contains(decoder, "lparam_bit(lparam, 30U)") ||
      !contains(proc, "decode_win32_keyboard_key(") ||
      !contains(proc, "if (key->system)") ||
      !contains(internal, "LPARAM lparam")) {
    return 2;
  }
  if (!contains(behavior, "WM_SYSKEYDOWN") ||
      !contains(behavior, "repeat_count != 3") ||
      !contains(behavior, "events[3].extended") ||
      !contains(xmake, "target(\"win32_keyboard_key_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_keyboard_key_structure_test\")")) {
    return 3;
  }
  if (line_count(keyboard_header) > 35 || line_count(decoder_header) > 20 ||
      line_count(decoder) > 45 || line_count(proc) > 40 ||
      line_count(behavior) > 95) {
    return 4;
  }
  constexpr const char* completion =
      "Phase F Step 549 adds real Win32 key and system-key decoding with "
      "scan-code, repeat, extended-key, and system-message metadata through "
      "a focused keyboard-key decoder. Step 550 Win32 dead-key production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 5;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_549_remaining_gap\": \"Step 550 Win32")) {
    return 6;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 550 Win32")) {
    return 7;
  }
  return 0;
}
