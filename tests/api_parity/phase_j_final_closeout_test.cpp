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
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string verification =
      read_source("docs/gpui-phase-j-final-verification.json");
  const std::string drift = read_source("docs/gpui-upstream-drift-policy.md");
  const std::string snapshot = read_source("docs/gpui-upstream-snapshot.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::array required{&roadmap, &ledger, &ledger_md, &verification,
                            &drift, &snapshot, &task_plan};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array bands{
      "- [x] Steps 799-804: Re-run upstream extractor",
      "- [x] Steps 805-810: Compile every official example equivalent",
      "- [x] Steps 811-816: Add performance baselines:",
      "- [x] Steps 817-822: Add stress tests:",
      "- [x] Steps 823-828: Complete documentation:",
      "- [x] Steps 829-834: Package Windows/Linux builds",
      "- [x] Steps 835-840: Run final non-macOS matrix verification"};
  for (std::size_t index = 0; index < bands.size(); ++index)
    if (!contains(roadmap, bands[index])) return 10 + static_cast<int>(index);

  if (!contains(ledger, "\"required\": 0") ||
      !contains(ledger, "\"phase_j_status\": \"complete_non_macos\"") ||
      !contains(ledger, "\"phase_j_steps_835_840_status\": \"complete_non_macos\"") ||
      !contains(ledger, "\"phase_j_required_non_macos_gaps\": 0") ||
      !contains(ledger, "Phase J Steps 841+ upstream drift delta plan")) return 20;
  if (!contains(ledger_md, "Phase J Step 840 completes non-macOS") ||
      !contains(task_plan, "- Status: complete_non_macos") ||
      !contains(task_plan, "7. [x] Steps 835-840+")) return 21;

  if (!contains(verification, "\"schema_version\": 1") ||
      !contains(verification, "\"windows_debug\"") ||
      !contains(verification, "\"wslg_wayland_debug\"") ||
      !contains(verification, "\"xvfb_x11_acceptance\"") ||
      !contains(verification, "\"windows_release\"") ||
      !contains(verification, "\"linux_release\"") ||
      !contains(verification, "\"performance\"") ||
      !contains(verification, "\"stress\"") ||
      !contains(verification, "\"macos_execution\": \"excluded_by_user\"") ||
      !contains(verification, "\"status\": \"passed\"")) return 30;

  if (!contains(drift, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      !contains(drift, "Steps 841+") ||
      !contains(drift, "new parity delta plan") ||
      !contains(drift, "must not mutate Phase J")) return 40;
  if (!contains(snapshot,
                "\"upstream_revision\": \"5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0\"") ||
      !contains(snapshot, "\"public_reexport_count\": 55") ||
      !contains(snapshot, "\"example_count\": 20")) return 41;

  constexpr std::string_view mac_evidence =
      "Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84)";
  if (!contains(roadmap, mac_evidence) || !contains(ledger, mac_evidence) ||
      !contains(ledger_md, mac_evidence)) return 50;
  return 0;
}
