#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  const auto root = CGPUI_SOURCE_ROOT;
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error(std::string("missing focused module: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}
std::size_t lines(const std::string& source) {
  return std::count(source.begin(), source.end(), '\n') + 1;
}
} // namespace

int main() {
  try {
    const auto leaf = read("include/cgpui/renderer/text/text_glyph_rasterizer.hpp");
    const auto aggregate = read("include/cgpui/ui/text.hpp");
    const auto dispatch = read("src/renderer/text/text_glyph_rasterizer.cpp");
    const auto native = read("src/renderer/text/text_glyph_raster_win32.cpp");
    const auto coverage = read("src/renderer/text/text_glyph_coverage_win32.cpp");
    const auto native_header = read("src/renderer/text/text_glyph_raster_win32_internal.hpp");
    const auto coverage_header = read("src/renderer/text/text_glyph_coverage_win32_internal.hpp");
    const auto production = read("src/renderer/planning/glyph_geometry.cpp");
    const auto build = read("mcpp.toml");
    const auto pixel_shape = read("tests/renderer/renderer_text_pixel_shape_support.hpp");
    const auto pixel_shape_source = read("tests/renderer/renderer_text_pixel_shape_support.cpp");
    require(lines(pixel_shape) + lines(pixel_shape_source) <= 60 &&
                has(pixel_shape, "text_ink_shape(") && has(pixel_shape_source, "text_ink_shape(") &&
                has(pixel_shape_source, "pixels.pixel_rgba8(") && !has(pixel_shape, "pixels.pixel_rgba8("),
            "pixel glyph inspection keeps its original combined cap and owns non-template work in a cpp");
    require(has(leaf, "Result<RasterizedGlyph>") && has(leaf, "rasterize_text_glyph(") &&
            lines(leaf) <= 50 && !has(leaf, "<windows.h>") && !has(leaf, "<dwrite.h>"),
            "public rasterizer leaf must expose the result API without Windows implementation");
    require(lines(aggregate) <= 40 && !has(aggregate, "CreateGlyphRunAnalysis") &&
            !has(aggregate, "class CoverageRenderer"), "text aggregate grew native implementation details");
    require(lines(dispatch) <= 100 && has(dispatch, "_WIN32") &&
            has(dispatch, "rasterize_fallback_glyph(") && !has(dispatch, "<dwrite.h>"),
            "platform dispatch must stay focused and preserve the explicit non-Windows fallback");
    require(lines(native) <= 320 && lines(coverage) <= 250 &&
            lines(native_header) <= 50 && lines(coverage_header) <= 100,
            "native rasterization exceeded the focused layout/coverage module boundaries");
    require(has(native, "CreateTextLayout") && !has(native, "CreateAlphaTexture") &&
            has(coverage, "CreateGlyphRunAnalysis") && has(coverage, "CreateAlphaTexture"),
            "DirectWrite layout and coverage extraction must remain separate modules");
    require(has(production, "rasterize_text_glyph(") && !has(production, "rasterize_fallback_glyph("),
            "Vulkan production glyph preparation bypassed the native rasterizer");
    // Native rasterization is physically renderer-owned; UI must not need
    // a mirrored exclusion list to keep its source wildcard correct.
    const auto renderer = read("packages/renderer/mcpp.toml");
    require(has(build, "\"tests/ui\",") &&
            has(renderer, "\"../../src/renderer/text/text_glyph_rasterizer.cpp\"") &&
            has(renderer, "\"../../src/renderer/text/text_glyph_raster_win32.cpp\"") &&
            has(renderer, "\"../../src/renderer/text/text_glyph_coverage_win32.cpp\"") &&
            has(renderer, "ldflags = [\"dwrite.lib\"]"),
            "native rasterizer must be owned and linked by cgpui_renderer");
    const auto ui = read("packages/ui/mcpp.toml");
    for (const auto* source : {"text_glyph_rasterizer.cpp", "text_glyph_raster_win32.cpp", "text_glyph_coverage_win32.cpp"}) {
      const auto root = std::filesystem::path(CGPUI_SOURCE_ROOT);
      require(!std::filesystem::exists(root / "src/ui" / source) &&
                  !has(ui, source),
              "native rasterization must remain renderer-owned without UI exclusions");
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native text structure failed: " << error.what() << '\n';
    return 1;
  }
}
