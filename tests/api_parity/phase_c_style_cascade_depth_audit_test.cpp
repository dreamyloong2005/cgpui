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

} // namespace

int main() {
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string xmake = read_source("xmake.lua");

  if (roadmap.empty() || ledger_md.empty() || ledger_json.empty() ||
      xmake.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"phase_c_style_cascade_depth_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap, "- [x] Steps 331-336: Add style cascade depth") ||
      !contains(roadmap,
                "Step 336 closes the Phase C style-cascade depth band") ||
      !contains(roadmap,
                "Step 337 starts the focusable/interactable band") ||
      !contains(roadmap,
                "Step 338 adds tab-order/focus-ring metadata") ||
      !contains(roadmap,
                "Step 339 adds focused click/drag gesture synthesis") ||
      !contains(roadmap,
                "Step 340 adds keyboard activation semantics") ||
      !contains(roadmap,
                "Step 341 adds disabled interaction semantics") ||
      !contains(roadmap,
                "Step 342 closes the focusable/interactable band") ||
      !contains(roadmap,
                "tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp")) {
    return 3;
  }

  constexpr std::array style_cascade_evidence{
      "StyleState::active",
      "ButtonBuilder::active_style",
      "StyleClassRule",
      "StyleCascade::set_class_rule",
      "StyleThemeTokens",
      "style_theme_cascade.cpp",
      "StyleAuthoredTextFields",
      "text_style_inheritance.cpp",
      "WindowRuntime::request_style_state_invalidation",
      "runtime_style_invalidation.cpp",
  };
  for (std::size_t index = 0; index < style_cascade_evidence.size(); ++index) {
    if (!contains(roadmap, style_cascade_evidence[index]) ||
        !contains(ledger_md, style_cascade_evidence[index]) ||
        !contains(ledger_json, style_cascade_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_md,
                "tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp") ||
      !contains(
          ledger_json,
          "tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp") ||
      !contains(ledger_md,
                "Phase C Step 336 style cascade depth closeout") ||
      !contains(ledger_json,
                "Phase C Step 336 style cascade depth closeout") ||
      !contains(ledger_md,
                "Phase C Step 337 pointer-active input semantics") ||
      !contains(ledger_json,
                "Phase C Step 337 pointer-active input semantics") ||
      !contains(ledger_md,
                "Phase C Step 338 tab-order/focus-ring metadata") ||
      !contains(ledger_json,
                "Phase C Step 338 tab-order/focus-ring metadata") ||
      !contains(ledger_md,
                "Phase C Step 339 click/drag gesture synthesis") ||
      !contains(ledger_json,
                "Phase C Step 339 click/drag gesture synthesis")) {
    return 30;
  }
  if (!contains(ledger_md,
                "Phase C Step 340 keyboard activation semantics") ||
      !contains(ledger_json,
                "Phase C Step 340 keyboard activation semantics") ||
      !contains(ledger_md,
                "should_dispatch_synthesized_keyboard_activation_event") ||
      !contains(ledger_json,
                "should_dispatch_synthesized_keyboard_activation_event")) {
    return 31;
  }
  if (!contains(ledger_md,
                "Phase C Step 341 disabled interaction semantics") ||
      !contains(ledger_json,
                "Phase C Step 341 disabled interaction semantics") ||
      !contains(ledger_md,
                "WindowRuntime::refresh_disabled_interaction_state") ||
      !contains(ledger_json,
                "WindowRuntime::refresh_disabled_interaction_state") ||
      !contains(ledger_md, "runtime_disabled_interaction.cpp") ||
      !contains(ledger_json, "runtime_disabled_interaction.cpp")) {
    return 32;
  }

  constexpr std::array exclusions{
      "runtime theme switching",
      "widget behavior",
      "click/drag gestures",
      "keyboard activation",
      "broad resolved-style",
      "layout/paint rewrites",
      "ClipboardItem",
      "gpui::test",
      "action macro payloads",
      "task priorities",
      "structured task groups",
  };
  for (std::size_t index = 0; index < exclusions.size(); ++index) {
    if (!contains(roadmap, exclusions[index])) {
      return 50 + static_cast<int>(index);
    }
  }

  return 0;
}
