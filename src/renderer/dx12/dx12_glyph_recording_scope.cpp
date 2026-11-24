#include "dx12_glyph_recording_scope_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_glyph_native_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include "dx12_glyph_command_api_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_texture_upload_frame_internal.hpp"
#include "dx12_text_cache_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Dx12GlyphRecordingScope::Dx12GlyphRecordingScope(Dx12State& state) noexcept : state_(state) {
  entered_ = state.texture_renderer->glyph_native->enter_scope();
}
Dx12GlyphRecordingScope::~Dx12GlyphRecordingScope() {
  if (!entered_) return;
  if (open_ && !attempted_close_) cleanup_close_ = close();
  if (open_ && !executed_) state_.texture_renderer->glyph_native->abandon_unexecuted();
  state_.texture_renderer->glyph_native->leave_scope();
}
bool Dx12GlyphRecordingScope::entered() const noexcept { return entered_; }
Result<void> Dx12GlyphRecordingScope::reset_and_begin() {
  auto* injected_commands = state_.texture_renderer->glyph_native->command_api;
  auto& commands = injected_commands ? *injected_commands : dx12_native_glyph_command_api();
  HRESULT result = commands.reset_allocator(*state_.allocator.Get());
  if (FAILED(result)) return std::unexpected(dx12_error(result, "allocator reset"));
  result = commands.reset_list(*state_.commands.Get(), *state_.allocator.Get(), state_.pipeline.Get());
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command list reset"));
  opened_after_reset(); // P before rewind; guard already owns all exceptional exits.
  state_.texture_renderer->glyph_native->begin_recording(state_.text_cache->lineage);
  if (auto prepared = dx12_begin_texture_upload_frame(state_); !prepared) return prepared;
  dx12_observe_glyph_lifetime(state_);
  return {};
}
void Dx12GlyphRecordingScope::opened_after_reset() noexcept {
  open_ = true; // Must precede all fallible post-Reset metadata preparation.
  state_.command_reuse->parent_reset(state_); // Same actual successful DIRECT Reset establishes reuse P.
  if (state_.command_observation) state_.command_observation->parent_reset(*state_.commands.Get(), *state_.allocator.Get());
  state_.texture_renderer->glyph_native->parent_reset();
  dx12_observe_glyph_lifetime(state_);
}
HRESULT Dx12GlyphRecordingScope::close() noexcept {
  if (attempted_close_) return cleanup_close_;
  attempted_close_ = true; // Failed Close is still attempted exactly once.
  auto* injected = state_.texture_renderer->glyph_native->command_api;
  auto& api = injected ? *injected : dx12_native_glyph_command_api();
  cleanup_close_ = api.close(*state_.commands.Get());
  state_.texture_renderer->glyph_native->record_close(cleanup_close_);
  dx12_observe_command(state_, {.kind = Dx12CommandEventKind::parent_closed,
      .list = state_.commands.Get(), .result = cleanup_close_});
  return cleanup_close_;
}
void Dx12GlyphRecordingScope::executed() noexcept {
  executed_ = true;
  state_.submitted = true;
  state_.texture_renderer->glyph_native->mark_executed();
}
} // namespace cgpui
