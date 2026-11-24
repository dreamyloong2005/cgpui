#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"
#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  SrvFailingHeapApi api;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "SRV transactional realization fixture unavailable");
  state->srv_pool = std::make_unique<Dx12SrvDescriptorPool>(4, &api);
  const auto image = srv_tile_image(0);
  if (!state->image_cache->upload(*state, image)) return dx12_fail(2, "SRV CPU snapshot unavailable");
  const auto source = state->image_cache->find(describe_image_asset(image));
  if (!source || !dx12_begin_texture_upload_frame(*state) || FAILED(state->allocator->Reset()) ||
      FAILED(state->commands->Reset(state->allocator.Get(), nullptr))) return dx12_fail(3, "unsubmitted list unavailable");
  api.fail_on = 1;
  const auto failed = dx12_prepare_texture(*state, *source);
  if (failed || failed.error().message.find("80070005") == std::string::npos ||
      state->srv_pool->stats().live_slots || state->srv_pool->stats().page_count ||
      state->texture_renderer->native_textures.at(source->get()).resource ||
      state->texture_renderer->native_textures.at(source->get()).srv.size())
    return dx12_fail(4, "failed native realization published a resource without a valid SRV lease");
  api.fail_on = 0;
  const auto prepared = dx12_prepare_texture(*state, *source);
  if (!prepared || !(*prepared)->resource || (*prepared)->srv.size() != 1 || (*prepared)->uploaded ||
      state->srv_pool->stats().live_slots != 1 || FAILED(state->commands->Close()))
    return dx12_fail(5, "unsubmitted realization retry was not atomic");
  const auto slot = (*prepared)->srv.gpu().ptr;
  // Discard actual copy/barrier commands: no ExecuteCommandLists occurred.
  for (unsigned pass = 0; pass != 2; ++pass) {
    state->frame_active = true;
    Dx12Frame frame(state);
    frame.draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(image)});
    if (!frame.request_pixel_capture() || !frame.present() || !state->pixels ||
        !dx12_pixel(&*state->pixels, 8, 8, {255, 0, 0, 255}, 0) || !(*prepared)->uploaded ||
        (*prepared)->srv.gpu().ptr != slot || state->srv_pool->stats().successful_page_creations != 1 ||
        state->diagnostics->upload_bytes.image_byte_count != (pass ? 0U : 4U))
      return dx12_fail(6, "discarded list lost/reused the lease or skipped the next genuine upload");
  }
  return dx12_check_debug_errors(*state) ? 0 : dx12_fail(7, "transactional realization debug error");
}
