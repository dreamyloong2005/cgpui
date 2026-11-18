#include "cgpui/ui/paint.hpp"
#include "cgpui/ui/text_glyph_rasterizer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

struct Mask {
  std::uint32_t width = 0, height = 0, stride = 0;
  std::vector<std::uint8_t> alpha;

  std::size_t ink() const {
    return std::count_if(alpha.begin(), alpha.end(), [](auto value) { return value != 0; });
  }
  bool shaped() const {
    std::uint32_t left = width, top = height, right = 0, bottom = 0;
    for (std::uint32_t y = 0; y < height; ++y) {
      for (std::uint32_t x = 0; x < width; ++x) {
        if (alpha[static_cast<std::size_t>(y) * stride + x]) {
          left = std::min(left, x); top = std::min(top, y);
          right = std::max(right, x + 1); bottom = std::max(bottom, y + 1);
        }
      }
    }
    if (right <= left || bottom <= top) return false;
    // Exclude transparent padding: a solid inset rectangle must still fail.
    return ink() < static_cast<std::size_t>(right - left) * (bottom - top) * 85 / 100;
  }
};

cgpui::TextDraw draw_for(const std::string& content, float scale = 1) {
  cgpui::PaintList paints;
  paints.set_scale({scale});
  paints.fill_text({{12, 18}, {240, 64}}, cgpui::rgb(255, 255, 255),
                   content, cgpui::FontDescriptor{"Segoe UI"}, 32);
  require(paints.commands().size() == 1, "PaintList did not emit one text command");
  const auto& text = paints.commands().front().text;
  return {.bounds = text.bounds, .color = text.color, .font = text.font,
    .content = text.content, .byte_length = text.byte_length,
    .font_size = text.font_size, .scale = text.scale,
    .device_font_size = text.device_font_size, .glyphs = text.glyphs, .lines = text.lines};
}

Mask production_mask(const std::string& content, float scale = 1) {
  cgpui::GlyphCache cache;
  const auto quads = cgpui::vulkan_build_textured_glyph_quads(draw_for(content, scale), cache);
  if (cache.upload_records().empty()) {
    require(quads.empty(), "glyph quads refer to missing atlas uploads");
    return {};
  }
  const auto& upload = cache.upload_records().front();
  require(upload.stride >= upload.width && upload.alpha.size() >=
          static_cast<std::size_t>(upload.stride) * upload.height,
          "glyph upload contains invalid alpha storage");
  return {upload.width, upload.height, upload.stride, upload.alpha};
}

void verify_native_masks() {
  const auto a = production_mask("A"), b = production_mask("B");
  const auto chinese_a = production_mask("\xE4\xB8\xAD");
  const auto chinese_b = production_mask("\xE6\x96\x87");
  const auto space = production_mask(" ");
  const auto large = production_mask("A", 2);
  require(a.ink() > 20 && b.ink() > 20 && a.alpha != b.alpha,
          "production Latin glyphs are missing or have identical alpha masks");
  require(chinese_a.ink() > 20 && chinese_b.ink() > 20 && chinese_a.alpha != chinese_b.alpha,
          "production CJK glyphs are missing or have identical alpha masks");
  require(a.shaped() && b.shaped() && chinese_a.shaped() && chinese_b.shaped(),
          "production glyph alpha fills its tight bounds like a placeholder rectangle");
  require(space.ink() == 0, "whitespace produced visible ink");
  require(std::any_of(a.alpha.begin(), a.alpha.end(), [](auto value) { return value > 0 && value < 255; }),
          "native glyph has no antialiased edge coverage");
  require(large.width >= a.width * 3 / 2 && large.height >= a.height * 3 / 2 && large.ink() > a.ink() * 2,
          "DPI scale did not increase production raster resolution");
  std::cout << "Production CGPUI alpha: A=" << a.width << 'x' << a.height << ", ink=" << a.ink()
            << "; A/B distinct=1; CJK distinct=1; whitespace ink=0; 2x DPI="
            << large.width << 'x' << large.height << '\n';
}

