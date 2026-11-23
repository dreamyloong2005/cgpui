#include "dx12_texture_upload_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
std::array<unsigned char, 4> texel(unsigned index, unsigned x, unsigned y,
                                  unsigned width, unsigned height, unsigned version) {
  return {static_cast<unsigned char>(20 + index * 17 + (x >= width / 2 ? 73 : 0)),
          static_cast<unsigned char>(30 + version * 31 + (y >= height / 2 ? 91 : 0)),
          static_cast<unsigned char>(40 + (x >= width / 2 ? 27 : 0) + (y >= height / 2 ? 53 : 0)), 255};
}
ImageAsset image(unsigned index, unsigned width, unsigned height, unsigned version) {
  const auto stride = width * 4 + 12;
  ImageAsset asset{.id = {18000 + index}, .logical_size = {float(width), float(height)},
      .bitmap = {.width = width, .height = height, .stride = stride,
          .pixels = std::vector<std::uint8_t>(std::size_t(stride) * (height - 1) + width * 4, 0xe7)}};
  for (unsigned y = 0; y != height; ++y) for (unsigned x = 0; x != width; ++x) {
    const auto color = texel(index, x, y, width, height, version);
    for (unsigned channel = 0; channel != 4; ++channel)
      asset.bitmap.pixels[std::size_t(y) * stride + x * 4 + channel] = color[channel];
  }
  return asset;
}
bool pixels_match(const RendererFramePixels& pixels, unsigned version, bool large) {
  // Verify every covered pixel of all eight downscaled images. Odd source
  // dimensions and padded rows expose incorrect placement/row-offset reuse.
  for (unsigned index = 0; index != 8; ++index)
    for (unsigned y = 0; y != 16; ++y) for (unsigned x = 0; x != 8; ++x) {
      const auto source_x = (2 * x + 1) * 33 / 16;
      const auto source_y = (2 * y + 1) * 257 / 32;
      if (!dx12_pixel(&pixels, index * 8 + x, y, texel(index, source_x, source_y, 33, 257, version)))
        return false;
    }
  for (unsigned y = 0; y != 32; ++y) for (unsigned x = 0; x != 64; ++x) {
    const auto expected = large ? texel(8, (2 * x + 1) * 513 / 128, (2 * y + 1) * 513 / 64,
                                       513, 513, version) : std::array<unsigned char, 4>{0, 0, 255, 255};
    if (!dx12_pixel(&pixels, x, y + 32, expected)) return false;
  }
  return true;
}
} // namespace

int main() {
  Dx12Window window;
  auto state = texture_upload_state(&window);
  if (!state) return dx12_fail(1, "texture multipage fixture unavailable");
  for (unsigned pass = 0; pass != 3; ++pass) {
    const bool large = pass != 1;
    state->frame_active = true;
    Dx12Frame frame(state);
    frame.clear({0, 0, 1, 1});
    for (unsigned index = 0; index != 8; ++index) {
      const auto asset = image(index, 33, 257, pass);
      frame.upload_image(asset);
      frame.draw_image({.bounds = {{float(index) * 8, 0}, {8, 16}},
          .asset = describe_image_asset(asset), .sampling = ImageSamplingMode::nearest});
    }
    if (large) {
      const auto asset = image(8, 513, 513, pass);
      frame.upload_image(asset);
      frame.draw_image({.bounds = {{0, 32}, {64, 32}},
          .asset = describe_image_asset(asset), .sampling = ImageSamplingMode::nearest});
    }
    if (!frame.request_pixel_capture() || !frame.present() || !state->pixels)
      return dx12_fail(2, "texture multipage GPU submission failed");
    if (!pixels_match(*state->pixels, pass, large))
      return dx12_fail(3, "texture multipage/oversize row offsets changed actual pixels");
    const auto draws = large ? 9U : 8U;
    const auto payload = large ? 1324068U : 271392U;
    if (!diagnostic_work(state->diagnostics ? &*state->diagnostics : nullptr, draws, draws, draws) ||
        state->diagnostics->submitted_draws.image_count != draws ||
        state->diagnostics->upload_bytes.image_byte_count != payload ||
        state->diagnostics->upload_bytes.glyph_atlas_byte_count)
      return dx12_fail(4, "texture multipage diagnostic payload includes padding or misses copies");
    const auto stats = state->texture_uploads->stats();
    // 33x257 consumes 65,668 footprint bytes, three per standard page.
    // 513x513 needs a temporary 1,245,184-byte page. Its next safe reset
    // retires it while retaining the three standard pages for replacement data.
    if (stats.page_count != (large ? 4U : 3U) ||
        stats.capacity_bytes != (large ? 2031616U : 786432U) ||
        stats.allocations_this_frame != (pass == 0 ? 4U : pass == 1 ? 0U : 1U))
      return dx12_fail(5, "texture multipage native allocation/reuse policy differs");
    std::cout << "TEXTURE_UPLOAD_PAGES pass=" << pass << " resources=" << stats.page_count
              << " capacity=" << stats.capacity_bytes << " new=" << stats.allocations_this_frame
              << " payload=" << payload << '\n';
  }
  return 0;
}
