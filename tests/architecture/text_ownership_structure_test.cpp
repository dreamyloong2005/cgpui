#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

namespace {
std::string read(const std::filesystem::path& path) {
  std::ifstream stream(path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
int fail(const std::filesystem::path& path, const char* reason) {
  std::cerr << path.generic_string() << ": " << reason << '\n';
  return 1;
}
} // namespace

int main() {
  namespace fs = std::filesystem;
  const fs::path root{CGPUI_SOURCE_ROOT};
  for (const auto* directory : {"include/cgpui/core", "include/cgpui/platform",
           "include/cgpui/renderer", "src/platform", "src/renderer"}) {
    for (const auto& entry : fs::recursive_directory_iterator(root / directory)) {
      const auto extension = entry.path().extension();
      if (!entry.is_regular_file() ||
          (extension != ".hpp" && extension != ".cpp" && extension != ".mm")) continue;
      if (read(entry.path()).find("\"cgpui/ui/") != std::string::npos) {
        return fail(entry.path(), "lower layers must not include UI compatibility headers");
      }
    }
  }
  const std::array leaves{
      "text_font_defaults", "text_shape", "text_shaping_backend", "text_glyphs",
      "text_measurement", "text_paragraph_layout", "text_wrapping", "text_hit_testing",
      "text_shape_cache", "text_glyph_rasterizer", "text_layout", "text_rich_text",
      "text_rich_text_inline_image"};
  for (const auto* name : leaves) {
    const std::string file = std::string{name} + ".hpp";
    const auto owner = root / "include/cgpui/renderer/text" / file;
    const auto facade = root / "include/cgpui/ui" / file;
    const auto source = read(facade);
    if (read(owner).empty() || source.find("cgpui/renderer/text/" + file) == std::string::npos ||
        std::count(source.begin(), source.end(), '\n') > 5) {
      return fail(facade, "compatibility headers must remain thin and reach the owning leaf");
    }
  }
  for (const auto& entry : fs::directory_iterator(root / "src/renderer/text")) {
    if (fs::exists(root / "src/ui" / entry.path().filename())) {
      return fail(entry.path(), "text implementations must have one physical owner");
    }
  }
  for (const auto* package : {"platform", "renderer"}) {
    const auto path = root / "packages" / package / "mcpp.toml";
    if (read(path).find("../../src/ui/") != std::string::npos) {
      return fail(path, "lower packages must compile sources from their owning modules");
    }
  }
  if (read(root / "packages/ui/mcpp.toml").find("!../../src/ui/text") != std::string::npos ||
      !fs::is_regular_file(root / "src/platform/font_database.cpp") ||
      fs::exists(root / "src/ui/text_font.cpp")) return 1;
  const std::array budgets{
      std::pair{"core/font.hpp", 70}, std::pair{"core/font_face.hpp", 55},
      std::pair{"core/paint_values.hpp", 60}, std::pair{"core/pixel_scale.hpp", 75},
      std::pair{"platform/font_database.hpp", 90}};
  for (const auto& [path, budget] : budgets) {
    const auto source = read(root / "include/cgpui" / path);
    if (source.empty() || std::count(source.begin(), source.end(), '\n') > budget) {
      return fail(path, "shared text/value leaves must stay focused");
    }
  }
  return 0;
}
