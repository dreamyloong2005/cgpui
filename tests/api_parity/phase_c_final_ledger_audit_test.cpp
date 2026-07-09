#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::string line_containing(const std::string& text, const char* anchor) {
  const std::size_t anchor_index = text.find(anchor);
  if (anchor_index == std::string::npos) {
    return {};
  }
  const std::size_t line_start = text.rfind('\n', anchor_index);
  const std::size_t line_end = text.find('\n', anchor_index);
  const std::size_t start =
      line_start == std::string::npos ? 0 : line_start + 1;
  const std::size_t end =
      line_end == std::string::npos ? text.size() : line_end;
  return text.substr(start, end - start);
}

std::string json_object_containing(const std::string& text,
                                   const char* anchor) {
  const std::size_t anchor_index = text.find(anchor);
  if (anchor_index == std::string::npos) {
    return {};
  }
  const std::size_t object_start = text.rfind("  {", anchor_index);
  const std::size_t object_end = text.find("\n    }", anchor_index);
  if (object_start == std::string::npos || object_end == std::string::npos) {
    return {};
  }
  return text.substr(object_start, object_end - object_start);
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || vocabulary.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_c_final_ledger_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_c_final_ledger_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 373-378: Run full Windows/WSL verification and "
                "update the ledger") ||
      contains(roadmap,
               "- [ ] Steps 373-378: Run full Windows/WSL verification and "
               "update the ledger") ||
      !contains(roadmap,
                "Phase C Step 373 final element/style/widget ledger audit "
                "closes Phase C") ||
      !contains(roadmap, "Phase D Step 379 text/font shaping")) {
    return 3;
  }

  if (!contains(roadmap, "## User Scope Decision After Phase C") ||
      !contains(roadmap, "Deferred: game-engine-specific integration") ||
      !contains(roadmap, "Android, iOS, and X11") ||
      !contains(roadmap, "Required for the active track") ||
      !contains(roadmap, "## Zero-Cost Abstraction Principle") ||
      !contains(roadmap, "hot runtime paths must not") ||
      !contains(roadmap, "Static fast paths come first") ||
      !contains(roadmap, "virtual dispatch") ||
      !contains(roadmap, "std::function`/heap churn")) {
    return 4;
  }

  constexpr std::array closeout_tests{
      "tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp",
      "tests/api_parity/phase_c_focusable_interactable_audit_test.cpp",
      "tests/api_parity/phase_c_uniform_list_audit_test.cpp",
      "tests/api_parity/phase_c_window_examples_closeout_test.cpp",
      "tests/api_parity/phase_c_svg_image_closeout_test.cpp",
      "tests/api_parity/phase_c_widget_family_structure_test.cpp",
      "tests/api_parity/phase_c_final_ledger_audit_test.cpp",
  };
  for (std::size_t index = 0; index < closeout_tests.size(); ++index) {
    if (!contains(roadmap, closeout_tests[index]) ||
        !contains(ledger_md, closeout_tests[index]) ||
        !contains(ledger_json, closeout_tests[index]) ||
        !contains(vocabulary, closeout_tests[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_md,
                "Phase C Step 373 final element/style/widget ledger audit") ||
      !contains(ledger_json,
                "\"step_373\": \"Phase C Step 373 final element/style/widget "
                "ledger audit\"") ||
      !contains(vocabulary,
                "Phase C Step 373 final element/style/widget ledger audit")) {
    return 30;
  }

  if (!contains(ledger_md, "Phase C element/style/widget band closed") ||
      !contains(ledger_json,
                "\"element_style_widget_status\": \"phase_c_closed\"") ||
      !contains(ledger_json,
                "\"next_step\": \"Phase D Step 379 text/font shaping\"")) {
    return 31;
  }

  const std::string div_row =
      line_containing(ledger_md, "| gpui::div |");
  const std::string div_json =
      json_object_containing(ledger_json, "\"upstream_gpui\": \"gpui::div\"");
  if (div_row.empty() || div_json.empty() ||
      !contains(div_row, "| Adapted |") ||
      !contains(div_row, "Phase C element/style/widget band closed") ||
      !contains(div_json, "\"status\": \"adapted\"") ||
      !contains(div_json,
                "\"next_step\": \"Phase C element/style/widget band closed\"") ||
      contains(div_json, "Phase C Step 355 window/examples widgets")) {
    return 40;
  }

  const std::string uniform_row =
      line_containing(ledger_md, "| gpui uniform_list |");
  const std::string uniform_json = json_object_containing(
      ledger_json, "\"upstream_gpui\": \"gpui uniform_list\"");
  if (uniform_row.empty() || uniform_json.empty() ||
      !contains(uniform_row, "| Adapted |") ||
      !contains(uniform_row, "Phase C uniform-list band closed") ||
      !contains(uniform_json, "\"status\": \"adapted\"") ||
      !contains(uniform_json,
                "\"next_step\": \"Phase C uniform-list band closed\"")) {
    return 41;
  }

  const std::string image_assets_row =
      line_containing(ledger_md, "| gpui image assets |");
  const std::string svg_row = line_containing(ledger_md, "| gpui SVG |");
  if (image_assets_row.empty() || svg_row.empty() ||
      !contains(image_assets_row, "| Required |") ||
      !contains(image_assets_row, "production loading/upload remains later") ||
      !contains(svg_row, "| Required |") ||
      !contains(svg_row, "production decoding/rendering remains later")) {
    return 50;
  }

  constexpr std::array out_of_scope{
      "ClipboardItem",
      "gpui::test",
      "action macro payloads",
      "task priorities",
      "structured task groups",
      "private runtime headers",
      "direct `WindowRuntime` use",
  };
  for (std::size_t index = 0; index < out_of_scope.size(); ++index) {
    if (!contains(roadmap, out_of_scope[index]) ||
        !contains(vocabulary, out_of_scope[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  return 0;
}
