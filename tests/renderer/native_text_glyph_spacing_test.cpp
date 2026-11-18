#include "cgpui/renderer/glyph_cache.hpp"
#include "cgpui/ui/paint.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

#if defined(_WIN32)
const cgpui::FontDescriptor font{.family = "Segoe UI"};
#else
const cgpui::FontDescriptor font{.family = "DejaVu Sans"};
#endif

struct InkBox {
  int left = 0;
  int right = 0;
};

cgpui::TextDraw draw_for(const std::string& content, float size, float scale) {
  cgpui::PaintList paints;
  paints.set_scale({scale});
  paints.fill_text({{12, 18}, {900, 80}}, cgpui::rgb(0, 0, 0), content, font, size);
  const auto& text = paints.commands().front().text;
  return {.bounds = text.bounds, .color = text.color, .font = text.font,
          .content = text.content, .byte_length = text.byte_length,
          .font_size = text.font_size, .scale = text.scale,
          .device_font_size = text.device_font_size, .glyphs = text.glyphs,
          .lines = text.lines};
}

// Ink boxes of every glyph quad, using the alpha of its atlas upload.
std::vector<InkBox> glyph_ink(const cgpui::TextDraw& draw, cgpui::GlyphCache& cache) {
  const auto quads = cgpui::vulkan_build_textured_glyph_quads(draw, cache);
  require(quads.size() == draw.glyphs.size(), "every glyph needs a quad");
  require(!cache.rasterization_error().has_value(), "native rasterization failed");
  std::vector<InkBox> boxes;
  for (const auto& quad : quads) {
    for (const auto& upload : cache.upload_records()) {
      if (upload.page_index != quad.page_index ||
          upload.atlas_bounds.origin.x != quad.atlas_bounds.origin.x ||
          upload.atlas_bounds.origin.y != quad.atlas_bounds.origin.y) {
        continue;
      }
      int left = static_cast<int>(upload.width), right = -1;
      for (std::uint32_t y = 0; y < upload.height; ++y) {
        for (std::uint32_t x = 0; x < upload.width; ++x) {
          if (upload.alpha[y * upload.stride + x] > 32) {
            left = std::min(left, static_cast<int>(x));
            right = std::max(right, static_cast<int>(x) + 1);
          }
        }
      }
      const int origin = static_cast<int>(std::lround(quad.device_bounds.origin.x));
      boxes.push_back(right < 0 ? InkBox{} : InkBox{origin + left, origin + right});
    }
  }
  require(boxes.size() == quads.size(), "a quad has no atlas upload");
  return boxes;
}

// Largest horizontal gap between the ink of neighbouring letters of one word.
float max_letter_gap(const std::string& content, float size, float scale) {
  cgpui::GlyphCache cache;
  const auto draw = draw_for(content, size, scale);
  const auto boxes = glyph_ink(draw, cache);
  float widest = 0.0F;
  for (std::size_t index = 1; index < boxes.size(); ++index) {
    const char previous = content[draw.glyphs[index - 1].key.byte_offset];
    const char current = content[draw.glyphs[index].key.byte_offset];
    if (previous == ' ' || current == ' ') continue;
    const float gap = static_cast<float>(boxes[index].left - boxes[index - 1].right);
    if (std::getenv("CGPUI_SPACING_DEBUG") != nullptr) {
      std::cout << previous << current << ' ' << gap << "\n";
    }
    widest = std::max(widest, gap);
  }
  return widest;
}

} // namespace

int main() {
  try {
    for (const float scale : {1.0F, 1.5F}) {
      const float size = 28.0F;
      const float limit = 0.2F * size * scale;
      const float gap = max_letter_gap("Floating panel", size, scale);
      std::cout << "Floating panel @" << size << " scale " << scale
                << ": widest letter gap " << gap << " px (limit " << limit << ")\n";
      require(gap <= limit, "letters inside a word are separated by a visible gap");
      const float title = max_letter_gap("CGPUI backdrop blur", 48.0F, scale);
      require(title <= 0.2F * 48.0F * scale, "title letters are separated by a gap");
    }
#if defined(_WIN32)
    // The fixed-advance fallback reproduced the "Fl oat i ng" defect.
    cgpui::set_default_text_shaping_backend(
        cgpui::TextShapingBackend::deterministic_fallback);
    const float fixed = max_letter_gap("Floating panel", 28.0F, 1.0F);
    std::cout << "deterministic fallback widest gap " << fixed << " px\n";
    require(fixed > 0.2F * 28.0F, "fallback comparison no longer shows the defect");
#endif
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Native glyph spacing failed: " << error.what() << '\n';
    return 1;
  }
}
