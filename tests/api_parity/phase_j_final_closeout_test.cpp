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
       offset = text.find(value, offset + value.size())) ++result;
  return result;
}
}  // namespace

int main() {
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string verification =
      read_source("docs/gpui-phase-j-final-verification.json");
  const std::string drift = read_source("docs/gpui-upstream-drift-policy.md");
  const std::string snapshot = read_source("docs/gpui-upstream-snapshot.json");
  const std::array required{
      &ledger,
      &verification,
      &drift,
      &snapshot};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(ledger, "\"required\": 0") ||
      !contains(ledger, "\"phase_j_status\"") ||
      !contains(ledger, "\"phase_j_steps_835_840_status\"") ||
      !contains(ledger, "\"phase_j_required_non_macos_gaps\": 0")) return 20;

  if (!contains(verification, "\"schema_version\": 1") ||
      !contains(verification, "\"windows_debug\"") ||
      !contains(verification, "\"wslg_wayland_debug\"") ||
      !contains(verification, "\"xvfb_x11_acceptance\"") ||
      !contains(verification, "\"windows_release\"") ||
      !contains(verification, "\"linux_release\"") ||
      !contains(verification, "\"performance\"") ||
      !contains(verification, "\"stress\"")) return 30;
  if (!contains(ledger, "\"phase_j_steps_829_834_status\"") ||
      !contains(ledger, "\"phase_j_macos_completion_status\"") ||
      !contains(ledger, "\"phase_j_required_gaps\": 0") ||
      !contains(ledger, "\"phase_j_required_macos_gaps\": 0") ||
      !contains(verification, "\"scope\": \"all_desktop\"") ||
      !contains(verification, "\"status\": \"passed\"") ||
      count(verification, "\"status\": \"passed_native\"") != 2 ||
      !contains(verification, "29630949183") ||
      !contains(verification, "1acd02b99ffa97b7dd374252b849a058948ea22e") ||
      !contains(verification, "9c29c096c66afcc0d864efdb05b6fc3f7f549bc0534eda0a2d756dc782ef5afa") ||
      !contains(verification, "d0911ddedefe93cd0f97838ff98c44572845b88165d8e92233c970dde7f51b78")) return 30;

  if (!contains(drift, "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0") ||
      !contains(drift, "new parity delta plan") ||
      !contains(drift, "immutable record of that revision") ||
      !contains(drift, "explicit errata that preserve the original evidence")) return 40;
  if (!contains(snapshot,
                "\"upstream_revision\": \"5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0\"") ||
      !contains(snapshot, "\"public_reexport_count\": 55") ||
      !contains(snapshot, "\"example_count\": 20")) return 41;

  return 0;
}
