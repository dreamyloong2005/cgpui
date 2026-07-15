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

std::size_t count(const std::string& text, const char* value) {
  std::size_t result = 0;
  std::size_t offset = 0;
  while ((offset = text.find(value, offset)) != std::string::npos) {
    ++result;
    offset += std::char_traits<char>::length(value);
  }
  return result;
}

std::string vulkan_test_matrix(const std::string& xmake) {
  constexpr const char* guard = "if is_plat(\"windows\", \"linux\") then";
  constexpr const char* first_target =
      "target(\"vulkan_frame_geometry_buffer_test\")";
  const auto first = xmake.find(first_target);
  if (first == std::string::npos) return {};
  const auto begin = xmake.rfind(guard, first);
  const auto end = xmake.find("\nend\n", first);
  if (begin == std::string::npos || end == std::string::npos) return {};
  return xmake.substr(begin, end + 5 - begin);
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  if (xmake.empty()) return 1;

  constexpr const char* production_target =
      "target(\"phase_e_glyph_atlas_production_test\")";
  if (count(xmake, production_target) != 1) return 2;

  const std::string matrix = vulkan_test_matrix(xmake);
  if (matrix.empty() || !contains(matrix, production_target)) return 3;
  if (!contains(matrix,
                "add_files(\"tests/api_parity/phase_e_glyph_atlas_production_test.cpp\")") ||
      !contains(matrix,
                "add_deps(\"cgpui_core\", \"cgpui_renderer\", \"cgpui_renderer_vulkan\")")) {
    return 4;
  }

  constexpr const char* structure_target =
      "target(\"phase_h_macos_configuration_structure_test\")";
  const std::string module = read_source("build/xmake/phase_h_structure_targets.lua");
  if (!contains(xmake, "includes(\"build/xmake/phase_h_structure_targets.lua\")") ||
      module.empty() || !contains(module, structure_target) ||
      count(module, structure_target) != 1 ||
      !contains(module,
                "add_files(path.join(os.projectdir(), \"tests/architecture/phase_h_macos_configuration_structure_test.cpp\"))")) {
    return 6;
  }
  return 0;
}
