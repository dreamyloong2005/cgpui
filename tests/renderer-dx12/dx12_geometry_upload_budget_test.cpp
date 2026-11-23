#include "dx12_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_geometry_upload_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_internal.hpp"
#include <chrono>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto state = std::make_shared<Dx12State>();
  if (!window.hwnd || !dx12_configure_surface(*state, window.descriptor()) ||
      !dx12_create_device(*state) || !dx12_create_pipeline(*state) || !dx12_create_swapchain(*state))
    return dx12_fail(1, "geometry upload budget fixture unavailable");
  for (unsigned pass = 0; pass != 3; ++pass) {
    state->frame_active = true;
    Dx12Frame frame(state);
    frame.clear({0, 0, 1, 1});
    for (unsigned index = 0; index != 128; ++index) {
      frame.draw_rect({.rect = {{float(index % 16) * 4, float(index / 16) * 8}, {4, 8}},
          .color = ((index + pass) % 2) ? Color{0, 1, 0, 1} : Color{1, 0, 0, 1}});
    }
    if (!frame.request_pixel_capture()) return dx12_fail(2, "upload budget capture unavailable");
    const auto start = std::chrono::steady_clock::now();
    if (!frame.present()) return dx12_fail(3, "geometry upload budget present failed");
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();
    const auto* pixels = state->pixels ? &*state->pixels : nullptr;
    for (unsigned index = 0; index != 128; ++index) {
      if (!dx12_pixel(pixels, (index % 16) * 4 + 2, (index / 16) * 8 + 4,
          ((index + pass) % 2) ? std::array<unsigned char, 4>{0,255,0,255}
                              : std::array<unsigned char, 4>{255,0,0,255}))
        return dx12_fail(4, "geometry slices aliased or retained stale pixels");
    }
    if (!state->diagnostics || state->diagnostics->submitted_draws.solid_rect_count != 128 ||
        !state->diagnostics->work.exact_match() || state->diagnostics->upload_bytes.total_byte_count)
      return dx12_fail(4, "geometry budget must retain real pixels and original work counts");
    const auto stats = state->geometry_uploads->stats();
    const auto resources = stats.page_count;
    std::cout << "GEOMETRY_UPLOAD_BUDGET frame=" << pass << " draws=128 resources=" << resources
              << " new_pages=" << stats.allocations_this_frame << " capacity=" << stats.capacity_bytes
              << " present_cpu_us=" << elapsed << '\n';
    if (resources == 0 || resources > 8 || !stats.used_bytes || state->texture_uploads->stats().page_count ||
        (pass == 0 ? stats.allocations_this_frame != stats.page_count : stats.allocations_this_frame != 0))
      return dx12_fail(5, "geometry upload resources still scale with every draw or are not reused");
  }
  return 0;
}
