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
  const std::string xmake = read_source("xmake.lua");
  const std::string structure =
      read_source("build/xmake/phase_h_structure_targets.lua");
  const std::string closeout =
      read_source("build/xmake/phase_h_closeout_targets.lua");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{&self, &api, &xmake, &structure,
                                &closeout, &manifest, &ledger_json};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  if (!contains(xmake, "includes(\"build/xmake/phase_h_closeout_targets.lua\")") ||
      contains(xmake, "target(\"phase_h_final_closeout_test\")") ||
      contains(xmake, "target(\"phase_h_final_closeout_structure_test\")") ||
      !contains(closeout, "target(\"phase_h_final_closeout_test\")") ||
      !contains(closeout, "tests/api_parity/phase_h_final_closeout_test.cpp") ||
      !contains(closeout,
                "target(\"phase_h_final_closeout_structure_test\")") ||
      !contains(closeout,
                "tests/architecture/phase_h_final_closeout_structure_test.cpp") ||
      !contains(closeout, "CGPUI_SOURCE_ROOT")) {
    return 2;
  }

  constexpr std::array phase_h_guards{
      "phase_h_macos_configuration_structure_test",
      "phase_h_cocoa_lifecycle_structure_test",
      "phase_h_metal_bootstrap_structure_test",
      "phase_h_metal_primitives_structure_test",
      "phase_h_macos_input_structure_test",
      "phase_h_macos_text_structure_test",
      "phase_h_macos_clipboard_drag_structure_test",
      "phase_h_macos_services_structure_test",
      "phase_h_macos_accessibility_structure_test",
      "phase_h_macos_example_smoke_structure_test",
      "phase_h_macos_full_debug_verification_structure_test",
  };
  for (const char* guard : phase_h_guards) {
    if (!contains(structure, guard)) return 3;
  }
  if (!contains(manifest,
                "phase_h_final_closeout_structure_test|tests/architecture/"
                "phase_h_final_closeout_structure_test.cpp") ||
      line_count(self) > 120 || line_count(api) > 260 ||
      line_count(closeout) > 30) {
    return 4;
  }

  if (!contains(ledger_json, "\"phase_h_status\": \"complete\"") ||
      !contains(ledger_json, "\"phase_h_required_macos_gaps\": 0") ||
      !contains(ledger_json,
                "\"phase_h_current_handoff\": \"Phase I Step 759 X11/XCB "
                "platform boundary\"")) {
    return 5;
  }
  return 0;
}
