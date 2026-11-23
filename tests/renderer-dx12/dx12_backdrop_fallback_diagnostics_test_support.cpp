#include "dx12_backdrop_fallback_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

namespace cgpui_test {
bool fallback_diagnostics(const Dx12BackdropFallbackFixture& fixture,
    std::size_t planned_commands, std::size_t submitted_commands,
    std::size_t solids, std::size_t rounded) {
  const auto& state = *fixture.state;
  const auto* value = state.diagnostics ? &*state.diagnostics : nullptr;
  if (!diagnostic_work(value, planned_commands, submitted_commands, solids + rounded))
    return false;
  const auto& counts = value->submitted_draws;
  const auto& work = value->work;
  return counts.solid_rect_count == solids && counts.rounded_rect_count == rounded &&
      counts.backdrop_blur_count == 0 && counts.text_count == 0 &&
      counts.text_selection_count == 0 && counts.text_caret_count == 0 &&
      counts.image_count == 0 && counts.box_shadow_count == 0 &&
      work.planned_work.batch_count == solids + rounded &&
      work.submitted_work.batch_count == solids + rounded &&
      work.pending_batch_count == 0 && work.unexpected_batch_count == 0 &&
      work.pending_upload_byte_count == 0 && work.unexpected_upload_byte_count == 0 &&
      work.exact_match() == (planned_commands == submitted_commands) &&
      value->upload_bytes.total_byte_count == 0 && value->dropped_resources.empty() &&
      value->backdrop_blur.blurred_backdrop_count == 0 &&
      value->backdrop_blur.offscreen_pass_count == 0 &&
      !value->backdrop_blur.offscreen_targets_allocated &&
      state.backdrop_blur->frame_targets.empty();
}
} // namespace cgpui_test
