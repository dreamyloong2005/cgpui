#include <array>
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
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string phase_c_final_audit =
      read_source("tests/api_parity/phase_c_final_ledger_audit_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || phase_c_final_audit.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"pre_phase_d_entry_gate_test\")") ||
      !contains(xmake,
                "tests/api_parity/pre_phase_d_entry_gate_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap, "## Pre-Phase-D Entry Gate") ||
      !contains(roadmap, "Phase D feature work may continue only after") ||
      !contains(roadmap, "Windows and WSL Arch Linux")) {
    return 3;
  }

  constexpr std::array deferred_scope{
      "game-engine-specific integration",
      "engine runtime embedding",
      "engine renderer/resource/asset/command systems",
      "game editor/runtime",
      "Android, iOS, and X11",
  };
  for (const char* item : deferred_scope) {
    if (!contains(roadmap, item) || !contains(ledger_md, item)) {
      return 10;
    }
  }

  constexpr std::array required_scope{
      "C++23-native public APIs without Rust FFI",
      "Vulkan-first",
      "production renderer",
      "declarative widgets",
      "reactive state/subscription",
      "low-allocation",
      "runtime structures",
      "static and dynamic widget support",
      "editor/AI-IDE-class",
      "tool UI primitives",
      "Win32 and Wayland production platform behavior",
      "later macOS Cocoa + Metal parity",
  };
  for (const char* item : required_scope) {
    if (!contains(roadmap, item) || !contains(ledger_md, item)) {
      return 20;
    }
  }

  constexpr std::array zero_cost_terms{
      "Static fast paths",
      "dynamic escape hatches",
      "per-frame tree-wide scans",
      "avoid hidden allocation",
      "broad type erasure",
      "avoidable virtual dispatch",
      "std::function`/heap churn",
  };
  for (const char* item : zero_cost_terms) {
    if (!contains(roadmap, item) || !contains(ledger_md, item)) {
      return 30;
    }
  }

  constexpr std::array gate_tests{
      "pre_phase_d_entry_gate_test/default",
      "phase_c_final_ledger_audit_test/default",
      "static_render_runtime_test/default",
      "ui_source_structure_test/default",
      "gpui_parity_ledger_test/default",
  };
  for (const char* item : gate_tests) {
    if (!contains(roadmap, item) || !contains(task_plan, item)) {
      return 40;
    }
  }

  if (!contains(ledger_json, "\"scope_guard\"") ||
      !contains(ledger_json, "\"pre_phase_d_entry_gate_test/default\"") ||
      !contains(ledger_json, "\"deferred_by_user\"") ||
      !contains(ledger_json, "\"active_track_required\"") ||
      !contains(ledger_json, "\"zero_cost_principle\"")) {
    return 50;
  }

  if (!contains(task_plan,
                "Pre-Phase-D entry gate is explicit and test-protected") ||
      !contains(task_plan,
                "future Phase D continuation must rerun the gate")) {
    return 60;
  }

  if (!contains(phase_c_final_audit, "## User Scope Decision After Phase C") ||
      !contains(phase_c_final_audit, "## Zero-Cost Abstraction Principle")) {
    return 70;
  }

  return 0;
}
