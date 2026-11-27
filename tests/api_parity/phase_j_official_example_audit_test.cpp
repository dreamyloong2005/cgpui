#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string snapshot =
      read_source("docs/gpui-upstream-snapshot.json");
  const std::string matrix =
      read_source("docs/gpui-official-example-matrix.json");
  const std::string report =
      read_source("docs/gpui-phase-j-example-audit.json");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string targets = read_source("scripts/ci/example-targets.txt");
  const std::string manifests = read_source("mcpp.toml");
  const std::string* required[]{&snapshot,    &matrix, &report, &ledger_json,
                                &ledger_md,   &targets, &manifests};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  constexpr std::array examples{
      "hello_world", "move_entity_between_windows", "image", "input",
      "on_window_close_quit", "opacity", "pattern", "set_menus", "shadow",
      "svg", "tab_stop", "text", "text_wrapper", "tree", "uniform_list",
      "window_shadow", "grid_layout", "list_example", "mouse_pressure",
      "a11y"};
  for (const auto* example : examples) {
    const std::string entry =
        "\"upstream_example\": \"" + std::string{example} + "\"";
    if (!contains(snapshot, "\"" + std::string{example} + "\"") ||
        !contains(matrix, entry) ||
        !contains(ledger_json, "\"" + std::string{example} + "\"") ||
        !contains(ledger_md, "`" + std::string{example} + "`")) {
      return 2;
    }
  }
  if (count(matrix, "\"upstream_example\":") != examples.size() ||
      count(matrix, "\"macos\": \"compiled_and_smoked\"") !=
          examples.size() ||
      count(matrix, "\"macos_tests\":") != examples.size() ||
      count(matrix, "\"status\": \"complete\"") != examples.size() ||
      !contains(matrix, "\"macos_execution\": \"required\"")) {
    return 3;
  }
  if (!contains(matrix, "\"windows\": \"compiled_and_smoked\"") ||
      !contains(matrix, "\"wayland\": \"compiled_and_smoked\"") ||
      !contains(matrix, "\"x11\": \"compiled_and_smoked\"") ||
      !contains(matrix, "\"pixel_tests\"") ||
      !contains(matrix, "\"input_tests\"")) {
    return 4;
  }
  if (!contains(report, "\"status\": \"complete\"") ||
      !contains(report, "\"official_examples\": 20") ||
      !contains(report, "\"mapped_examples\": 20") ||
      !contains(report, "\"macos_mapped_examples\": 20") ||
      !contains(report, "\"issues\": []") ||
      !contains(report, "\"macos_execution\": \"required\"")) {
    return 5;
  }
  if (!contains(ledger_json,
                "\"phase_j_steps_805_810_status\"")) {
    return 6;
  }
  return 0;
}
