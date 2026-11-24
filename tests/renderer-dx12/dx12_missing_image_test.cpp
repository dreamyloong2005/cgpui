#include "../../src/renderer/dx12/dx12_image_batch_internal.hpp"
#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"
#include "../../src/renderer/dx12/dx12_image_resolve_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_diagnostics_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <limits>
#include <stdexcept>
#include <iostream>

using namespace cgpui;
namespace {
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
const ImageAsset image{.id = {914}, .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
Dx12DrawCommand request(ImageAssetDescriptor asset, std::size_t index) {
  return {Dx12PendingImageDraw{ImageDraw{.bounds = {{0, 0}, {16, 16}}, .asset = asset}},
      RendererPrimitiveKind::image, index};
}
void ledger_and_marker() {
  Dx12State state; state.width = state.height = 64;
  const SolidRect rect{.rect = {{0, 0}, {16, 16}}, .color = {1, 0, 0, 1}};
  const auto geometry = build_solid_rect_geometry(std::span{&rect, 1});
  std::vector<Dx12DrawCommand> draws{{geometry, RendererPrimitiveKind::solid_rect, 0},
      request(describe_image_asset(image), 1), {geometry, RendererPrimitiveKind::solid_rect, 2}};
  draws[1].resource_offset = 7;
  Dx12ImageBatch batch;
  require(bool(batch.prepare(state, draws)) && bool(dx12_require_resolved_images(draws)) &&
      std::holds_alternative<Dx12MissingImageDraw>(draws[1].payload), "absence did not become resolved missing marker");
  {
    Dx12FrameDiagnostics ledger(state, draws, 3);
    ledger.begin_draw(0); ledger.submitted(0); // CPU ledger observation, not a native claim.
    ledger.begin_draw(1);
    require(bool(dx12_record_draw(state, draws[1])), "missing dispatch needs native state");
    ledger.begin_draw(2); ledger.submitted(0);
    ledger.publish();
  }
  const auto& s = *state.diagnostics;
  require(s.planned_draws.image_count == 1 && s.submitted_draws.image_count == 0 &&
      s.work.pending_draw_count == 1 && s.work.pending_batch_count == 1 && !s.work.exact_match() &&
      s.work.planned_work.command_count == 3 && s.work.submitted_work.command_count == 2 &&
      s.work.planned_work.batch_count == 3 && s.work.submitted_work.batch_count == 2 &&
      s.work.pending_command_count == 1 && s.work.planned_work.draw_count == 3 &&
      s.work.submitted_work.draw_count == 2 && s.work.unexpected_batch_count == 0 &&
      s.work.unexpected_command_count == 0 && s.work.unexpected_draw_count == 0 && s.upload_bytes.image_byte_count == 0 &&
      s.dropped_resources.resources.size() == 1 && s.dropped_resources.missing_submission_resource_count == 1 &&
      s.dropped_resources.resources[0].resource == RendererFrameResource{RendererPrimitiveKind::image, 1, 7} &&
      state.image_cache->retained_image_payload().byte_count == 0, "missing plan/submission/identity/retention is false");
  draws = {{Dx12MissingImageDraw{Rect{{128, 128}, {1, 1}}}, RendererPrimitiveKind::image, 0}};
  { Dx12FrameDiagnostics ledger(state, draws, 1); ledger.publish(); }
  require(state.diagnostics->planned_draws.image_count == 0 && state.diagnostics->dropped_resources.empty(),
      "fully clipped marker invented a visible missing resource");
}
void validation_before_absence() {
  Dx12State state;
  const auto asset = describe_image_asset(image);
  require(!state.image_cache->lookup({}), "typed lookup classified invalid identity as absence");
  for (unsigned cached = 0; cached < 2; ++cached) {
    if (cached) require(bool(state.image_cache->upload(state, image)), "CPU seed failed");
    for (unsigned kind = 0; kind < 6; ++kind) {
      Dx12PendingImageDraw request{ImageDraw{.bounds = {{0, 0}, {16, 16}}, .asset = asset}};
      if (kind == 0) request.image.asset.pixel_width = 0;
      if (kind == 1) request.image.asset.pixel_width = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION + 1;
      if (kind == 2) request.image.metadata.transform.skew_x = std::numeric_limits<float>::quiet_NaN();
      if (kind == 3) request.image.clip_rect = Rect{{0, 0}, {std::numeric_limits<float>::infinity(), 1}};
      if (kind == 4) request.image.sampling = static_cast<ImageSamplingMode>(255);
      if (kind == 5) request.image.metadata.transform.scale_x = std::numeric_limits<float>::max();
      const auto result = dx12_resolve_image_draw(state, request);
      require(!result && result.error().code == ErrorCode::invalid_argument && !state.failed,
          "invalid absent/cached request became skip/native failure");
    }
  }
  auto changed = asset; changed.pixel_width++;
  const auto mismatch = state.image_cache->lookup(changed);
  require(!mismatch && mismatch.error().code == ErrorCode::invalid_argument, "cached mismatch became absence");
  changed = asset; changed.stride = 0; changed.byte_size = 0;
  require(bool(state.image_cache->lookup(changed)) && state.image_cache->lookup(changed)->has_value(),
      "draw layout metadata incorrectly determines readiness");
  state.image_cache->invalidate(asset.id);
  std::vector<Dx12DrawCommand> conflict{request(asset, 0), request(asset, 1)};
  std::get<Dx12PendingImageDraw>(conflict[1].payload).image.asset.pixel_width++;
  Dx12ImageBatch batch;
  const auto result = batch.prepare(state, conflict);
  require(!result && result.error().code == ErrorCode::invalid_argument, "absent same-ID descriptors conflict silently");
}
}
int main() {
  try { ledger_and_marker(); validation_before_absence(); return 0; }
  catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
