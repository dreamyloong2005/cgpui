#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  std::string text = source
                         ? std::string{std::istreambuf_iterator<char>(source), {}}
                         : std::string{};
  text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
  return text;
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
  const std::string manifests = read_source("examples/hello_window/mcpp.toml");
  const std::string module = read_source("tests/platform/mcpp.toml");
  const std::string targets = read_source("tests/architecture/mcpp.toml");
  const std::string workflow = read_source(".github/workflows/phase-h-macos.yml");
  const std::vector<std::string> expected = lines(canonical);
  const std::vector<std::string> actual = lines(macos);
  if (expected.size() != 21 || actual != expected ||
      std::set<std::string>(actual.begin(), actual.end()).size() != actual.size()) {
    return 1;
  }
  const std::string api_parity = read_source("examples/api_parity/mcpp.toml");
  for (const std::string& example : actual) {
    const std::string declaration = "[targets." + example + "]";
    if (api_parity.find(declaration) == std::string::npos) return 2;
  }
  if (!contains(script, "set -euo pipefail") ||
      !contains(script, "macos-example-targets.txt") ||
      !contains(script, "\"$mcpp\" run -p examples/api_parity") ||
      !contains(script, "\"$mcpp\" test -p tests/platform macos_example_smoke_test") ||
      !contains(script, "macos_first_frame") ||
      !contains(script, "macos_resize_after_first_frame") ||
      !contains(script, "macos_demo_smoke_flow") ||
      !contains(script, "macos_close_after_first_frame")) {
    return 3;
  }
  // hello_window's macOS smoke cases are its own test programs; the public
  // hello_world example has none.
  const std::string hello = manifests;
  if (!contains(manifests, "cgpui.platform-macos") ||
      !contains(manifests, "cgpui.renderer-metal") ||
      contains(api_parity, "macos_first_frame") ||
      !contains(hello, "\"tests/macos_first_frame.cpp\"") ||
      !contains(hello, "\"tests/macos_resize_after_first_frame.cpp\"") ||
      !contains(hello, "\"tests/macos_demo_smoke_flow.cpp\"") ||
      !contains(hello, "\"tests/macos_close_after_first_frame.cpp\"")) {
    return 4;
  }
  if (!contains(module, "\"macos/macos_*.mm\"") ||
      !contains(targets, "\"phase_h_macos_example_smoke_structure_test.cpp\"")) {
    return 5;
  }
  if (!contains(workflow, "runs-on: macos-latest") ||
      !contains(workflow, "scripts/ci/macos-example-smoke.sh") ||
      !contains(workflow, "an mcpp on PATH") ||
      !contains(workflow, "3.0.9")) {
    return 6;
  }
  return 0;
}
