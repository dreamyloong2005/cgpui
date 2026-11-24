#include "dx12_frame_diagnostics_internal.hpp"
#include "dx12_draw_commands_internal.hpp"
#include "dx12_state_internal.hpp"
#include <limits>

namespace cgpui {
namespace {
void saturating_add(std::size_t& count, std::size_t amount, bool& saturated) {
  if (amount > std::numeric_limits<std::size_t>::max() - count) {
    count = std::numeric_limits<std::size_t>::max();
    saturated = true;
  } else {
    count += amount;
  }
}
} // namespace

Dx12FrameDiagnostics::Dx12FrameDiagnostics(Dx12State& state,
    std::span<const Dx12DrawCommand> draws, std::size_t command_count,
    std::uint64_t admission_call_ns)
    : state_(state), draws_(draws), submitted_commands_(command_count) {
  renderer_add_frame_timing(snapshot_.timings, RendererFrameTimingStage::pacing_wait, admission_call_ns);
  planned_work_.command_count = command_count;
  plan();
  current_group_ = groups_.size();
  // This bounded CPU preparation includes diagnostic planning. Preparation
  // performed by RenderFrame draw calls before present is outside these laps.
  finish_stage(RendererFrameTimingStage::resource_preparation);
  state_.active_diagnostics = this;
}

Dx12FrameDiagnostics::~Dx12FrameDiagnostics() {
  if (state_.active_diagnostics == this) state_.active_diagnostics = nullptr;
}

void Dx12FrameDiagnostics::begin_draw(std::size_t group_index) {
  current_group_ = group_index;
}

void Dx12FrameDiagnostics::submitted(std::size_t local_resource_index) {
  if (current_group_ >= groups_.size()) return;
  auto& group = groups_[current_group_];
  const auto& draw = draws_[current_group_];
  if (local_resource_index > std::numeric_limits<std::size_t>::max() - draw.resource_offset)
    return;
  const RendererFrameResource resource{draw.kind, draw.command_index,
      draw.resource_offset + local_resource_index};
  // Hooks report each recorded native resource exactly once. Do not filter
  // against the plan: an unexpected submission must remain observable.
  submitted_resources_.push_back(resource);
  renderer_add_draw_count(snapshot_.submitted_draws, draw.kind);
  if (!group.submitted) {
    group.submitted = true;
    ++submitted_work_.batch_count;
  }
  if (!submitted_commands_[draw.command_index]) {
    submitted_commands_[draw.command_index] = true;
    ++submitted_work_.command_count;
  }
  if (draw.kind == RendererPrimitiveKind::backdrop_blur) {
    auto& blur = snapshot_.backdrop_blur;
    // This report follows the successful composite, after its downsample,
    // horizontal and vertical offscreen draws were actually recorded.
    bool saturated = false;
    saturating_add(blur.blurred_backdrop_count, 1, saturated);
    saturating_add(blur.offscreen_pass_count, 3, saturated);
  }
}

void Dx12FrameDiagnostics::uploaded(bool glyph, std::size_t bytes) {
  auto& uploads = snapshot_.upload_bytes;
  auto& count = glyph ? uploads.glyph_atlas_byte_count : uploads.image_byte_count;
  saturating_add(count, bytes, uploads.saturated);
  saturating_add(uploads.total_byte_count, bytes, uploads.saturated);
}

void Dx12FrameDiagnostics::finish_stage(RendererFrameTimingStage stage) {
  const auto now = Clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - stage_started_).count();
  renderer_add_frame_timing(snapshot_.timings, stage,
      elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0);
  stage_started_ = now;
}

void Dx12FrameDiagnostics::publish() {
  if (published_) return;
  prepare_publication();
  publish_prepared();
}
} // namespace cgpui
