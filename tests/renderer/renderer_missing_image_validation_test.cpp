#include "renderer_pixel_test_support.hpp"
#include <cstdio>
#include <limits>

using namespace cgpui;
using namespace cgpui_test;
namespace {
int fail(int code, const char* message) { std::fprintf(stderr, "%s\n", message); return code; }
ImageDraw draw(ImageAssetDescriptor asset) { return {.bounds = {{8, 8}, {32, 32}}, .asset = asset}; }
bool rejected(RenderFrame& frame, Renderer& renderer) {
  const auto result = frame.present(); // Never retry Present; later frames resupply as needed.
  const auto pixels = completed_pixel_capture(renderer);
  return !result && result.error().code == ErrorCode::invalid_argument && pixels &&
      pixel_near(*pixels, 56, 56, {0, 0, 0, 255});
}
}
int main() {
  RendererPixelTestWindow window(L"CGPUIMissingImageValidation", L"CGPUI Missing Image Validation");
  auto renderer = create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!window.valid() || !renderer) return fail(1, "native validation fixture unavailable");
  const ImageAsset green{.id = {971415}, .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
  const auto asset = describe_image_asset(green);
  {
    auto frame = begin_pixel_capture(**renderer);
    if (!frame) return fail(2, "validation seed begin failed");
    (*frame)->clear({0, 0, 0, 1}); (*frame)->upload_image(green); (*frame)->draw_image(draw(asset));
    if (!(*frame)->present()) return fail(3, "validation seed failed");
  }
  const auto generation = (*renderer)->backend_info().device_generation;
  for (unsigned cached = 0; cached < 2; ++cached) {
    for (unsigned kind = 0; kind < 11; ++kind) {
      auto bad = draw(asset);
      if (!cached) bad.asset.id.value++;
      const float invalid = std::numeric_limits<float>::quiet_NaN();
      if (kind == 0) bad.asset.pixel_width = 0;
      if (kind == 1) bad.asset.pixel_height = 0;
      if (kind == 2) bad.asset.format = static_cast<ImageFormat>(255);
      if (kind == 3) bad.sampling = static_cast<ImageSamplingMode>(255);
      if (kind == 4) bad.bounds.origin.x = invalid;
      if (kind == 5) bad.source_rect = Rect{{invalid, 0}, {1, 1}};
      if (kind == 6) bad.tint = Color{1, 1, 1, invalid};
      if (kind == 7) bad.clip_rect = Rect{{0, 0}, {invalid, 1}};
      if (kind == 8) bad.metadata.transform.skew_x = invalid;
      if (kind == 9) bad.metadata.opacity = invalid;
      if (kind == 10) bad.metadata.transform.scale_x = (std::numeric_limits<float>::max)();
      auto frame = begin_pixel_capture(**renderer);
      if (!frame) return fail(4, "invalid input begin failed");
      (*frame)->clear({1, 0, 1, 1}); (*frame)->draw_image(bad);
      if (!rejected(**frame, **renderer)) return fail(5, "invalid absent/cached draw was skipped or overwrote capture");
    }
  }
  for (unsigned kind = 0; kind < 6; ++kind) {
    auto bad = green;
    if (kind == 0) bad.id = {};
    if (kind == 1) bad.bitmap.width = 0;
    if (kind == 2) bad.bitmap.stride = 0;
    if (kind == 3) bad.bitmap.pixels.resize(3);
    if (kind == 4) bad.bitmap.format = static_cast<ImageFormat>(255);
    if (kind == 5) bad.bitmap.width = (std::numeric_limits<std::uint32_t>::max)();
    auto frame = begin_pixel_capture(**renderer);
    if (!frame) return fail(6, "invalid upload begin failed");
    (*frame)->clear({1, 0, 1, 1}); (*frame)->upload_image(green); (*frame)->upload_image(bad);
    if (!rejected(**frame, **renderer)) return fail(7, "bad final upload was hidden by missing/earlier pixels");
  }
  for (unsigned conflict = 0; conflict < 2; ++conflict) {
    auto bad = draw(asset); bad.asset.pixel_width = 2;
    auto frame = begin_pixel_capture(**renderer);
    if (!frame) return fail(8, "identity conflict begin failed");
    (*frame)->clear({1, 0, 1, 1});
    if (conflict) {
      bad.asset.id.value += 2;
      auto other = bad; other.asset.pixel_width = 1; (*frame)->draw_image(other);
    }
    (*frame)->draw_image(bad);
    if (!rejected(**frame, **renderer)) return fail(9, "installed/absent same-ID conflict silently skipped");
  }
  auto frame = begin_pixel_capture(**renderer);
  if (!frame || (*renderer)->backend_info().device_generation != generation) return fail(10, "input errors poisoned native state");
  auto invalid = green; invalid.bitmap.stride = 0;
  (*frame)->upload_image(invalid); (*frame)->upload_image(green); // Final-ID replacement; no rollback assumption.
  auto padded = draw(asset); padded.asset.stride = 0; padded.asset.byte_size = 0;
  (*frame)->clear({1, 1, 0, 1}); (*frame)->draw_image(padded);
  if (!(*frame)->present()) return fail(11, "valid final resupply or layout metadata rejected");
  const auto pixels = completed_pixel_capture(**renderer);
  return pixels && pixel_near(*pixels, 12, 12, {0, 255, 0, 255}) &&
      pixel_near(*pixels, 56, 56, {255, 255, 0, 255}) ? 0 : fail(12, "healthy resupply pixels differ");
}
