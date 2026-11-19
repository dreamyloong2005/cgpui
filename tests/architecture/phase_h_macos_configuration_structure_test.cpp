#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

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

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t count(const std::string& text, const char* value) {
  std::size_t result = 0;
  std::size_t offset = 0;
  while ((offset = text.find(value, offset)) != std::string::npos) {
    ++result;
    offset += std::char_traits<char>::length(value);
  }
  return result;
}

// The Windows / Linux test table of a suite manifest: the tests that are not
// built on macOS (Vulkan).
std::string vulkan_test_matrix(const std::string& manifest) {
  constexpr const char* guard =
      "[target.'cfg(any(os = \"windows\", os = \"linux\"))'.test]";
  const auto begin = manifest.find(guard);
  if (begin == std::string::npos) return {};
  const auto end = manifest.find("\n]\n", begin);
  if (end == std::string::npos) return {};
  return manifest.substr(begin, end + 3 - begin);
}

} // namespace

int main() {
  const std::string manifests = read_source("mcpp.toml");
  const std::string api_parity = read_source("tests/api_parity/mcpp.toml");
  if (manifests.empty() || api_parity.empty()) return 1;

  constexpr const char* production_target =
      "\"phase_e_glyph_atlas_production_test.cpp\"";
  if (count(api_parity, production_target) != 1) return 2;

  const std::string matrix = vulkan_test_matrix(api_parity);
  if (matrix.empty() || !contains(matrix, production_target)) return 3;
  if (!contains(api_parity,
                "cgpui.renderer-vulkan = { path = \"../../packages/renderer-vulkan\" }")) {
    return 4;
  }

  constexpr const char* structure_target =
      "\"phase_h_macos_configuration_structure_test.cpp\"";
  const std::string module = read_source("tests/architecture/mcpp.toml");
  if (!contains(manifests, "\"tests/architecture\",") ||
      module.empty() || !contains(module, structure_target) ||
      count(module, structure_target) != 1 ||
      !contains(module,
                "\"phase_h_macos_configuration_structure_test.cpp\"")) {
    return 6;
  }
  return 0;
}
