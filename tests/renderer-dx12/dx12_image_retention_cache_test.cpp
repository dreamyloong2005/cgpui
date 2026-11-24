#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <array>
#include <iostream>
#include <stdexcept>

using namespace cgpui;
namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
ImageAsset padded() {
  return {.id = {9801}, .logical_size = {2, 2}, .bitmap = {.width = 2, .height = 2, .stride = 12,
      .pixels = {255, 0, 0, 255, 255, 0, 0, 255, 9, 9, 9, 9,
                 255, 0, 0, 255, 255, 0, 0, 255}}};
}
}
int main() {
  try {
    Dx12State state, peer; // CPU cache only; no window, device or GPU initialization.
    auto& cache = *state.image_cache;
    auto image = padded();
    const auto descriptor = describe_image_asset(image);
    require(bool(cache.upload(state, image)) && cache.retained_image_payload().byte_count == 16,
        "padded RGBA upload-without-draw must count only tight CPU payload");
    auto first = cache.find(descriptor);
    require(bool(first), "first immutable snapshot unavailable");
    image.bitmap.pixels.assign(64, 17); // Caller capacity/size is not part of the census.
    require(cache.retained_image_payload().byte_count == 16, "caller storage changed renderer payload");
    require(bool(cache.upload(state, image)) && cache.retained_image_payload().byte_count == 32,
        "same-id replacement discarded a still-live old snapshot");
    auto second = cache.find(descriptor);
    require(bool(second) && (*first)->pixels[0] == 255 && (*second)->pixels[0] == 17,
        "replacement changed the still-owned old immutable payload or failed to copy the new bytes");
    cache.invalidate(image.id);
    require(cache.retained_image_payload().byte_count == 32, "invalidation counted only current cache entries");
    second->reset();
    require(cache.retained_image_payload().byte_count == 16, "replacement release did not update weak census");
    first->reset();
    require(cache.retained_image_payload().byte_count == 0, "last immutable owner was artificially retained");
    image = padded();
    require(bool(cache.upload(state, image)), "valid reupload failed");
    auto invalid = image;
    invalid.bitmap.stride = 0;
    require(!cache.upload(state, invalid) && cache.retained_image_payload().byte_count == 16,
        "invalid upload registered pixels or replaced the original cache entry");
    auto still_valid = cache.find(descriptor);
    require(still_valid && (*still_valid)->pixels[0] == 255, "invalid upload changed previous pixels");
    const std::array<std::uint8_t, 4> alpha{1, 2, 3, 4};
    auto glyph = dx12_create_texture(state, 2, 2, 2, DXGI_FORMAT_R8_UNORM, alpha);
    require(glyph && cache.retained_image_payload().byte_count == 16 &&
        peer.image_cache->retained_image_payload().byte_count == 0,
        "generic glyph creation or another cache polluted image accounting");
    cache.invalidate(image.id);
    require(cache.retained_image_payload().byte_count == 16, "live returned handle was omitted");
    still_valid->reset();
    require(cache.retained_image_payload().byte_count == 0, "invalidated final snapshot remained counted");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
