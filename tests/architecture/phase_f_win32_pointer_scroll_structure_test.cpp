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
  const std::string pointer_header =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string decoder =
      read_source("src/platform/win32/win32_pointer_scroll.cpp");
  const std::string decoder_header = read_source(
      "src/platform/win32/win32_pointer_scroll_internal.hpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_pointer.cpp");
  const std::string pointer_events =
      read_source("src/platform/win32/win32_window_pointer_events.cpp");
  const std::string events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_pointer_scroll_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &pointer_header, &decoder, &decoder_header, &proc, &pointer_events,
      &events, &behavior, &manifests};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(pointer_header, "bool precise = false;") ||
      !contains(decoder_header, "Win32PointerScrollMessage") ||
      !contains(decoder, "WM_MOUSEWHEEL") ||
      !contains(decoder, "WM_MOUSEHWHEEL") ||
      !contains(decoder, "raw_delta % WHEEL_DELTA") ||
      !contains(proc, "decode_win32_pointer_scroll(") ||
      !contains(pointer_events, ".precise = precise") ||
      contains(events, "Win32Window::pointer_scrolled(")) {
    return 2;
  }
  if (!contains(behavior, "WM_MOUSEHWHEEL") ||
      !contains(behavior, "WHEEL_DELTA / 4") ||
      !contains(behavior, "events[2].precise") ||
      !contains(manifests, "\"win32_pointer_scroll_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_win32_pointer_scroll_structure_test.cpp\"")) {
    return 3;
  }
  if (line_count(pointer_header) > 60 || line_count(decoder_header) > 20 ||
      line_count(decoder) > 30 || line_count(proc) > 60 ||
      line_count(pointer_events) > 55 || line_count(events) > 105 ||
      line_count(behavior) > 90) {
    return 4;
  }
  return 0;
}
