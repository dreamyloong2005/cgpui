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
  const std::string phase_c_audit =
      read_source("tests/api_parity/phase_c_final_ledger_audit_test.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &phase_c_audit, &vocabulary, &manifests, &manifest,
      &ledger_json};
  for (const auto* source : required) if (source->empty()) return 1;

  if (contains(phase_c_audit, "production loading/upload remains later") ||
      contains(phase_c_audit, "production decoding/rendering remains later") ||
      !contains(phase_c_audit, "This closes the public SVG/image") ||
      !contains(phase_c_audit, "front-end authoring band") ||
      !contains(phase_c_audit,
                "while keeping SVG decoding, PNG/JPEG loading, renderer upload,") ||
      !contains(vocabulary, "out of scope for this freeze")) {
    return 2;
  }

  if (!contains(manifests,
                "\"phase_g_windows_full_debug_verification_structure_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_windows_full_debug_verification_structure_test.cpp\"") ||
      !contains(manifest,
                "phase_g_windows_full_debug_verification_structure_test|"
                "tests/architecture/phase_g_windows_full_debug_verification_structure_test.cpp")) {
    return 3;
  }

  if (!contains(ledger_json, "\"phase_g_step_673_sources\"")) {
    return 6;
  }
  if (line_count(read_source("mcpp.toml")) > 100 ||
      line_count(phase_c_audit) > 220) {
    return 7;
  }
  return 0;
}
