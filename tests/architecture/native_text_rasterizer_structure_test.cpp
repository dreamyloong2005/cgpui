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
  const auto root = std::getenv("CGPUI_SOURCE_ROOT");
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
    const auto leaf = read("include/cgpui/ui/text_glyph_rasterizer.hpp");
    const auto aggregate = read("include/cgpui/ui/text.hpp");
    const auto dispatch = read("src/ui/text_glyph_rasterizer.cpp");
    const auto native = read("src/ui/text_glyph_raster_win32.cpp");
    const auto coverage = read("src/ui/text_glyph_coverage_win32.cpp");
    const auto native_header = read("src/ui/text_glyph_raster_win32_internal.hpp");
    const auto coverage_header = read("src/ui/text_glyph_coverage_win32_internal.hpp");
    const auto production = read("src/renderer/vulkan/vulkan_report_text_quads.cpp");
    const auto build = read("xmake.lua");
    const auto module = read("build/xmake/capabilities/native_text_targets.lua");
    const auto pixel_shape = read("tests/renderer/vulkan_text_pixel_shape_support.hpp");
    require(lines(pixel_shape) <= 60 && has(pixel_shape, "text_ink_shape("),
            "pixel glyph shape inspection must stay in a focused test helper");
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
    const auto renderer_start = module.find("target(\"cgpui_renderer\")");
    const auto renderer_end = module.find("target_end()", renderer_start);
    const auto renderer = module.substr(renderer_start, renderer_end - renderer_start);
    require(renderer_start != std::string::npos && has(build, "includes(\"build/xmake/capabilities/*.lua\")") &&
            has(renderer, "text_glyph_rasterizer.cpp") && has(renderer, "text_glyph_raster_win32.cpp") &&
            has(renderer, "text_glyph_coverage_win32.cpp") && has(renderer, "add_syslinks(\"dwrite\""),
            "native rasterizer must be owned and linked by cgpui_renderer");
    const auto ui_start = module.find("target(\"cgpui_ui\")");
    const auto ui = module.substr(ui_start, module.find("target_end()", ui_start) - ui_start);
    for (const auto* source : {"text_glyph_rasterizer.cpp", "text_glyph_raster_win32.cpp", "text_glyph_coverage_win32.cpp"}) {
      require(ui_start != std::string::npos && has(ui, std::string("remove_files(path.join(os.projectdir(), \"src/ui/") + source + "\"))"),
              "UI wildcard duplicated renderer-owned native rasterizer sources");
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native text structure failed: " << error.what() << '\n';
    return 1;
  }
}
