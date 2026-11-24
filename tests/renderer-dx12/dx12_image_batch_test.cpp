#include "../../src/renderer/dx12/dx12_image_batch_internal.hpp"
#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"
#include "../../src/renderer/dx12/dx12_draw_commands_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <iostream>
#include <stdexcept>

using namespace cgpui;
namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
ImageAsset image(unsigned red, unsigned green) {
  return {.id = {9880}, .logical_size = {1, 1}, .bitmap = {.width = 1, .height = 1, .stride = 4,
      .pixels = {static_cast<unsigned char>(red), static_cast<unsigned char>(green), 0, 255}}};
}
Dx12DrawCommand request(const ImageAssetDescriptor& asset, std::size_t command) {
  return {Dx12PendingImageDraw{ImageDraw{.bounds = {{0, 0}, {16, 16}}, .asset = asset}},
      RendererPrimitiveKind::image, command};
}
void final_upload_and_ownership() {
  Dx12State state; // CPU-only: no HWND, device, command queue or GPU.
  auto red = image(255, 0), green = image(0, 255);
  const auto asset = describe_image_asset(red);
  Dx12ImageBatch batch;
  std::vector<Dx12DrawCommand> draws{request(asset, 4), {RectGeometry{}, RendererPrimitiveKind::solid_rect, 7},
      request(asset, 9)};
  batch.upload(red); batch.invalidate(asset.id); batch.invalidate(asset.id); batch.invalidate({});
  batch.upload(green);
  green.bitmap.pixels.assign(4, 0);
  require(state.image_cache->retained_image_payload().byte_count == 0 && !state.image_cache->find(asset),
      "recording mutated the installed image cache");
  require(!dx12_require_resolved_images(draws) && bool(batch.prepare(state, draws)) &&
      bool(dx12_require_resolved_images(draws)), "image prefix did not resolve every pending request");
  const auto& left = std::get<Dx12TexturedDraw>(draws[0].payload);
  const auto& right = std::get<Dx12TexturedDraw>(draws[2].payload);
  require(left.texture == right.texture && left.texture->pixels == std::vector<std::uint8_t>{0, 255, 0, 255} &&
      draws[0].command_index == 4 && draws[1].command_index == 7 && draws[2].command_index == 9 &&
      std::holds_alternative<RectGeometry>(draws[1].payload) &&
      state.image_cache->retained_image_payload().byte_count == 4,
      "last-ID copy, immutable dedupe, original slots or tight census changed");
  state.image_cache->invalidate(asset.id);
  require(state.image_cache->retained_image_payload().byte_count == 4, "resolved draw lost its immutable owner");
  draws.clear();
  require(state.image_cache->retained_image_payload().byte_count == 0 && bool(batch.prepare(state, {})) &&
      state.image_cache->retained_image_payload().byte_count == 0,
      "completed queue retained/reapplied raw requests or added invisible snapshot ownership");
}
void validation_and_abandonment() {
  Dx12State state;
  const auto red = image(255, 0), green = image(0, 255);
  const auto asset = describe_image_asset(red);
  require(bool(state.image_cache->upload(state, red)), "CPU cache seed failed");
  { Dx12ImageBatch abandoned; abandoned.invalidate(asset.id); abandoned.upload(green); }
  auto preserved = state.image_cache->find(asset);
  require(preserved && (*preserved)->pixels[0] == 255, "abandoned batch mutated cache");
  preserved->reset();
  auto invalid = green; invalid.bitmap.stride = 0;
  Dx12ImageBatch replaced;
  replaced.upload(invalid); replaced.upload(green);
  std::vector<Dx12DrawCommand> draws{request(asset, 0)};
  require(bool(replaced.prepare(state, draws)) && std::get<Dx12TexturedDraw>(draws[0].payload).texture->pixels[1] == 255,
      "invalid intermediate upload was latched instead of replaced by the final value");
  Dx12ImageBatch invalid_final;
  invalid_final.upload(red); invalid_final.upload(invalid);
  const auto rejected = invalid_final.prepare(state, {});
  require(!rejected && rejected.error().code == ErrorCode::invalid_argument && !state.failed &&
      bool(invalid_final.prepare(state, {})), "invalid final request poisoned native state or survived preparation");
  auto missing = asset; missing.id.value = 9881;
  std::vector<Dx12DrawCommand> absent{request(missing, 1)};
  Dx12ImageBatch unresolved;
  require(!unresolved.prepare(state, absent) && !state.failed,
      "DX12 missing-image input rejection was silently changed into skip or device failure");
}
}
int main() {
  try { final_upload_and_ownership(); validation_and_abandonment(); return 0; }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
