#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

std::string read(const std::string& path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error("missing focused module: " + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}

void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}

std::size_t lines(const std::string& source) {
  return static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) + 1U;
}

// Line limits for every module the native shaping capability owns.
const std::vector<std::pair<std::string, std::size_t>> limits{
    {"include/cgpui/ui/style_font.hpp", 50},
    {"include/cgpui/ui/text_shape_cache.hpp", 40},
    {"include/cgpui/ui/text_font_defaults.hpp", 40},
    {"src/ui/text_shaping_native.cpp", 240},
    {"src/ui/text_shaping_native_internal.hpp", 90},
    {"src/ui/text_shaping_unicode.cpp", 110},
    {"src/ui/text_shape_cache.cpp", 230},
    {"src/ui/text_shape_cache_internal.hpp", 30},
    {"src/ui/text_shaping_dispatch.cpp", 70},
    {"src/ui/text_shaping_directwrite.cpp", 240},
    {"src/ui/text_dwrite_win32_internal.hpp", 110},
    {"src/ui/text_dwrite_fonts_win32.cpp", 270},
    {"src/ui/text_dwrite_analysis_win32.cpp", 220},
    {"src/ui/text_glyph_id_raster_internal.hpp", 30},
    {"src/ui/text_glyph_id_raster_win32.cpp", 120},
    {"src/ui/text_glyph_paint_internal.hpp", 30},
    {"src/ui/text_font_defaults.cpp", 70},
    {"src/ui/text_measurement_grapheme.cpp", 210},
    {"src/ui/text_line_metrics.cpp", 90},
    {"src/renderer/vulkan/vulkan_text_glyph_placement.cpp", 80},
    {"src/renderer/vulkan/vulkan_text_glyph_placement.hpp", 40},
};

} // namespace

int main() {
  try {
    for (const auto& [path, limit] : limits) {
      require(lines(read(path)) <= limit, path + " exceeds " + std::to_string(limit) + " lines");
    }
    for (const char* header : {"include/cgpui/ui/style_font.hpp",
                               "include/cgpui/ui/text_shape_cache.hpp",
                               "include/cgpui/ui/text_font_defaults.hpp",
                               "include/cgpui/ui/text_shape.hpp",
                               "include/cgpui/ui/text_glyphs.hpp",
                               "include/cgpui/ui/text_font.hpp"}) {
      const std::string source = read(header);
      require(!has(source, "<windows.h>") && !has(source, "dwrite") &&
                  !has(source, "IDWrite") && !has(source, "<hb") &&
                  !has(source, "FT_") && !has(source, "Fc"),
              std::string(header) + " leaks platform text types");
    }
    const std::string aggregate = read("include/cgpui/ui/text.hpp");
    require(has(aggregate, "text_shape_cache.hpp") && has(aggregate, "text_font_defaults.hpp") &&
                lines(aggregate) <= 40,
            "text aggregate must include the new leaves and stay thin");
    const std::string shaper = read("src/ui/text_shaping_directwrite.cpp");
    const std::string analysis = read("src/ui/text_dwrite_analysis_win32.cpp");
    const std::string fonts = read("src/ui/text_dwrite_fonts_win32.cpp");
    const std::string raster = read("src/ui/text_glyph_id_raster_win32.cpp");
    require(has(shaper, "GetGlyphs(") && has(shaper, "GetGlyphPlacements(") &&
                has(shaper, "NativeShapeBuilder") && !has(shaper, "MapCharacters"),
            "DirectWrite shaping must stay in its module and use the shared builder");
    require(has(analysis, "AnalyzeScript(") && has(analysis, "AnalyzeBidi(") &&
                has(analysis, "MapCharacters(") && !has(analysis, "GetGlyphs("),
            "itemization and font fallback belong to the analysis module");
    require(has(fonts, "IDWriteLocalFontFileLoader") && has(fonts, "GetMetrics("),
            "face identity and metrics belong to the fonts module");
    require(has(raster, "CreateGlyphRunAnalysis(") && has(raster, "DWRITE_GLYPH_RUN") &&
                !has(raster, "CreateTextLayout"),
            "glyph-id rasterization must not re-lay out source text");
    const std::string dispatch = read("src/ui/text_shaping_dispatch.cpp");
    require(has(dispatch, "find_cached_text_shape(") && has(dispatch, "store_cached_text_shape(") &&
                has(dispatch, "unsupported_font_source"),
            "dispatch must consult the shape cache and type test-font fallback");
    const std::string quads = read("src/renderer/vulkan/vulkan_report_text_quads.cpp");
    require(has(quads, "native_glyph_placement(") && has(quads, "cell_glyph_placement("),
            "Vulkan quads must place native glyphs by glyph id");
    const std::string module = read("build/xmake/capabilities/native_text_shaping_targets.lua");
    require(has(module, "target(\"cgpui_renderer\")") && has(module, "target(\"cgpui_ui\")") &&
                has(module, "remove_files(") && has(module, "text_shaping_directwrite.cpp") &&
                has(module, "text_dwrite_analysis_win32.cpp") &&
                has(module, "add_requires(\"harfbuzz\", {system = true})"),
            "native shaping sources must be renderer-owned and excluded from cgpui_ui");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native text shaping structure failed: " << error.what() << '\n';
    return 1;
  }
}
