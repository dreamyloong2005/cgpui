#include "dx12_frame_diagnostics_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"
#include <array>
#include <type_traits>
#include <utility>

namespace cgpui {
void Dx12FrameDiagnostics::prepare_publication() {
  static constexpr std::array supported{
      RendererPrimitiveKind::solid_rect, RendererPrimitiveKind::rounded_rect,
      RendererPrimitiveKind::text, RendererPrimitiveKind::text_selection,
      RendererPrimitiveKind::text_caret, RendererPrimitiveKind::image,
      RendererPrimitiveKind::box_shadow, RendererPrimitiveKind::backdrop_blur};
  planned_work_.draw_count = snapshot_.planned_draws.total_count;
  submitted_work_.draw_count = snapshot_.submitted_draws.total_count;
  // Copy-demand observations are finalized before successful cache aging.
  planned_work_.upload_byte_count = snapshot_.upload_bytes.total_byte_count;
  submitted_work_.upload_byte_count = snapshot_.upload_bytes.total_byte_count;
  snapshot_.work = compare_renderer_frame_work(planned_work_, submitted_work_);
  snapshot_.dropped_resources = classify_renderer_dropped_resources(
      planned_resources_, submitted_resources_, supported);
  snapshot_.backdrop_blur.offscreen_targets_allocated =
      state_.backdrop_blur && !state_.backdrop_blur->frame_targets.empty();
}

void Dx12FrameDiagnostics::publish_prepared() noexcept {
  static_assert(std::is_nothrow_assignable_v<decltype(state_.diagnostics)&,
                                            RendererFrameDiagnosticSnapshot&&>);
  state_.diagnostics = std::move(snapshot_);
  published_ = true;
}

void Dx12FrameDiagnostics::complete_successful_frame() {
  if (published_) return;
  // Existing diagnostic classification may allocate; perform it before aging.
  prepare_publication();
  state_.image_cache->finish_frame();
  const auto retained = state_.image_cache->retained_image_payload();
  snapshot_.retained_image_byte_count = retained.byte_count;
  snapshot_.retained_image_byte_count_saturated = retained.saturated;
  publish_prepared();
}
} // namespace cgpui
