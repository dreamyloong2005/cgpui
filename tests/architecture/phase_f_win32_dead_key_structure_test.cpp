#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &text_header, &decoder_header, &decoder, &proc, &window_text,
      &window_events, &window_internal, &message_target, &behavior, &manifests};
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
      !contains(manifests, "\"win32_dead_key_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_dead_key_structure_test.cpp\"")) {
    return 3;
  }
  if (line_count(text_header) > 80 || line_count(decoder_header) > 20 ||
      line_count(decoder) > 25 || line_count(window_text) > 35 ||
      line_count(window_internal) > 134 || line_count(proc) > 50 ||
      line_count(behavior) > 70) {
    return 4;
  }
  return 0;
}
