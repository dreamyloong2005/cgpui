#include <cstdlib>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

std::vector<std::string> lines(const std::string& text) {
  std::vector<std::string> result;
  std::istringstream stream{text};
  for (std::string line; std::getline(stream, line);) {
    if (!line.empty() && line.front() != '#') result.push_back(std::move(line));
  }
  return result;
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

}  // namespace

int main() {
  const std::string canonical = read_source("scripts/ci/example-targets.txt");
  const std::string macos = read_source("scripts/ci/macos-example-targets.txt");
  const std::string script = read_source("scripts/ci/macos-example-smoke.sh");
  const std::string xmake = read_source("xmake.lua");
  const std::string module = read_source("build/xmake/phase_h_macos.lua");
  const std::string targets = read_source("build/xmake/phase_h_structure_targets.lua");
  const std::string workflow = read_source(".github/workflows/phase-h-macos.yml");
  const std::vector<std::string> expected = lines(canonical);
  const std::vector<std::string> actual = lines(macos);
  if (expected.size() != 21 || actual != expected ||
      std::set<std::string>(actual.begin(), actual.end()).size() != actual.size()) {
    return 1;
  }
  for (const std::string& example : actual) {
    const std::string declaration = "target(\"" + example + "\")";
    if (xmake.find(declaration) == std::string::npos) return 2;
  }
  if (!contains(script, "set -euo pipefail") ||
      !contains(script, "macos-example-targets.txt") ||
      !contains(script, "xmake -r -P \"$repo_root\"") ||
      !contains(script, "xmake run -P \"$repo_root\"") ||
      !contains(script, "macos_example_smoke_test/default") ||
      !contains(script, "hello_window/macos_first_frame") ||
      !contains(script, "hello_window/macos_resize_after_first_frame") ||
      !contains(script, "hello_window/macos_demo_smoke_flow") ||
      !contains(script, "hello_window/macos_close_after_first_frame")) {
    return 3;
  }
  const std::size_t public_hello_begin =
      xmake.find("target(\"api_parity_hello_world\")");
  const std::size_t public_hello_end =
      xmake.find("target(\"api_parity_public_api_compatibility\")");
  const std::size_t hello_begin = xmake.find("target(\"hello_window\")");
  const std::string public_hello =
      xmake.substr(public_hello_begin, public_hello_end - public_hello_begin);
  const std::string hello = xmake.substr(hello_begin);
  if (!contains(xmake, "add_deps(\"cgpui_platform_macos\", \"cgpui_renderer_metal\")") ||
      !contains(xmake, "add_frameworks(\"AppKit\", \"QuartzCore\", \"Metal\")") ||
      contains(public_hello, "add_tests(\"macos_first_frame\"") ||
      !contains(hello, "add_tests(\"macos_first_frame\"") ||
      !contains(hello, "add_tests(\"macos_resize_after_first_frame\"") ||
      !contains(hello, "add_tests(\"macos_demo_smoke_flow\"") ||
      !contains(hello, "add_tests(\"macos_close_after_first_frame\"")) {
    return 4;
  }
  if (!contains(module, "target(\"macos_example_smoke_test\")") ||
      !contains(targets, "target(\"phase_h_macos_example_smoke_structure_test\")")) {
    return 5;
  }
  if (!contains(workflow, "runs-on: macos-latest") ||
      !contains(workflow, "scripts/ci/macos-example-smoke.sh") ||
      !contains(workflow, "xmake-io/xmake-action") ||
      !contains(workflow, "3.0.9")) {
    return 6;
  }
  return 0;
}
