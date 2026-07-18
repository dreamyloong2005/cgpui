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
      read_source("build/xmake/phase_j_example_targets.lua");
  const std::string xmake = read_source("xmake.lua");
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
  const std::string macos =
      read_source("scripts/ci/macos-example-smoke.sh");
  const std::string macos_inventory =
      read_source("scripts/ci/macos-example-targets.txt");
  const std::string auditor =
      read_source("tools/gpui_parity/audit_official_examples.py");
  const std::string* required[]{&self,    &behavior, &module, &xmake,
                                &manifest, &canonical, &windows, &wayland,
                                &x11,      &xvfb,    &macos,  &auditor};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  if (!contains(xmake, "includes(\"build/xmake/phase_j_example_targets.lua\")") ||
      contains(xmake, "target(\"phase_j_official_example_audit_test\")") ||
      !contains(module, "target(\"phase_j_official_example_audit_test\")") ||
      !contains(module, "target(\"phase_j_official_example_structure_test\")") ||
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
      !contains(macos, "scripts/ci/example-targets.txt") ||
      !macos_inventory.empty() ||
      !contains(auditor, "--matrix") || !contains(auditor, "--snapshot") ||
      !contains(auditor, "--output") ||
      !contains(auditor, "(\"windows\", \"wayland\", \"x11\", \"macos\")") ||
      !contains(auditor, "macos_tests")) {
    return 3;
  }
  if (line_count(self) > 120 || line_count(behavior) > 150 ||
      line_count(module) > 30 || line_count(x11) > 130 ||
      line_count(xvfb) > 80 ||
      line_count(auditor) > 180) {
    return 4;
  }
  return 0;
}
