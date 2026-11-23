#include "../../src/renderer/dx12/dx12_draw_commands_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_diagnostics_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace cgpui;

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
RectGeometry two_rectangles() {
  const std::array rects{
      SolidRect{.rect = {{2, 2}, {8, 8}}, .color = {1, 0, 0, 1}},
      SolidRect{.rect = {{16, 2}, {8, 8}}, .color = {0, 1, 0, 1}}};
  auto geometry = build_solid_rect_geometry(rects);
  require(geometry.draws.size() == 2, "ledger fixture requires two independent ranges");
  return geometry;
}

void missing_resource_identity() {
  Dx12State state;
  state.width = state.height = 64;
  const std::vector<Dx12DrawCommand> draws{
      {two_rectangles(), RendererPrimitiveKind::text_selection, 1, 4}};
  {
    Dx12FrameDiagnostics ledger(state, draws, 2);
    ledger.begin_draw(0);
    ledger.submitted(0);
    ledger.publish();
  }
  require(state.diagnostics.has_value() && !state.active_diagnostics,
          "publication must survive the scoped observer without retaining its pointer");
  const auto& snapshot = *state.diagnostics;
  require(snapshot.planned_draws.text_selection_count == 2 &&
              snapshot.submitted_draws.text_selection_count == 1 &&
              snapshot.work.pending_draw_count == 1 &&
              snapshot.work.submitted_work.batch_count == 1 &&
              snapshot.work.submitted_work.command_count == 1,
          "missing range must remain pending without flattening its original primitive kind");
  const auto& dropped = snapshot.dropped_resources;
  require(dropped.resources.size() == 1 && dropped.missing_submission_resource_count == 1 &&
              dropped.unsupported_primitive_count == 0 &&
              dropped.resources.front().resource ==
                  RendererFrameResource{RendererPrimitiveKind::text_selection, 1, 5} &&
              dropped.resources.front().reason == RendererDroppedResourceReason::missing_submission_resource,
          "missing resource must preserve command identity plus the original range offset");
}

void unexpected_submission_is_observed() {
  Dx12State state;
  state.width = state.height = 64;
  auto geometry = two_rectangles();
  geometry.draws.resize(1);
  const std::vector<Dx12DrawCommand> draws{
      {std::move(geometry), RendererPrimitiveKind::rounded_rect, 1, 7}};
  {
    Dx12FrameDiagnostics ledger(state, draws, 2);
    ledger.begin_draw(0);
    ledger.submitted(0);
    // A deliberately unmatched native observation must not be gated by the plan.
    ledger.submitted(2);
    ledger.publish();
  }
  require(state.diagnostics.has_value(), "unexpected-resource candidate was not published");
  const auto& snapshot = *state.diagnostics;
  require(snapshot.planned_draws.rounded_rect_count == 1 &&
              snapshot.submitted_draws.rounded_rect_count == 2 &&
              snapshot.work.unexpected_draw_count == 1 && snapshot.work.pending_draw_count == 0 &&
              snapshot.work.submitted_work.command_count == 1 &&
              snapshot.work.submitted_work.batch_count == 1 && snapshot.dropped_resources.empty(),
          "actual native submissions must be independent of planned-resource membership");
}

void culled_ranges_keep_their_indices() {
  Dx12State state;
  state.width = state.height = 64;
  auto geometry = two_rectangles();
  geometry.draws.front().clip_rect = Rect{{128, 128}, {8, 8}};
  const std::vector<Dx12DrawCommand> draws{
      {std::move(geometry), RendererPrimitiveKind::text_caret, 0, 9}};
  {
    Dx12FrameDiagnostics ledger(state, draws, 1);
    ledger.begin_draw(0);
    ledger.submitted(1);
    ledger.publish();
  }
  const auto& snapshot = *state.diagnostics;
  require(snapshot.work.exact_match() && snapshot.planned_draws.text_caret_count == 1 &&
              snapshot.submitted_draws.text_caret_count == 1 && snapshot.dropped_resources.empty(),
          "culling an earlier range must not renumber the later native resource");
}

void upload_saturation_and_scope() {
  Dx12State state;
  const std::vector<Dx12DrawCommand> draws;
  const auto maximum = std::numeric_limits<std::size_t>::max();
  {
    Dx12FrameDiagnostics ledger(state, draws, 0);
    require(state.active_diagnostics == &ledger, "candidate observer is not active in its scope");
    ledger.uploaded(false, 17);
  }
  require(!state.active_diagnostics && !state.diagnostics,
          "unpublished candidate must clear its observer without creating a completed snapshot");
  {
    Dx12FrameDiagnostics ledger(state, draws, 0);
    ledger.uploaded(false, maximum);
    ledger.publish();
  }
  require(state.diagnostics->upload_bytes.total_byte_count == maximum &&
              !state.diagnostics->upload_bytes.saturated,
          "exactly representable maximum upload is not arithmetic saturation");
  {
    Dx12FrameDiagnostics ledger(state, draws, 0);
    ledger.uploaded(false, 1);
  }
  require(!state.active_diagnostics && state.diagnostics->upload_bytes.total_byte_count == maximum,
          "abandoned candidate must leave the prior published snapshot unchanged");
  {
    Dx12FrameDiagnostics ledger(state, draws, 0);
    ledger.uploaded(false, maximum - 2);
    ledger.uploaded(false, 4);
    ledger.uploaded(true, 1);
    ledger.publish();
  }
  const auto& snapshot = *state.diagnostics;
  require(snapshot.upload_bytes.image_byte_count == maximum &&
              snapshot.upload_bytes.glyph_atlas_byte_count == 1 &&
              snapshot.upload_bytes.total_byte_count == maximum && snapshot.upload_bytes.saturated &&
              snapshot.work.planned_work.upload_byte_count == maximum &&
              snapshot.work.submitted_work.upload_byte_count == maximum && snapshot.work.exact_match(),
          "upload category and total counters must saturate independently instead of wrapping");
}
} // namespace

int main() {
  try {
    // Ledger unit coverage creates CPU state only; native drawing is tested separately.
    missing_resource_identity();
    unexpected_submission_is_observed();
    culled_ranges_keep_their_indices();
    upload_saturation_and_scope();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
