#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}

std::size_t count(const std::string& text, std::string_view value) {
  std::size_t result = 0;
  for (std::size_t offset = text.find(value); offset != std::string::npos;
       offset = text.find(value, offset + value.size())) {
    ++result;
  }
  return result;
}

}  // namespace

int main() {
  const std::string extractor =
      read_source("tools/gpui_parity/extract_upstream_symbols.py");
  const std::string auditor =
      read_source("tools/gpui_parity/audit_upstream_snapshot.py");
  const std::string snapshot =
      read_source("docs/gpui-upstream-snapshot.json");
  const std::string audit =
      read_source("docs/gpui-phase-j-upstream-audit.json");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string* required[]{&extractor, &auditor, &snapshot,
                                &audit,     &ledger,  &roadmap};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  constexpr std::string_view revision =
      "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0";
  if (!contains(extractor, "--require-live") ||
      !contains(extractor, "content_sha256") ||
      !contains(extractor, "fetch_mode") ||
      !contains(snapshot, "\"upstream_revision\": \"" +
                              std::string{revision} + "\"") ||
      !contains(snapshot, "\"fetch_mode\": \"live\"") ||
      !contains(snapshot, "\"public_reexport_count\": 55") ||
      !contains(snapshot, "\"example_count\": 20") ||
      count(snapshot, "\"sha256\":") < 2) {
    return 2;
  }

  constexpr std::array official_examples{
      "hello_world", "move_entity_between_windows", "image", "input",
      "on_window_close_quit", "opacity", "pattern", "set_menus", "shadow",
      "svg", "tab_stop", "text", "text_wrapper", "tree", "uniform_list",
      "window_shadow", "grid_layout", "list_example", "mouse_pressure",
      "a11y"};
  for (const auto* example : official_examples) {
    if (!contains(snapshot, "\"" + std::string{example} + "\"")) return 3;
  }

  if (!contains(auditor, "--snapshot") || !contains(auditor, "--ledger") ||
      !contains(auditor, "--output") ||
      !contains(audit, "\"status\": \"complete\"") ||
      !contains(audit, "\"required_rows\": 0") ||
      !contains(audit, "\"accepted_rows\": 32") ||
      !contains(audit, "\"snapshot_fetch_mode\": \"live\"") ||
      !contains(audit, "\"upstream_revision\": \"" +
                           std::string{revision} + "\"") ||
      !contains(ledger, "\"required\": 0") ||
      count(ledger, "\"upstream_gpui\":") != 32) {
    return 4;
  }

  if (!contains(roadmap,
                "- [x] Steps 799-804: Re-run upstream extractor") ||
      !contains(roadmap,
                "Phase J Step 804 locks the live pinned-upstream audit") ||
      !contains(roadmap, "Step 805 official example equivalence matrix")) {
    return 5;
  }
  return 0;
}
