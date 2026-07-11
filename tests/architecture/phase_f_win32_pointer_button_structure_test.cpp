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
      read_source("src/platform/win32/win32_pointer_button.cpp");
  const std::string decoder_header = read_source(
      "src/platform/win32/win32_pointer_button_internal.hpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_pointer.cpp");
  const std::string pointer_events =
      read_source("src/platform/win32/win32_window_pointer_events.cpp");
  const std::string events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string factory =
      read_source("src/platform/win32/win32_window_factory.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_pointer_button_test.cpp");
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
      &pointer_header, &decoder, &decoder_header, &proc, &pointer_events,
      &events, &factory, &behavior, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(pointer_header, "back,") ||
      !contains(pointer_header, "forward,") ||
      !contains(decoder_header, "Win32PointerButtonMessage") ||
      !contains(decoder, "WM_LBUTTONDBLCLK") ||
      !contains(decoder, "WM_XBUTTONDOWN") ||
      !contains(decoder, "MouseButton::back") ||
      !contains(decoder, "MouseButton::forward") ||
      !contains(proc, "decode_win32_pointer_button(") ||
      !contains(pointer_events, ".click_count = click_count") ||
      contains(events, "Win32Window::pointer_button(") ||
      !contains(factory, "CS_DBLCLKS")) {
    return 2;
  }
  if (!contains(behavior, "GetClassLongPtrW") ||
      !contains(behavior, "WM_XBUTTONDOWN") ||
      !contains(behavior, "click_count == 2") ||
      !contains(xmake, "target(\"win32_pointer_button_test\")") ||
      !contains(xmake,
                "target(\"phase_f_win32_pointer_button_structure_test\")")) {
    return 3;
  }
  if (line_count(decoder_header) > 25 || line_count(decoder) > 60 ||
      line_count(proc) > 60 || line_count(pointer_events) > 55 ||
      line_count(events) > 105 ||
      line_count(behavior) > 70) {
    return 4;
  }
  constexpr const char* completion =
      "Phase F Step 547 adds real Win32 double-click counts and back/forward "
      "XBUTTON mapping through a focused pointer-button decoder. Step 548 "
      "Win32 wheel and high-precision scroll production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 5;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_547_remaining_gap\": \"Step 548 Win32")) {
    return 6;
  }
  return 0;
}
