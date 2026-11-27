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

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

}  // namespace

int main() {
  const std::string self = read_source(
      "tests/architecture/phase_h_final_closeout_structure_test.cpp");
  const std::string api =
      read_source("tests/api_parity/phase_h_final_closeout_test.cpp");
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string structure =
      read_source("tests/architecture/mcpp.toml");
  const std::string closeout =
      read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/architecture/build.mcpp");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{&self, &api, &manifests, &structure,
                                &closeout, &manifest, &ledger_json};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  if (!contains(manifests, "\"tests/architecture\",") ||
      contains(read_source("mcpp.toml"), "\"phase_h_final_closeout_test.cpp\"") ||
      contains(read_source("mcpp.toml"), "\"phase_h_final_closeout_structure_test.cpp\"") ||
      !contains(closeout, "\"phase_h_final_closeout_test.cpp\"") ||
      !contains(closeout, "\"phase_h_final_closeout_test.cpp\"") ||
      !contains(closeout,
                "\"phase_h_final_closeout_structure_test.cpp\"") ||
      !contains(closeout,
                "\"phase_h_final_closeout_structure_test.cpp\"") ||
      !contains(closeout, "source_root_define(2)")) {
    return 2;
  }

  constexpr std::array phase_h_guards{
      "phase_h_macos_configuration_structure_test.cpp",
      "phase_h_cocoa_lifecycle_structure_test.cpp",
      "phase_h_metal_bootstrap_structure_test.cpp",
      "phase_h_metal_primitives_structure_test.cpp",
      "phase_h_macos_input_structure_test.cpp",
      "phase_h_macos_text_structure_test.cpp",
      "phase_h_macos_clipboard_drag_structure_test.cpp",
      "phase_h_macos_services_structure_test.cpp",
      "phase_h_macos_accessibility_structure_test.cpp",
      "phase_h_macos_example_smoke_structure_test.cpp",
      "phase_h_macos_full_debug_verification_structure_test.cpp",
  };
  for (const char* guard : phase_h_guards) {
    if (!contains(structure, guard)) return 3;
  }
  if (!contains(manifest,
                "phase_h_final_closeout_structure_test|tests/architecture/"
                "phase_h_final_closeout_structure_test.cpp") ||
      line_count(self) > 120 || line_count(api) > 260 ||
      line_count(read_source("tests/architecture/mcpp.toml")) > 240) {
    return 4;
  }

  if (!contains(ledger_json, "\"phase_h_status\"") ||
      !contains(ledger_json, "\"phase_h_required_macos_gaps\": 0")) {
    return 5;
  }
  return 0;
}
