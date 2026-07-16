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

std::string section_between(const std::string& text, std::string_view begin,
                            std::string_view end) {
  const std::size_t start = text.find(begin);
  const std::size_t finish = text.find(end, start);
  if (start == std::string::npos || finish == std::string::npos) return {};
  return text.substr(start, finish - start);
}

std::string line_containing(const std::string& text, std::string_view anchor) {
  const std::size_t index = text.find(anchor);
  if (index == std::string::npos) return {};
  const std::size_t begin = text.rfind('\n', index);
  const std::size_t end = text.find('\n', index);
  return text.substr(begin == std::string::npos ? 0 : begin + 1,
                     end == std::string::npos ? std::string::npos
                                              : end - (begin + 1));
}

std::string json_row(const std::string& text, std::string_view name) {
  const std::string anchor = "\"upstream_gpui\": \"" + std::string{name} + "\"";
  const std::size_t index = text.find(anchor);
  if (index == std::string::npos) return {};
  const std::size_t begin = text.rfind("    {", index);
  const std::size_t end = text.find("\n    }", index);
  if (begin == std::string::npos || end == std::string::npos) return {};
  return text.substr(begin, end - begin);
}

}  // namespace

int main() {
  const std::string phase_h =
      read_source("tests/api_parity/phase_h_final_closeout_test.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string api_parity =
      read_source("docs/gpui-core-api-parity.md");
  const std::string mac_readiness =
      read_source("docs/platform-mac-readiness.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string progress = read_source("progress.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&phase_h,      &roadmap,  &ledger_md,
                                &ledger_json,  &api_parity,
                                &mac_readiness, &task_plan, &progress,
                                &findings};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  constexpr std::array bands{
      "- [x] Steps 759-764: Add X11/XCB platform boundary",
      "- [x] Steps 765-770: Implement X11 window lifecycle",
      "- [x] Steps 771-776: Implement X11 input:",
      "- [x] Steps 777-782: Implement X11 clipboard and drag/drop",
      "- [x] Steps 783-788: Implement X11 menus/dialog service policy",
      "- [x] Steps 789-794: Run Linux matrix verification",
      "- [x] Steps 795-798: Update the parity ledger",
  };
  for (std::size_t index = 0; index < bands.size(); ++index) {
    if (!contains(roadmap, bands[index])) return 10 + static_cast<int>(index);
  }

  const std::string x11_md = line_containing(
      ledger_md, "| gpui_platform x11 feature | X11 + Vulkan |");
  const std::string x11_json =
      json_row(ledger_json, "gpui_platform x11 feature");
  const std::string platform_md =
      section_between(ledger_md, "## Platform Targets", "## Scope Guard");
  const std::string platform_json = section_between(
      ledger_json, "\"platform_targets\": {", "\"status_counts\": {");
  if (!contains(x11_md, "| Adapted |") ||
      !contains(x11_md, "| Phase I closed |") ||
      !contains(x11_json, "\"status\": \"adapted\"") ||
      !contains(x11_json, "\"next_step\": \"Phase I closed\"") ||
      !contains(ledger_json, "\"required\": 0") ||
      !contains(ledger_json, "\"adapted\": 31") ||
      !contains(ledger_json, "\"deferred\": 0") ||
      !contains(ledger_json, "\"non_goal\": 1") ||
      !contains(platform_md, "| gpui_platform x11 feature | Required |") ||
      !contains(platform_json,
                "\"x11\": {\n      \"backend\": \"x11\",\n      "
                "\"renderer\": \"vulkan\",\n      \"status\": \"required\"")) {
    return 30;
  }

  constexpr std::string_view mac_evidence =
      "Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84), "
      "Xcode 26.6 (17F113), and Xmake 3.0.9+HEAD.2b184e178, including "
      "native Cocoa, Metal primitive/clip/text-image pixel capture, "
      "accessibility, and public-example smoke coverage.";
  if (!contains(phase_h,
                "Phase H macOS full debug passes 380/380 on macOS 26.5.2 ") ||
      !contains(phase_h,
                "native Cocoa, Metal primitive/clip/text-image pixel capture, ") ||
      !contains(phase_h,
                "accessibility, and public-example smoke coverage.") ||
      !contains(roadmap, mac_evidence) || !contains(ledger_md, mac_evidence) ||
      !contains(ledger_json, mac_evidence)) {
    return 40;
  }

  constexpr std::string_view completion =
      "Phase I Step 798 completes strict Linux backend parity";
  constexpr std::string_view gap_summary = "Phase I required X11 gaps: 0";
  constexpr std::string_view handoff =
      "Phase J Step 799 re-run upstream";
  constexpr std::string_view handoff_json =
      "Phase J Step 799 re-run upstream extractor against the pinned revision";
  if (!contains(ledger_json, "\"phase_i_status\": \"complete\"") ||
      !contains(ledger_json, "\"phase_i_required_x11_gaps\": 0") ||
      !contains(ledger_json, "\"phase_i_current_handoff\": \"" +
                                 std::string{handoff_json} + "\"") ||
      !contains(ledger_json, "\"phase_f_current_handoff\": \"" +
                                 std::string{handoff_json} + "\"")) {
    return 50;
  }
  for (const auto* document :
       std::array{&roadmap, &ledger_md, &ledger_json, &api_parity,
                  &mac_readiness, &task_plan, &progress, &findings}) {
    if (!contains(*document, completion) || !contains(*document, gap_summary) ||
        !contains(*document, handoff)) {
      return 51;
    }
  }
  if (!contains(task_plan,
                "- Status: complete\n- Authoritative scope: Phase I") ||
      !contains(api_parity,
                "Linux provides Wayland and X11 platform backends") ||
      !contains(mac_readiness, "X11: XCB + Vulkan, completed in Phase I") ||
      !contains(roadmap,
                "Phase J Step 799 re-runs the upstream extractor") ||
      contains(api_parity, "Phase I remains optional") ||
      contains(roadmap, "- [ ] Steps 789-794:") ||
      contains(roadmap, "- [ ] Steps 795-798:")) {
    return 52;
  }
  return 0;
}
