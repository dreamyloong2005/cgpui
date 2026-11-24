#include "renderer_image_validation_internal.hpp"
#include <array>
#include <limits>
#include <cstdio>

using namespace cgpui;
namespace {
ImageDraw sample() {
  return {.bounds = {{1, 2}, {16, 16}}, .asset = {.id = {1}, .pixel_width = 2, .pixel_height = 2},
      .source_rect = Rect{{0, 0}, {2, 2}}, .tint = Color{1, 1, 1, 1},
      .clip_rect = Rect{{100, 100}, {1, 1}}, // Invisible does not exempt malformed input.
      .clip_stack = {.clips = {Rect{{0, 0}, {64, 64}}}, .full_depth = 1,
          .current_clip_rect = Rect{{0, 0}, {64, 64}}},
      .composition_stack = {.entries = {PaintMetadata{}}, .full_depth = 1}};
}
std::vector<float*> fields(ImageDraw& d) {
  std::vector<float*> result;
  const auto rect = [&](Rect& r) {
    for (float* p : {&r.origin.x, &r.origin.y, &r.size.width, &r.size.height}) result.push_back(p);
  };
  const auto metadata = [&](PaintMetadata& m) {
    auto& t = m.transform;
    for (float* p : {&m.opacity, &t.scale_x, &t.skew_y, &t.skew_x, &t.scale_y, &t.translate_x, &t.translate_y})
      result.push_back(p);
  };
  rect(d.bounds); rect(*d.source_rect); rect(*d.clip_rect);
  rect(d.clip_stack.clips[0]); rect(*d.clip_stack.current_clip_rect);
  for (float* p : {&d.tint->r, &d.tint->g, &d.tint->b, &d.tint->a}) result.push_back(p);
  metadata(d.metadata); metadata(d.composition_stack.entries[0]); metadata(d.composition_stack.current_metadata);
  return result;
}
bool rejected(const ImageDraw& d) {
  const auto r = renderer_validate_image_draw(d);
  return !r && r.error().code == ErrorCode::invalid_argument;
}
}
int main() {
  auto valid = sample();
  if (!renderer_validate_image_draw(valid)) return 1;
  for (auto invalid : {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
                       -std::numeric_limits<float>::infinity()}) {
    for (std::size_t index = 0; index < fields(valid).size(); ++index) {
      auto draw = sample(); *fields(draw)[index] = invalid;
      if (!rejected(draw)) { std::fprintf(stderr, "nonfinite raw field %zu accepted\n", index); return 2; }
    }
  }
  for (unsigned kind = 0; kind < 5; ++kind) {
    auto draw = sample();
    if (kind == 0) draw.asset.id = {};
    if (kind == 1) draw.asset.pixel_width = 0;
    if (kind == 2) draw.asset.pixel_height = 0;
    if (kind == 3) draw.asset.format = static_cast<ImageFormat>(255);
    if (kind == 4) draw.sampling = static_cast<ImageSamplingMode>(255);
    if (!rejected(draw)) return 3;
  }
  auto derived = sample();
  derived.composition_stack.current_metadata.transform.scale_x = std::numeric_limits<float>::max();
  if (!rejected(derived)) return 4;
  for (unsigned kind = 0; kind < 3; ++kind) {
    auto draw = sample();
    auto& rect = kind == 0 ? draw.bounds : kind == 1 ? *draw.source_rect : *draw.clip_rect;
    rect.origin.x = rect.size.width = std::numeric_limits<float>::max();
    if (!rejected(draw)) return 5;
  }
  auto padding = valid; padding.asset.stride = 999; padding.asset.byte_size = 0;
  if (!renderer_validate_image_draw(padding) || !renderer_same_image_allocation(valid.asset, padding.asset)) return 6;
  padding.asset.pixel_width++;
  if (renderer_same_image_allocation(valid.asset, padding.asset)) return 7;
  valid.bounds.size = {-16, 0}; // Preserve existing draw-geometry policy.
  return renderer_validate_image_draw(valid) ? 0 : 8;
}
