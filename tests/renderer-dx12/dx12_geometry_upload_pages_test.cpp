#include "dx12_geometry_upload_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  Dx12State state;
  if (!window.hwnd || !dx12_configure_surface(state, window.descriptor()) ||
      !dx12_create_device(state) || !dx12_create_pipeline(state) || !dx12_create_swapchain(state))
    return dx12_fail(1, "multipage geometry fixture unavailable");
  for (unsigned pass = 0; pass != 3; ++pass) {
    const unsigned count = pass == 1 ? 128 : 8192;
    std::vector<SolidRect> rectangles;
    for (unsigned i = 0; i != count; ++i) {
      const auto cell = i % 128;
      rectangles.push_back({.rect = {{float(cell % 16) * 4, float(cell / 16) * 8}, {4, 8}},
          .color = ((cell + pass) % 2) ? Color{0, 1, 0, 1} : Color{1, 0, 0, 1}});
    }
    const SolidRect first{.rect = {{0, 0}, {4, 8}}, .color = {1, 1, 0, 1}};
    const SolidRect last{.rect = {{60, 56}, {4, 8}}, .color = {0, 1, 1, 1}};
    // One large payload forces an oversized vertex page; later payloads must
    // use disjoint nonzero-offset vertex/index slices in retained standard pages.
    const std::vector<Dx12DrawCommand> draws{
        {build_solid_rect_geometry(rectangles), RendererPrimitiveKind::solid_rect, 0},
        {build_solid_rect_geometry(std::span{&first, 1}), RendererPrimitiveKind::solid_rect, 1},
        {build_solid_rect_geometry(std::span{&last, 1}), RendererPrimitiveKind::solid_rect, 2}};
    if (!dx12_submit_frame(state, {0,0,1,1}, draws, true, 3) || !state.pixels)
      return dx12_fail(2, "multipage geometry capture/presentation failed");
    for (unsigned cell = 0; cell != 128; ++cell) {
      const std::array<unsigned char, 4> expected = cell == 0
          ? std::array<unsigned char, 4>{255,255,0,255} : cell == 127
          ? std::array<unsigned char, 4>{0,255,255,255} : ((cell + pass) % 2)
          ? std::array<unsigned char, 4>{0,255,0,255} : std::array<unsigned char, 4>{255,0,0,255};
      if (!dx12_pixel(&*state.pixels, (cell % 16) * 4 + 2, (cell / 16) * 8 + 4, expected))
        return dx12_fail(3, "oversize/offset geometry changed draw order or GPU pixels");
    }
    const auto stats = state.geometry_uploads->stats();
    if (!state.diagnostics || state.diagnostics->work.planned_work.command_count != 3 ||
        state.diagnostics->submitted_draws.total_count != count + 2 ||
        !state.diagnostics->work.exact_match() || state.diagnostics->upload_bytes.total_byte_count ||
        !state.uploads.empty()) return dx12_fail(4, "geometry pages changed logical diagnostic work");
    if (pass == 1 ? (stats.capacity_bytes > 4 * geometry_page_bytes || stats.allocations_this_frame != 0)
                  : (stats.page_count < 2 || stats.capacity_bytes <= geometry_page_bytes ||
                     stats.allocations_this_frame == 0))
      return dx12_fail(5, "native multipage reuse or transient retirement budget differs");
    std::cout << "GEOMETRY_UPLOAD_PAGES pass=" << pass << " resources=" << stats.page_count
              << " capacity=" << stats.capacity_bytes << " new=" << stats.allocations_this_frame << '\n';
  }
  return 0;
}
