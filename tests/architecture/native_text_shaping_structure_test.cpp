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
  const char* root = CGPUI_SOURCE_ROOT;
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
    {"include/cgpui/renderer/text/text_shape_cache.hpp", 40},
    {"include/cgpui/renderer/text/text_font_defaults.hpp", 40},
    {"src/renderer/text/text_shaping_native.cpp", 240},
    {"src/renderer/text/text_shaping_native_internal.hpp", 90},
    {"src/renderer/text/text_shaping_unicode.cpp", 110},
    {"src/renderer/text/text_shape_cache.cpp", 230},
    {"src/renderer/text/text_shape_cache_internal.hpp", 30},
    {"src/renderer/text/text_shaping_dispatch.cpp", 70},
    {"src/renderer/text/text_shaping_directwrite.cpp", 240},
    {"src/renderer/text/text_dwrite_win32_internal.hpp", 110},
    {"src/renderer/text/text_dwrite_fonts_win32.cpp", 270},
    {"src/renderer/text/text_dwrite_analysis_win32.cpp", 220},
    {"src/renderer/text/text_glyph_id_raster_internal.hpp", 30},
    {"src/renderer/text/text_glyph_id_raster_win32.cpp", 120},
    {"src/renderer/text/text_glyph_paint_internal.hpp", 30},
    {"src/renderer/text/text_font_defaults.cpp", 70},
    {"src/renderer/text/text_measurement_grapheme.cpp", 210},
    {"src/renderer/text/text_line_metrics.cpp", 90},
    {"src/renderer/text/text_shaping_harfbuzz.cpp", 270},
    {"src/renderer/text/text_fontconfig_linux_internal.hpp", 50},
    {"src/renderer/text/text_fontconfig_faces_linux.cpp", 210},
    {"src/renderer/text/text_glyph_id_raster_freetype.cpp", 110},
    {"src/renderer/renderer_text_glyph_placement.cpp", 80},
    {"src/renderer/renderer_text_glyph_placement_internal.hpp", 40},
    {"src/renderer/text/text_shaping_coretext_macos.cpp", 200},
    {"src/renderer/text/text_coretext_fonts_macos.cpp", 230},
    {"src/renderer/text/text_coretext_macos_internal.hpp", 45},
    {"src/renderer/text/text_glyph_id_raster_coretext.cpp", 100},
};

} // namespace

int main() {
  try {
    for (const auto& [path, limit] : limits) {
      require(lines(read(path)) <= limit, path + " exceeds " + std::to_string(limit) + " lines");
    }
    for (const char* header : {"include/cgpui/core/font.hpp",
                               "include/cgpui/core/font_face.hpp",
                               "include/cgpui/ui/style_font.hpp",
                               "include/cgpui/renderer/text/text_shape_cache.hpp",
                               "include/cgpui/renderer/text/text_font_defaults.hpp",
                               "include/cgpui/renderer/text/text_shape.hpp",
                               "include/cgpui/renderer/text/text_glyphs.hpp",
                               "include/cgpui/platform/font_database.hpp"}) {
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
    const std::string shaper = read("src/renderer/text/text_shaping_directwrite.cpp");
    const std::string analysis = read("src/renderer/text/text_dwrite_analysis_win32.cpp");
    const std::string fonts = read("src/renderer/text/text_dwrite_fonts_win32.cpp");
    const std::string raster = read("src/renderer/text/text_glyph_id_raster_win32.cpp");
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
    const std::string harfbuzz = read("src/renderer/text/text_shaping_harfbuzz.cpp");
    const std::string fontconfig = read("src/renderer/text/text_fontconfig_faces_linux.cpp");
    const std::string freetype = read("src/renderer/text/text_glyph_id_raster_freetype.cpp");
    require(has(harfbuzz, "hb_shape(") && has(harfbuzz, "fontconfig_fallback_faces(") &&
                has(harfbuzz, "NativeShapeBuilder") && !has(harfbuzz, "FcFontSort"),
            "HarfBuzz shaping must use the fontconfig module and the shared builder");
    require(has(fontconfig, "FcFontSort(") && has(fontconfig, "FcCharSetHasChar(") &&
                has(fontconfig, "FT_New_Face(") && !has(fontconfig, "hb_shape"),
            "fontconfig matching, coverage and FreeType faces belong to one module");
    require(has(freetype, "FT_Load_Glyph(") && has(freetype, "FT_Render_Glyph(") &&
                !has(freetype, "hb_shape"),
            "FreeType rasterizes by glyph id without reshaping");
    const std::string core_text = read("src/renderer/text/text_shaping_coretext_macos.cpp");
    const std::string core_text_fonts = read("src/renderer/text/text_coretext_fonts_macos.cpp");
    const std::string core_text_raster = read("src/renderer/text/text_glyph_id_raster_coretext.cpp");
    require(has(core_text, "CTLineCreateWithAttributedString(") &&
                has(core_text, "NativeShapeBuilder") && !has(core_text, "CTFontDrawGlyphs"),
            "Core Text shaping must use the shared builder and not rasterize");
    require(has(core_text_fonts, "core_text_register_face(") &&
                has(core_text_fonts, "kCTFontFeatureSettingsAttribute"),
            "Core Text face identity and features belong to the fonts module");
    require(has(core_text_raster, "CTFontDrawGlyphs(") &&
                !has(core_text_raster, "CTLineCreate"),
            "Core Text rasterizes by glyph id without reshaping");
    const std::string metal_text = read("src/renderer/metal/metal_text.mm");
    require(has(metal_text, "native_glyph_placement(") &&
                has(metal_text, "rasterize_text_glyph("),
            "Metal text must place native glyphs by glyph id");
    const std::string dispatch = read("src/renderer/text/text_shaping_dispatch.cpp");
    require(has(dispatch, "find_cached_text_shape(") && has(dispatch, "store_cached_text_shape(") &&
                has(dispatch, "unsupported_font_source"),
            "dispatch must consult the shape cache and type test-font fallback");
    const std::string quads = read("src/renderer/planning/glyph_geometry.cpp");
    require(has(quads, "native_glyph_placement(") && has(quads, "cell_glyph_placement("),
            "Vulkan quads must place native glyphs by glyph id");
    const std::string module = read("packages/renderer/mcpp.toml") +
      read("packages/ui/mcpp.toml") +
      read("packages/renderer/build.mcpp");
    require(has(module, "[targets.cgpui_renderer]") && has(module, "[targets.cgpui_ui]") &&
                !has(module, "\"!../../src/ui/text_") && has(module, "\"../../src/renderer/text/text_shaping_directwrite.cpp\"") &&
                has(module, "\"../../src/renderer/text/text_dwrite_analysis_win32.cpp\"") &&
                has(module, "\"harfbuzz\""),
            "native shaping sources must have renderer ownership without mirrored UI exclusions");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native text shaping structure failed: " << error.what() << '\n';
    return 1;
  }
}
