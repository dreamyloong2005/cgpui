#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

}  // namespace

int main() {
  const std::string script = read_source("scripts/ci/macos-debug.sh");
  const std::string xmake = read_source("xmake.lua");
  const std::string targets = read_source("build/xmake/phase_h_structure_targets.lua");
  const std::string manifest = read_source("scripts/ci/architecture-header-targets.txt");
  const std::string workflow = read_source(".github/workflows/phase-h-macos.yml");
  const std::string task_plan = read_source("task_plan.md");
  const std::string progress = read_source("progress.md");
  const std::string findings = read_source("findings.md");
  if (script.empty() || xmake.empty() || targets.empty() || manifest.empty() ||
      workflow.empty()) {
    return 1;
  }
  const std::size_t confinement = script.find("case \"$output_root/\"");
  const std::size_t traversal_rejection =
      script.find("output root must not contain parent traversal");
  const std::size_t output_creation = script.find("mkdir -p \"$output_root\"");
  if (!contains(script, "set -euo pipefail") ||
      !contains(script, "macos-debug.sh requires macOS") ||
      !contains(script, "output root must not contain parent traversal") ||
      !contains(script, "output root must not be the repository root") ||
      !contains(script, "xmake v3.0.9") ||
      !contains(script, "XMAKE_CONFIGDIR") ||
      !contains(script, "XMAKE_GLOBALDIR") ||
      !contains(script, "b9256335e0b6e70808e23dfe71627d8a4dcc0abf") ||
      !contains(script, "build/phase-h-ci/macos-debug") ||
      !contains(script, "sw_vers -productVersion") ||
      !contains(script, "xcodebuild -version") ||
      !contains(script, "xmake f -y -c -m debug -P \"$repo_root\"") ||
      !contains(script, "xmake test -y -P \"$repo_root\"") ||
      !contains(script, "git diff --check") ||
      confinement == std::string::npos ||
      traversal_rejection == std::string::npos ||
      output_creation == std::string::npos ||
      traversal_rejection >= output_creation || confinement >= output_creation) {
    return 2;
  }
  if (!contains(xmake, "set_policy(\"package.requires_lock\", true)") ||
      !contains(targets,
                "target(\"phase_h_macos_full_debug_verification_structure_test\")") ||
      !contains(manifest,
                "phase_h_macos_full_debug_verification_structure_test|") ||
      !contains(workflow, "macos-full-debug:") ||
      !contains(workflow, "scripts/ci/macos-debug.sh")) {
    return 3;
  }
  constexpr const char* evidence =
      "Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84), "
      "Xcode 26.6 (17F113), and Xmake 3.0.9+HEAD.2b184e178, including "
      "native Cocoa, Metal primitive/clip/text-image pixel capture, "
      "accessibility, and public-example smoke coverage.";
  if (!contains(task_plan, evidence) || !contains(progress, evidence) ||
      !contains(findings, evidence)) {
    return 4;
  }
  return 0;
}