void verify_atlas_and_metadata() {
  auto draw = draw_for("A");
  const cgpui::Rect clip{{10, 16}, {32, 40}};
  draw.clip_rect = clip;
  draw.clip_stack = {{clip}, 1, false, clip};
  draw.metadata = {0.6F, cgpui::AffineTransform::translation(3, 5)};
  draw.composition_stack = {{draw.metadata}, 1, false, draw.metadata};
  draw.color = {0.2F, 0.4F, 0.8F, 0.45F};
  cgpui::GlyphCache cache;
  const auto first = cgpui::vulkan_build_textured_glyph_quads(draw, cache);
  const auto second = cgpui::vulkan_build_textured_glyph_quads(draw, cache);
  require(first.size() == 1 && second.size() == 1 && cache.upload_records().size() == 1,
          "repeated glyph draw did not reuse the native atlas allocation");
  const auto& quad = first.front();
  require(quad.clip_rect && quad.clip_rect->origin.x == clip.origin.x &&
          quad.clip_rect->size.width == clip.size.width && quad.clip_stack == draw.clip_stack &&
          quad.composition_stack == draw.composition_stack && quad.metadata == draw.metadata &&
          quad.color.r == draw.color.r && quad.color.a == draw.color.a,
          "native glyph quads lost clipping, composition, color or alpha");
  require(second.front().device_bounds.origin.x == quad.device_bounds.origin.x &&
          second.front().device_bounds.origin.y == quad.device_bounds.origin.y,
          "cached glyph lost its native ink bearing");
  const auto batches = cgpui::vulkan_plan_glyph_atlas_uploads(cache.upload_records(), cache.atlas_pages());
  require(!batches.empty(), "native alpha never reached the atlas upload plan");
  const auto b = cgpui::vulkan_build_textured_glyph_quads(draw_for("B"), cache);
  require(b.size() == 1 && cache.upload_records().size() == 2 &&
          cache.upload_records()[0].alpha != cache.upload_records()[1].alpha,
          "different TextDraw content at the same offset aliased the glyph cache");
  const auto repeated = cgpui::vulkan_build_textured_glyph_quads(draw_for("AA"), cache);
  require(repeated.size() == 2 && cache.upload_records().size() == 2 &&
          repeated[1].device_bounds.origin.x > repeated[0].device_bounds.origin.x &&
          repeated[1].key.byte_offset == 1,
          "repeated glyph at another position did not reuse alpha while preserving draw metadata");
  auto wide = draw_for("A");
  wide.glyphs.front().device_advance *= 1.5F;
  const auto wider = cgpui::vulkan_build_textured_glyph_quads(wide, cache);
  require(wider.size() == 1 && cache.upload_records().size() == 3,
          "different layout-cell advances aliased one fitted native glyph mask");
}

void verify_input_validation() {
  const auto draw = draw_for("A");
  require(!draw.glyphs.empty(), "glyph metadata missing");
  const auto native = cgpui::rasterize_text_glyph(draw.glyphs.front(), "A");
  require(native.has_value() && native->baseline > 0 && native->baseline <= draw.device_font_size &&
          native->left_bearing >= 0 && native->top_bearing >= 0 &&
          native->left_bearing + native->bitmap.width <= std::ceil(draw.glyphs.front().device_advance) + 1 &&
          native->top_bearing + native->bitmap.height <= std::ceil(draw.device_font_size) + 1,
          "native glyph bearings or baseline do not fit its existing layout cell");
  const auto empty = cgpui::rasterize_text_glyph(draw.glyphs.front(), "");
  require(empty.has_value() && empty->bitmap.width ==
          cgpui::rasterized_glyph_dimension(draw.glyphs.front().device_advance) &&
          empty->bitmap.height == cgpui::rasterized_glyph_dimension(draw.device_font_size) &&
          std::all_of(empty->bitmap.alpha.begin(), empty->bitmap.alpha.end(), [](auto value) { return value == 0; }),
          "metadata-only draws lost their transparent geometry compatibility");
  const auto invalid_utf8 = cgpui::rasterize_text_glyph(draw.glyphs.front(), "\xff");
  require(!invalid_utf8, "native rasterizer accepted invalid UTF-8");
  auto zero_advance = draw.glyphs.front();
  zero_advance.device_advance = 0;
  const auto zero = cgpui::rasterize_text_glyph(zero_advance, "A");
  require(zero.has_value() && zero->bitmap.width == 1 && zero->bitmap.height == 1 &&
          zero->bitmap.alpha == std::vector<std::uint8_t>{0} && zero->advance == 0 &&
          zero->baseline > 0 && zero->baseline <= draw.device_font_size,
          "zero-advance cells should preserve metrics without visible ink");
  auto invalid = draw.glyphs.front();
  invalid.key.font_size = -1;
  require(!cgpui::rasterize_text_glyph(invalid, "A"), "native rasterizer accepted a negative font size");
  auto harfbuzz = draw.glyphs.front();
  harfbuzz.key.shaping_backend = cgpui::TextShapingBackend::harfbuzz;
  const auto unsupported = cgpui::rasterize_text_glyph(harfbuzz, "A");
  require(!unsupported && unsupported.error().code == cgpui::ErrorCode::unsupported_platform,
          "native HarfBuzz runs were silently treated as source clusters");
  auto invalid_draw = draw;
  invalid_draw.content = "\xff";
  cgpui::GlyphCache cache;
  const auto quads = cgpui::vulkan_build_textured_glyph_quads(invalid_draw, cache);
  require(quads.empty() && cache.rasterization_error().has_value(),
          "production raster failure was hidden or replaced by placeholder glyphs");
}
} // namespace

int main() {
  // Asserts fixed deterministic advances; native shaping is covered by
  // native_text_shaping_test.
  cgpui::set_default_text_shaping_backend(
      cgpui::TextShapingBackend::deterministic_fallback);
  try {
    verify_native_masks();
    verify_atlas_and_metadata();
    verify_input_validation();
    std::cout << "Native text regression passed: silhouettes, CJK, whitespace, DPI, cache, atlas and metadata.\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native text regression failed: " << error.what() << '\n';
    return 1;
  }
}
