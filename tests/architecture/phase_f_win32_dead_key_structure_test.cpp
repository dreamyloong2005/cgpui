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
  const std::string text_header =
      read_source("include/cgpui/core/event_text.hpp");
  const std::string decoder_header = read_source(
      "src/platform/win32/win32_dead_key_internal.hpp");
  const std::string decoder =
      read_source("src/platform/win32/win32_dead_key.cpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_keyboard.cpp");
  const std::string window_text =
      read_source("src/platform/win32/win32_window_text.cpp");
  const std::string window_events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string window_internal =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string message_target = read_source(
      "src/platform/win32/win32_window_message_internal.hpp");
  const std::string behavior =
      read_source("tests/platform/win32_dead_key_test.cpp");
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
      &text_header, &decoder_header, &decoder, &proc, &window_text,
      &window_events, &window_internal, &message_target, &behavior, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(text_header, "bool composed = false") ||
      !contains(decoder_header, "decode_win32_dead_key(") ||
      !contains(decoder, "WM_DEADCHAR") ||
      !contains(decoder, "WM_SYSDEADCHAR") ||
      !contains(proc, "decode_win32_dead_key(") ||
      !contains(proc, "window->dead_key(dead_key->character)") ||
      !contains(proc, "if (dead_key->system)") ||
      !contains(message_target, "virtual void dead_key(wchar_t character)") ||
      !contains(window_internal, "Win32TextInputState text_input_state_")) {
    return 2;
  }
  if (!contains(window_text, "Win32Window::dead_key(") ||
      !contains(window_text, "record_win32_dead_key(text_input_state_") ||
      !contains(window_text, ".composed = commit->composed") ||
      contains(window_events, "Win32Window::text_input(") ||
      !contains(window_events, "reset_win32_text_input_state(text_input_state_)") ||
      !contains(behavior, "WM_SYSDEADCHAR") ||
      !contains(behavior, "text_events[3].composed") ||
      !contains(xmake, "target(\"win32_dead_key_test\")") ||
      !contains(xmake, "target(\"phase_f_win32_dead_key_structure_test\")")) {
    return 3;
  }
  if (line_count(text_header) > 65 || line_count(decoder_header) > 20 ||
      line_count(decoder) > 25 || line_count(window_text) > 35 ||
      line_count(window_internal) > 120 || line_count(proc) > 50 ||
      line_count(behavior) > 70) {
    return 4;
  }
  constexpr const char* completion =
      "Phase F Step 550 adds explicit Win32 dead/system-dead character "
      "suppression with pending composition state, composed TextInput "
      "metadata, and focus-loss reset through focused dead-key and "
      "window-text modules. Step 551 Win32 text-input production behavior "
      "is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 5;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_550_remaining_gap\": \"Step 551 Win32")) {
    return 6;
  }
  return 0;
}
