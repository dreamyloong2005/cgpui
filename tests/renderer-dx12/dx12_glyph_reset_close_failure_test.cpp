#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_command_api_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct FailingCommands final : Dx12GlyphCommandApi {
  unsigned fail = 0, allocator_calls = 0, list_calls = 0, close_calls = 0;
  HRESULT reset_allocator(ID3D12CommandAllocator& allocator) noexcept override {
    ++allocator_calls;
    return fail == 1 ? E_ACCESSDENIED : dx12_native_glyph_command_api().reset_allocator(allocator);
  }
  HRESULT reset_list(ID3D12GraphicsCommandList& list, ID3D12CommandAllocator& allocator,
                     ID3D12PipelineState* pipeline) noexcept override {
    ++list_calls;
    return fail == 2 ? E_ACCESSDENIED : dx12_native_glyph_command_api().reset_list(list, allocator, pipeline);
  }
  HRESULT close(ID3D12GraphicsCommandList& list) noexcept override {
    ++close_calls;
    return fail == 3 ? E_ACCESSDENIED : dx12_native_glyph_command_api().close(list);
  }
};
int run(unsigned fault) {
  FailingCommands api; // Outlives the borrowed state seam.
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "Reset/Close failure fixture");
  auto& owner = *state->texture_renderer->glyph_native;
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  auto draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(2, "Reset/Close completed source");
  const auto pixels = state->pixels->rgba8;
  const auto diagnostics = *state->diagnostics;
  glyph_add_record(*state, 2);
  auto ab = glyph_publish(*state);
  draw = {}; a.reset(); // Old source expires after production pre-Reset retained.clear.
  api.fail = fault; owner.command_api = &api;
  draw = glyph_draw(ab);
  const auto result = dx12_submit_frame(*state, {1, 0, 0, 1}, std::span{&draw, 1}, true, 1);
  if (result || result.error().message.find("80070005") == std::string::npos ||
      state->submitted || state->pixels->rgba8 != pixels || !diagnostic_same(&*state->diagnostics, diagnostics))
    return dx12_fail(3, "injected native phase error replaced primary error or published success");
  if (fault < 3) {
    if (owner.stats().detached_nodes != 1 || !owner.stats().resources ||
        state->srv_pool->stats().live_slots != 1 || api.close_calls ||
        api.allocator_calls != 1 || api.list_calls != (fault == 2 ? 1U : 0U))
      return dx12_fail(4, "allocator/list Reset failure released G-only old dependencies");
  } else {
    if (api.close_calls != 1 || owner.stats().close_attempts != 1 ||
        owner.stats().last_close != E_ACCESSDENIED || !owner.stats().pins || !owner.stats().resources ||
        state->texture_renderer->native_textures.at(ab.get()).uploaded ||
        owner.stats().executed_unknown)
      return dx12_fail(5, "failed Close retried or released/promoted unexecuted target");
  }
  // Lower-level ownership fixture: release parent commands after known old-G/new-never-executed.
  // This is not a public frame retry/recovery success claim.
  dx12_release_native_objects(*state);
  if (owner.stats().pins || owner.stats().resources || owner.stats().completed_families ||
      state->srv_pool->stats().live_slots || state->device)
    return dx12_fail(6, "safe parent release did not retire exact old/new owners");
  owner.command_api = nullptr;
  return 0;
}
}
int main() { for (unsigned fault = 1; fault <= 3; ++fault) if (const auto result = run(fault)) return result; return 0; }
