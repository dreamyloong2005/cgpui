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

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

}  // namespace

int main() {
  const std::string self = read_source(
      "tests/architecture/phase_j_official_example_structure_test.cpp");
  const std::string behavior = read_source(
      "tests/api_parity/phase_j_official_example_audit_test.cpp");
  const std::string module =
      read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/api_parity/mcpp.toml");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string canonical =
      read_source("scripts/ci/example-targets.txt");
  const std::string windows =
      read_source("scripts/ci/windows-example-smoke.ps1");
  const std::string wayland =
      read_source("scripts/ci/linux-example-smoke.sh");
  const std::string x11 =
      read_source("scripts/ci/linux-x11-example-smoke.sh");
  const std::string xvfb =
      read_source("scripts/ci/linux-x11-xvfb-example-smoke.sh");
  const std::string auditor =
      read_source("tools/gpui_parity/audit_official_examples.py");
  const std::string* required[]{&self, &behavior, &module, &manifests, &manifest,
                                &canonical, &windows, &wayland, &x11, &xvfb,
                                &auditor};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  if (!contains(manifests, "\"tests/architecture\",") ||
      contains(read_source("mcpp.toml"), "\"phase_j_official_example_audit_test.cpp\"") ||
      !contains(module, "\"phase_j_official_example_audit_test.cpp\"") ||
      !contains(module, "\"phase_j_official_example_structure_test.cpp\"") ||
      !contains(manifest,
                "phase_j_official_example_structure_test|tests/architecture/"
                "phase_j_official_example_structure_test.cpp")) {
    return 2;
  }
  if (!contains(windows, "example-targets.txt") ||
      !contains(wayland, "example-targets.txt") ||
      !contains(wayland, "CGPUI_CI_USE_CURRENT_BUILD") ||
      !contains(wayland, "CGPUI_CI_SKIP_BUILD") ||
      !contains(x11, "example-targets.txt") ||
      !contains(x11, "CGPUI_CI_SKIP_BUILD") ||
      !contains(x11, "CGPUI_LINUX_BACKEND=x11") ||
      !contains(x11, "x11_demo_smoke_flow") ||
      !contains(x11, "hello_window/$test_name") ||
      !contains(xvfb, "unshare --mount") || !contains(xvfb, "Xvfb") ||
      !contains(xvfb, "linux-x11-example-smoke.sh") ||
      !contains(auditor, "--matrix") || !contains(auditor, "--snapshot") ||
      !contains(auditor, "--output")) {
    return 3;
  }
  if (line_count(self) > 100 || line_count(behavior) > 150 ||
      line_count(read_source("tests/architecture/mcpp.toml")) > 240 || line_count(x11) > 130 ||
      line_count(xvfb) > 80 ||
      line_count(auditor) > 180) {
    return 4;
  }
  return 0;
}
