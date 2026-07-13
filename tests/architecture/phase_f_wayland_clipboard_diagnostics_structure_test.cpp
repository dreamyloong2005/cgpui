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
  const std::string internal =
      read_source("src/platform/clipboard_wayland_internal.hpp");
  const std::string state_header =
      read_source("src/platform/clipboard_wayland_diagnostics_internal.hpp");
  const std::string state =
      read_source("src/platform/clipboard_wayland_diagnostics.cpp");
  const std::string connection =
      read_source("src/platform/clipboard_wayland_connection.cpp");
  const std::string source =
      read_source("src/platform/clipboard_wayland_source.cpp");
  const std::string source_io =
      read_source("src/platform/clipboard_wayland_source_io.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_clipboard_diagnostics_test.cpp");
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
      &public_header, &internal, &state_header, &state, &connection, &source,
      &source_io, &behavior, &platform_structure, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "enum class WaylandClipboardOperation") ||
      !contains(public_header, "enum class WaylandClipboardFailure") ||
      !contains(public_header, "struct WaylandClipboardDiagnostics") ||
      !contains(public_header, "WaylandClipboardDiagnostics diagnostics() const")) {
    return 2;
  }
  if (!contains(state_header, "WaylandClipboardDiagnosticState") ||
      !contains(state_header, "std::atomic_uint64_t sequence_") ||
      !contains(state, "sequence_.fetch_add(1") ||
      !contains(state, "before / 2U")) return 3;
  if (!contains(internal, "WaylandClipboardDiagnosticState diagnostics_") ||
      !contains(connection, "diagnostics_.snapshot()") ||
      !contains(source, "WaylandClipboardOperation::write") ||
      !contains(source_io, "WaylandClipboardOperation::send") ||
      !contains(source_io, "WaylandClipboardFailure::receiver_closed") ||
      !contains(source_io, "WaylandClipboardFailure::transfer_timeout")) return 4;
  if (!contains(behavior, "clipboard.diagnostics()") ||
      !contains(behavior, "receiver_closed") ||
      !contains(behavior, "recovered.bytes_transferred")) return 5;
  if (!contains(platform_structure,
                "clipboard_wayland_diagnostics_internal.hpp") ||
      !contains(platform_structure, "clipboard_wayland_diagnostics.cpp") ||
      !contains(xmake, "target(\"wayland_clipboard_diagnostics_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_clipboard_diagnostics_structure_test\")")) {
    return 6;
  }
  if (line_count(public_header) > 110 || line_count(internal) > 150 ||
      line_count(state_header) > 60 || line_count(state) > 80 ||
      line_count(source) > 120 || line_count(source_io) > 50 ||
      line_count(behavior) > 90) return 7;
  constexpr const char* completion =
      "Phase F Step 570 adds allocation-free Wayland clipboard diagnostic "
      "snapshots for write and async send operations, reports receiver-close "
      "and timeout failures with byte counts and revisions, and records "
      "recovery after failure. Step 571 Win32 OLE drop target production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_570_remaining_gap\": \"Step 571 Win32")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 670 examples and smoke test matrix coverage")) {
    return 10;
  }
  return 0;
}
