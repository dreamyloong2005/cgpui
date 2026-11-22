#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string self = read_source(
      "tests/architecture/phase_g_final_dual_host_verification_structure_test.cpp");
  const std::string candidate =
      read_source("tests/api_parity/phase_g_candidate_ledger_closeout_test.cpp");
  const std::string manifests = read_source("tests/architecture/mcpp.toml") +
      read_source("third_party/VENDORED.md") +
      read_source("tests/renderer/mcpp.toml");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &self,       &candidate, &manifests,
      &manifest,   &ledger_json};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(manifests,
                "\"phase_g_final_dual_host_verification_structure_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_final_dual_host_verification_structure_test.cpp\"") ||
      !contains(manifest,
                "phase_g_final_dual_host_verification_structure_test|"
                "tests/architecture/phase_g_final_dual_host_verification_structure_test.cpp")) {
    return 2;
  }
  if (!contains(candidate, "phase_g_step_676_remaining_gap")) {
    return 3;
  }
  if (!contains(manifests, "Source archive SHA-256") ||
      !contains(manifests, "\"wayland_frame_pixel_capture_test.cpp\"") ||
      !contains(manifests, "\"wayland_vulkan_surface_test.cpp\"")) {
    return 4;
  }

  if (!contains(ledger_json, "\"phase_g_step_677_sources\"")) {
    return 6;
  }
  if (line_count(self) > 130 || line_count(read_source("mcpp.toml")) > 100) {
    return 7;
  }
  return 0;
}
