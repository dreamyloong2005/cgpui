#include "dx12_frame_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_surface_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"
#include "dx12_geometry_upload_frame_internal.hpp"
#include "dx12_frame_pacing_internal.hpp"
#include "dx12_glyph_native_internal.hpp"
#include "dx12_glyph_recording_scope_internal.hpp"

#include <optional>
#include <type_traits>

namespace cgpui {
static_assert(std::is_nothrow_move_assignable_v<std::optional<RendererFramePixels>>);
Result<void> dx12_submit_frame(Dx12State& state, Color clear,
                              std::span<const Dx12DrawCommand> draws, bool capture,
                              std::size_t command_count, std::uint64_t admission_call_ns) {
  Dx12GlyphRecordingScope recording(state);
  if (!recording.entered()) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 nested native recording is not permitted"});
  if (auto resolved = dx12_require_resolved_images(draws); !resolved) return resolved;
  Dx12FrameDiagnostics diagnostics(state, draws, command_count, admission_call_ns);
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  if (state.texture_renderer->glyph_native->stats().executed_unknown)
    dx12_commit_texture_uploads(state); // Never promote an abandoned/unexecuted pending marker.
  diagnostics.finish_stage(RendererFrameTimingStage::pacing_wait);
  if (auto prepared = dx12_begin_geometry_upload_frame(state); !prepared) return prepared;
  dx12_begin_texture_frame(state);
  dx12_begin_backdrop_blur_frame(state);
  state.readback.Reset();
  if (capture) {
    if (auto prepared = dx12_prepare_capture(state); !prepared) return prepared;
  }
  diagnostics.finish_stage(RendererFrameTimingStage::resource_preparation);
  const UINT index = state.swapchain->GetCurrentBackBufferIndex();
  diagnostics.finish_stage(RendererFrameTimingStage::image_acquisition);
  if (auto prepared = recording.reset_and_begin(); !prepared) return prepared;
  auto& target = *state.targets[index].Get();
  dx12_transition(*state.commands.Get(), target, D3D12_RESOURCE_STATE_PRESENT,
                   D3D12_RESOURCE_STATE_RENDER_TARGET);
  auto handle = state.rtv_heap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += std::size_t(index) * state.rtv_stride;
  state.commands->OMSetRenderTargets(1, &handle, FALSE, nullptr);
  const auto premultiplied = dx12_surface_clear(state, clear);
  const float color[] = {premultiplied.r, premultiplied.g, premultiplied.b, premultiplied.a};
  state.commands->ClearRenderTargetView(handle, color, 0, nullptr);
  const D3D12_VIEWPORT viewport{0, 0, static_cast<float>(state.width),
                               static_cast<float>(state.height), 0, 1};
  state.commands->RSSetViewports(1, &viewport);
  state.commands->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  for (std::size_t group = 0; group < draws.size(); ++group) {
    diagnostics.begin_draw(group);
    const auto& draw = draws[group];
    const auto recorded = dx12_record_draw(state, draw);
    if (!recorded) {
      return recorded;
    }
  }
  if (capture) {
    dx12_transition(*state.commands.Get(), target, D3D12_RESOURCE_STATE_RENDER_TARGET,
                     D3D12_RESOURCE_STATE_COPY_SOURCE);
    dx12_record_capture(state, target);
  }
  dx12_transition(*state.commands.Get(), target,
      capture ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_RENDER_TARGET,
      D3D12_RESOURCE_STATE_PRESENT);
  const HRESULT result = recording.close();
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command list close"));
  diagnostics.finish_stage(RendererFrameTimingStage::command_recording);
  ID3D12CommandList* lists[] = {state.commands.Get()};
  state.queue->ExecuteCommandLists(1, lists);
  recording.executed(); // Immediate noexcept ownership marking after Execute.
  diagnostics.finish_stage(RendererFrameTimingStage::queue_submission);
  const HRESULT presented = state.swapchain->Present(1, 0);
  state.frame_pacing->presentation_attempted();
  // The fence follows presentation as well as drawing; resize/destruction
  // cannot retire resources while DXGI is still using this queue's frame.
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  dx12_commit_texture_uploads(state);
  if (FAILED(presented)) return std::unexpected(dx12_error(presented, "present"));
  std::optional<RendererFramePixels> candidate_pixels;
  if (capture) {
    auto read = dx12_read_capture(state);
    if (!read) return std::unexpected(read.error());
    candidate_pixels = std::move(*read);
  }
  if (auto checked = dx12_check_debug_errors(state); !checked) return checked;
  if (auto committed = dx12_commit_composition(state); !committed) return committed;
  diagnostics.finish_stage(RendererFrameTimingStage::presentation);
  diagnostics.complete_successful_frame();
  if (candidate_pixels) state.pixels = std::move(candidate_pixels);
  // DXGI_STATUS_OCCLUDED is a valid hidden-window presentation. The render
  // and optional readback completed; avoid a tight retry loop for hidden HWNDs.
  return {};
}
} // namespace cgpui
