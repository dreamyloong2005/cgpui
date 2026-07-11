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
  const std::string decoder_header = read_source(
      "src/platform/win32/win32_text_input_internal.hpp");
  const std::string decoder =
      read_source("src/platform/win32/win32_text_input.cpp");
  const std::string proc =
      read_source("src/platform/win32/win32_window_proc_text.cpp");
  const std::string main_proc =
      read_source("src/platform/win32/win32_window_proc.cpp");
  const std::string keyboard_proc = read_source(
      "src/platform/win32/win32_window_proc_keyboard.cpp");
  const std::string window_text =
      read_source("src/platform/win32/win32_window_text.cpp");
  const std::string window_events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string window_internal =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string behavior = read_source(
      "tests/platform/win32_text_input_production_test.cpp");
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
      &decoder_header, &decoder, &proc, &main_proc, &keyboard_proc,
      &window_text, &window_events, &window_internal, &behavior, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(decoder_header, "struct Win32TextInputState") ||
      !contains(decoder_header, "std::array<wchar_t, 2> utf16") ||
      !contains(decoder, "WM_CHAR") || !contains(decoder, "WM_SYSCHAR") ||
      !contains(decoder, "WM_UNICHAR") ||
      !contains(decoder, "wparam == UNICODE_NOCHAR") ||
      !contains(decoder, "is_control_character(message.value)") ||
      !contains(decoder, "consume_win32_text_input(")) {
    return 2;
  }
  if (!contains(proc, "decode_win32_text_input(") ||
      !contains(proc, "result = TRUE") ||
      !contains(proc, "if (text->system)") ||
      !contains(main_proc, "win32_window_proc_handle_text(") ||
      contains(keyboard_proc, "WM_CHAR") ||
      !contains(window_text, "consume_win32_text_input(text_input_state_") ||
      !contains(window_events, "reset_win32_text_input_state(text_input_state_)") ||
      !contains(window_internal, "Win32TextInputState text_input_state_")) {
    return 3;
  }
  if (!contains(behavior, "UNICODE_NOCHAR") ||
      !contains(behavior, "WM_SYSCHAR") ||
      !contains(behavior, "0xD83D") || !contains(behavior, "0xDE00") ||
      !contains(behavior, "VK_BACK") || !contains(behavior, "0x110000") ||
      !contains(xmake, "target(\"win32_text_input_production_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_text_input_structure_test\")")) {
    return 4;
  }
  if (line_count(decoder_header) > 55 || line_count(decoder) > 115 ||
      line_count(proc) > 40 || line_count(window_text) > 40 ||
      line_count(window_internal) > 110 || line_count(behavior) > 90) {
    return 5;
  }
  constexpr const char* completion =
      "Phase F Step 551 adds production Win32 text input with UTF-16 "
      "surrogate pairing, WM_UNICHAR negotiation and codepoint delivery, "
      "system-character suppression, and focus-loss state reset through "
      "focused text-input and window-procedure modules. Step 552 Win32 "
      "cursor theme and system cursor production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 6;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_551_remaining_gap\": \"Step 552 Win32")) {
    return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 552 Win32")) {
    return 8;
  }
  return 0;
}
