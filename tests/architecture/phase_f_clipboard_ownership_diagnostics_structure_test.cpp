#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
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
  const std::string public_header = read_source("include/cgpui/platform/clipboard.hpp");
  const std::string state_header = read_source("src/platform/clipboard_wayland_diagnostics_internal.hpp");
  const std::string state = read_source("src/platform/clipboard_wayland_diagnostics.cpp");
  const std::string source = read_source("src/platform/clipboard_wayland_source.cpp");
  const std::string source_events = read_source("src/platform/clipboard_wayland_source_events.cpp");
  const std::string behavior = read_source("tests/platform/wayland_clipboard_ownership_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_window_churn_diagnostics_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&public_header, &state_header, &state, &source,
      &source_events, &behavior, &previous, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  const char* fields[]{"owns_selection", "owned_payload_bytes",
      "ownership_revision"};
  for (const char* field : fields) {
    if (!contains(public_header, field) || !contains(state_header, field) ||
        !contains(state, field) || !contains(behavior, field)) return 2;
  }
  if (!contains(state_header, "std::atomic_flag writer_lock_") ||
      !contains(state, "writer_lock_.test_and_set(") ||
      !contains(state, "writer_lock_.clear(") ||
      !contains(state, "record_ownership(")) return 3;
  if (!contains(source, "diagnostics_.record_ownership(true, text.size())") ||
      !contains(source_events,
                "diagnostics_.record_ownership(false, 0)")) return 4;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "ownership_revision != index + 1") ||
      !contains(behavior,
                "clipboard_client_selection_replacement_was_continuous") ||
      !contains(previous, "Phase F Step 603")) return 5;
  if (line_count(public_header) > 110 || line_count(state_header) > 60 ||
      line_count(state) > 80 || line_count(source) > 120 ||
      line_count(source_events) > 100 || line_count(behavior) > 90) return 6;
  if (!contains(xmake, "target(\"wayland_clipboard_ownership_test\")") ||
      !contains(xmake,
                "target(\"phase_f_clipboard_ownership_diagnostics_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 604 reports current Wayland selection ownership, owned payload bytes, and ownership revisions through writer-serialized diagnostics, and verifies 64 continuous replacements serve the newest payload. Step 605 drag-and-drop cancellation diagnostics and stress production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 658 official image/GIF examples and asset closeout\"")) return 9;
  return 0;
}
