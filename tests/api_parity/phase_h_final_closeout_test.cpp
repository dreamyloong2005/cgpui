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

}  // namespace

int main() {
  const std::string phase_g =
      read_source("tests/api_parity/phase_g_final_closeout_test.cpp");
  const std::string mac_guard = read_source(
      "tests/architecture/phase_h_macos_full_debug_verification_structure_test.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string api_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string mac_readiness = read_source("docs/platform-mac-readiness.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string progress = read_source("progress.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &phase_g,      &mac_guard,   &roadmap,      &ledger_md,
      &ledger_json,  &api_parity,  &mac_readiness, &task_plan,
      &progress,     &findings,
  };
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  constexpr std::array bands{
      "- [x] Steps 679-686: Add Cocoa application and NSWindow lifecycle",
      "- [x] Steps 687-694: Add CAMetalLayer and Metal renderer bootstrap",
      "- [x] Steps 695-702: Port renderer primitives to Metal",
      "- [x] Steps 703-710: Add macOS input",
      "- [x] Steps 711-718: Add macOS text and fonts",
      "- [x] Steps 719-726: Add macOS clipboard and drag/drop",
      "- [x] Steps 727-734: Add macOS menus, accelerators, file dialogs",
      "- [x] Steps 735-742: Add macOS accessibility",
      "- [x] Steps 743-750: Port all public examples and smoke tests to macOS",
      "- [x] Steps 751-758: Run macOS full debug and close Phase H",
  };
  for (std::size_t index = 0; index < bands.size(); ++index) {
    if (!contains(roadmap, bands[index])) return 10 + static_cast<int>(index);
  }

  constexpr std::string_view mac_evidence =
      "Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84), "
      "Xcode 26.6 (17F113), and Xmake 3.0.9+HEAD.2b184e178, including "
      "native Cocoa, Metal primitive/clip/text-image pixel capture, "
      "accessibility, and public-example smoke coverage.";
  if (!contains(mac_guard,
                "Phase H macOS full debug passes 380/380 on macOS 26.5.2 ") ||
      !contains(mac_guard,
                "native Cocoa, Metal primitive/clip/text-image pixel capture, ") ||
      !contains(mac_guard,
                "accessibility, and public-example smoke coverage.")) {
    return 30;
  }
  for (const auto* document : std::array{
           &roadmap, &ledger_md, &ledger_json, &api_parity,
           &mac_readiness, &task_plan, &progress, &findings}) {
    if (!contains(*document, mac_evidence)) {
      return 31;
    }
  }

  constexpr std::string_view historical_handoff =
      "Phase H required macOS gaps: 0. Phase I Step 759 X11/XCB platform "
      "boundary.";
  if (!contains(ledger_json, "\"phase_h_status\": \"complete\"") ||
      !contains(ledger_json, "\"phase_h_required_macos_gaps\": 0")) {
    return 50;
  }
  for (const auto* document : std::array{
           &roadmap, &ledger_md, &api_parity, &mac_readiness,
           &task_plan, &progress, &findings}) {
    if (!contains(*document, "Phase H required macOS gaps: 0") ||
        !contains(*document, historical_handoff)) {
      return 51;
    }
  }
  if (contains(api_parity, "macOS Cocoa + Metal remains a later parity track") ||
      contains(mac_readiness, "not a full macOS parity port") ||
      contains(mac_readiness, "Full Cocoa event parity")) {
    return 52;
  }

  constexpr std::string_view phase_g_completion =
      "Phase G Step 678 completes final closeout for Steps 619-678";
  if (!contains(phase_g, phase_g_completion) ||
      !contains(roadmap, phase_g_completion) ||
      !contains(ledger_md, phase_g_completion) ||
      !contains(ledger_json, phase_g_completion)) {
    return 60;
  }
  return 0;
}
