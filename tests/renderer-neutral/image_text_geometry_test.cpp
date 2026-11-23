#include "cgpui/renderer/planning/image_color.hpp"
#include "cgpui/renderer/planning/image_geometry.hpp"
#include "cgpui/renderer/planning/text_geometry.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <type_traits>

namespace {
bool near(float a, float b) { return std::abs(a - b) < 0.0001F; }
int fail(int code, const char* message) {
  std::cerr << "image/text planning: " << message << '\n';
  return code;
}
}

int main() {
  using namespace cgpui;
  static_assert(std::is_standard_layout_v<ImageVertex>);
  static_assert(std::is_standard_layout_v<TextVertex>);
  static_assert(sizeof(ImageVertex) == sizeof(TextVertex));
  static_assert(offsetof(ImageVertex, image_uv) == offsetof(TextVertex, atlas_uv));
  static_assert(offsetof(ImageVertex, color) == offsetof(TextVertex, color));
  ImageDraw image{
      .bounds = {{2, 3}, {10, 8}},
      .asset = {.pixel_width = 100, .pixel_height = 80},
      .source_rect = Rect{{25, -20}, {100, 60}},
      .tint = Color{0.2F, 0.4F, 0.6F, 0.8F},
      .metadata = {.opacity = 0.5F},
  };
  image.composition_stack = {
      .entries = {PaintMetadata{.opacity = 0.5F}, PaintMetadata{.opacity = 0.25F}},
      .full_depth = 2,
      .current_metadata = {.opacity = 0.25F,
          .transform = {.scale_x = 2, .scale_y = 3, .translate_x = 5, .translate_y = 7}},
  };
  const auto tint = resolve_image_tint(image);
  const auto vertices = build_image_vertices(std::array{image});
  if (vertices.size() != image_vertices_per_quad || vertices.size() != 6 ||
      !near(tint.r, 0.2F) || !near(tint.a, 0.2F)) return fail(1, "tint opacity applied incorrectly");
  const std::array<std::array<float, 2>, 6> positions{
      {{9, 16}, {29, 16}, {29, 40}, {9, 16}, {29, 40}, {9, 40}}};
  const std::array<std::array<float, 2>, 6> uvs{
      {{0.25F, 0}, {1, 0}, {1, 0.5F}, {0.25F, 0}, {1, 0.5F}, {0.25F, 0.5F}}};
  for (std::size_t i = 0; i != vertices.size(); ++i) {
    if (vertices[i].position != positions[i] || vertices[i].image_uv != uvs[i] ||
        !near(vertices[i].color[3], 0.2F)) return fail(2, "image transform/crop/order differs");
  }
  ImageDraw full{.bounds = {{0, 0}, {1, 1}}};
  const auto default_tint = resolve_image_tint(full);
  if (default_tint.r != 1 || default_tint.g != 1 || default_tint.b != 1 ||
      default_tint.a != 1) return fail(3, "default tint is not opaque white");
  full.source_rect = Rect{{4, 4}, {2, 2}};
  const auto batch = build_image_vertices(std::array{image, full});
  if (batch.size() != 12 || batch[6].image_uv != std::array<float, 2>{0, 0} ||
      batch[8].image_uv != std::array<float, 2>{1, 1}) {
    return fail(4, "zero-size source fallback or batch order differs");
  }
  TexturedGlyphQuad quad{
      .device_bounds = {{1.25F, 2.5F}, {3.75F, 4.5F}},
      .atlas_uv_bounds = {{0.25F, 0.125F}, {0.5F, 0.25F}},
      .color = {0.6F, 0.4F, 0.2F, 0.3F},
      .metadata = {.opacity = 0.5F, .transform = AffineTransform::translation(10, 20)},
  };
  const auto preserved = build_text_vertices(std::array{quad});
  const auto snapped = build_text_vertices(
      std::array{quad}, TextPositioningPolicy::snap_to_device_pixels);
  if (default_text_positioning_policy() != TextPositioningPolicy::preserve_subpixel ||
      preserved.size() != text_vertices_per_quad || snapped.size() != 6 ||
      preserved[0].position != std::array<float, 2>{11.25F, 22.5F} ||
      snapped[0].position != std::array<float, 2>{11, 23} ||
      snapped[2].position != std::array<float, 2>{15, 27}) {
    return fail(5, "text positioning or transform differs");
  }
  for (std::size_t i = 0; i != snapped.size(); ++i) {
    if (snapped[i].atlas_uv != preserved[i].atlas_uv ||
        snapped[i].color != std::array<float, 4>{0.6F, 0.4F, 0.2F, 0.3F}) {
      return fail(6, "text UV changed or precomposed opacity applied twice");
    }
  }
  if (preserved[0].atlas_uv != std::array<float, 2>{0.25F, 0.125F} ||
      preserved[2].atlas_uv != std::array<float, 2>{0.75F, 0.375F} ||
      !build_image_vertices({}).empty() || !build_text_vertices({}).empty()) {
    return fail(7, "text UV bounds or empty geometry differs");
  }
  return 0;
}
