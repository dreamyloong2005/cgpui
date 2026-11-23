#include "dx12_diagnostics_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
std::array<unsigned char, 4> texel(unsigned image, unsigned x, unsigned y) {
  return {static_cast<unsigned char>((image * 29 + x * 61 + y * 17) % 256),
          static_cast<unsigned char>((image * 13 + x * 19 + y * 71) % 256),
          static_cast<unsigned char>((image * 7 + x * 43 + y * 37) % 256), 255};
}
ImageAsset image(unsigned index, unsigned variant) {
  ImageAsset asset{.id = {12000 + index}, .logical_size = {3, 2},
      .bitmap = {.width = 3, .height = 2, .stride = 16, .pixels = std::vector<std::uint8_t>(28, 0xe7)}};
  for (unsigned y = 0; y != 2; ++y) for (unsigned x = 0; x != 3; ++x) {
    const auto color = texel(index + variant, x, y);
    for (unsigned channel = 0; channel != 4; ++channel)
      asset.bitmap.pixels[y * 16 + x * 4 + channel] = color[channel];
  }
  return asset;
}
} // namespace

int main() {
  Dx12Window window;
  auto state = std::make_shared<Dx12State>();
  if (!window.hwnd || !dx12_configure_surface(*state, window.descriptor()) ||
      !dx12_create_device(*state) || !dx12_create_pipeline(*state) || !dx12_create_swapchain(*state))
    return dx12_fail(1, "texture staging budget fixture unavailable");
  for (unsigned pass = 0; pass != 3; ++pass) {
    const unsigned variant = pass == 2 ? 129 : 0;
    state->frame_active = true;
    Dx12Frame frame(state);
    frame.clear({0, 0, 1, 1});
    for (unsigned index = 0; index != 128; ++index) {
      const auto asset = image(index, variant);
      if (pass != 1) frame.upload_image(asset); // Cold, cached, then immutable replacements.
      frame.draw_image({.bounds = {{float(index % 16) * 4, float(index / 16) * 4}, {3, 2}},
          .asset = describe_image_asset(asset), .sampling = ImageSamplingMode::nearest});
    }
    if (!frame.request_pixel_capture() || !frame.present())
      return dx12_fail(2, "texture staging budget frame failed");
    const auto* pixels = state->pixels ? &*state->pixels : nullptr;
    for (unsigned index = 0; index != 128; ++index)
      for (unsigned y = 0; y != 2; ++y) for (unsigned x = 0; x != 3; ++x)
        if (!dx12_pixel(pixels, (index % 16) * 4 + x, (index / 16) * 4 + y, texel(index + variant, x, y)))
          return dx12_fail(3, "texture staging budget changed padded image texels");
    const auto payload = pass == 1 ? 0U : 3072U;
    if (!diagnostic_work(state->diagnostics ? &*state->diagnostics : nullptr, 128, 128, 128) ||
        state->diagnostics->submitted_draws.image_count != 128 ||
        state->diagnostics->upload_bytes.image_byte_count != payload || state->diagnostics->upload_bytes.glyph_atlas_byte_count)
      return dx12_fail(4, "texture staging budget changed logical upload bytes or draw counts");
    const auto stats = state->texture_uploads->stats();
    std::cout << "TEXTURE_UPLOAD_BUDGET pass=" << pass << " images=128 texels=768 payload_bytes=" << payload
              << " resources=" << stats.page_count << " new_pages=" << stats.allocations_this_frame << '\n';
    if (stats.page_count == 0 || stats.page_count > 8 ||
        (pass == 0 ? stats.allocations_this_frame != stats.page_count : stats.allocations_this_frame != 0) ||
        (pass == 1 ? stats.used_bytes != 0 : stats.used_bytes == 0))
      return dx12_fail(5, "texture staging resources still scale per texture or are not reused");
  }
  return 0;
}
