#include "probe_internal.hpp"
#include <cstdlib>

namespace renderer_probe {
namespace {
std::array<std::uint8_t, 4> color(unsigned index) {
  constexpr std::array<std::array<std::uint8_t, 4>, 6> palette{{
      {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255},
      {255, 255, 0, 255}, {255, 0, 255, 255}, {0, 255, 255, 255}}};
  return palette[index % palette.size()];
}
bool pixel(const cgpui::RendererFramePixels& pixels, unsigned x, unsigned y,
           const std::array<std::uint8_t, 4>& expected) {
  const auto actual = pixels.pixel_rgba8(x, y);
  if (!actual) return false;
  for (unsigned channel = 0; channel != 4; ++channel)
    if (std::abs(int((*actual)[channel]) - int(expected[channel])) > 1) return false;
  return true;
}
} // namespace
Scene::Scene(SceneKind selected) : kind(selected) {
  for (unsigned index = 0; index != assets.size(); ++index) {
    auto& asset = assets[index];
    asset.id = {70000 + index}; asset.logical_size = {2, 2};
    asset.bitmap.width = 2; asset.bitmap.height = 2; asset.bitmap.stride = 8;
    asset.bitmap.format = cgpui::ImageFormat::rgba8_unorm;
    for (unsigned texel = 0; texel != 4; ++texel) {
      const auto rgba = color(index + texel);
      asset.bitmap.pixels.insert(asset.bitmap.pixels.end(), rgba.begin(), rgba.end());
    }
  }
}
std::size_t Scene::rectangles() const { return kind == SceneKind::images ? 0 : 128; }
std::size_t Scene::images() const { return kind == SceneKind::rectangles ? 0 : assets.size(); }
void Scene::replay(cgpui::RenderFrame& frame, bool upload) const {
  frame.clear({0, 0, 0, 1});
  if (upload) for (std::size_t i = 0; i != images(); ++i) frame.upload_image(assets[i]);
  for (unsigned index = 0; index != rectangles(); ++index) {
    const auto rgba = color(index);
    frame.draw_rect({.rect = {{float(index % 16) * 24, float(index / 16) * 24}, {16, 16}},
        .color = {rgba[0] / 255.0F, rgba[1] / 255.0F, rgba[2] / 255.0F, 1}});
  }
  for (unsigned index = 0; index != images(); ++index)
    frame.draw_image({.bounds = {{float(index % 8) * 48, 240 + float(index / 8) * 48}, {32, 32}},
        .asset = cgpui::describe_image_asset(assets[index]), .sampling = cgpui::ImageSamplingMode::nearest});
}
bool Scene::verify(const cgpui::RendererFramePixels& pixels) const {
  if (!pixels.valid() || pixels.width != 800 || pixels.height != 600 ||
      !pixel(pixels, 799, 599, {0, 0, 0, 255})) return false;
  for (unsigned index = 0; index != rectangles(); ++index)
    if (!pixel(pixels, (index % 16) * 24 + 8, (index / 16) * 24 + 8, color(index))) return false;
  for (unsigned index = 0; index != images(); ++index)
    for (unsigned texel = 0; texel != 4; ++texel)
      if (!pixel(pixels, (index % 8) * 48 + (texel % 2) * 16 + 8,
          240 + (index / 8) * 48 + (texel / 2) * 16 + 8, color(index + texel))) return false;
  return true;
}
} // namespace renderer_probe
