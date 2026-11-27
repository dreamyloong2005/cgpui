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

}  // namespace

int main() {
  const std::string mac_guard = read_source(
      "tests/architecture/phase_h_macos_full_debug_verification_structure_test.cpp");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string api_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string mac_readiness = read_source("docs/platform-mac-readiness.md");
  const std::string* required[]{
      &mac_guard,
      &ledger_json,
      &api_parity,
      &mac_readiness};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  if (!contains(mac_guard, "cgpui_require_macos_arm64") ||
      !contains(mac_guard, "xcodebuild -version") ||
      !contains(mac_guard, "scripts/ci/macos-debug.sh")) {
    return 30;
  }

  if (!contains(ledger_json, "\"phase_h_status\"") ||
      !contains(ledger_json, "\"phase_h_required_macos_gaps\": 0")) {
    return 50;
  }

  if (contains(api_parity, "macOS Cocoa + Metal remains a later parity track") ||
      contains(mac_readiness, "not a full macOS parity port") ||
      contains(mac_readiness, "Full Cocoa event parity")) {
    return 52;
  }

  return 0;
}
