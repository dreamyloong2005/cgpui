#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  std::atomic_bool released = false;
  auto state = srv_native_state(&window);
  if (!state || !srv_render_tiles(state, true, false)) return dx12_fail(1, "SRV healthy orphan fixture failed");
  if (!srv_heap_sentinel(*state->texture_renderer->native_textures.begin()->second.srv.heap(), released))
    return dx12_fail(2, "healthy orphan heap sentinel unavailable");
  state->frame_active = true;
  auto frame = std::make_unique<Dx12Frame>(state);
  frame->clear({0, 0, 0, 1});
  frame->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(srv_tile_image(0))});
  state->owner_alive = false; // Controlled private owner seam, not a second public facade implementation.
  std::weak_ptr<Dx12State> weak = state;
  state.reset(); // Only the already-acquired frame now owns native state/pages.
  if (released.load() || !frame->request_pixel_capture() || !frame->present())
    return dx12_fail(3, "healthy acquired orphan lost its SRV heap or could not present");
  { auto retained = weak.lock();
    if (!retained || !retained->pixels || released.load() ||
        !dx12_pixel(&*retained->pixels, 8, 8, {255, 0, 0, 255}, 0))
      return dx12_fail(4, "healthy orphan descriptor identity/pixel changed");
  }
  frame.reset();
  return weak.expired() && released.load() ? 0 : dx12_fail(5, "final orphan state leaked its native page");
}
