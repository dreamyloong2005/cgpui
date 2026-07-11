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
  const std::string pointer_header =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string decoder =
      read_source("src/platform/win32/win32_pointer_scroll.cpp");
  const std::string decoder_header = read_source(
      "src/platform/win32/win32_pointer_scroll_internal.hpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_pointer.cpp");
  const std::string events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_pointer_scroll_test.cpp");
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
      &pointer_header, &decoder, &decoder_header, &proc, &events, &behavior,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
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
      !contains(events, ".precise = precise")) {
    return 2;
  }
  if (!contains(behavior, "WM_MOUSEHWHEEL") ||
      !contains(behavior, "WHEEL_DELTA / 4") ||
      !contains(behavior, "events[2].precise") ||
      !contains(xmake, "target(\"win32_pointer_scroll_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_pointer_scroll_structure_test\")")) {
    return 3;
  }
  if (line_count(pointer_header) > 50 || line_count(decoder_header) > 20 ||
      line_count(decoder) > 30 || line_count(proc) > 60 ||
      line_count(events) > 105 || line_count(behavior) > 90) {
    return 4;
  }
  constexpr const char* completion =
      "Phase F Step 548 adds real Win32 vertical and horizontal wheel routing "
      "with fractional high-precision deltas and explicit precision metadata "
      "through a focused pointer-scroll decoder. Step 549 Win32 keyboard "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 5;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_548_remaining_gap\": \"Step 549 Win32")) {
    return 6;
  }
  return 0;
}
