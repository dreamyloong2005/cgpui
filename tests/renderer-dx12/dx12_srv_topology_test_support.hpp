#pragma once

#include "dx12_texture_upload_test_support.hpp"
#include <cstdint>
#include <vector>

namespace cgpui_test {
struct SrvNativeObservation {
  std::uintptr_t heap_identity = 0;
  UINT64 gpu = 0;
  UINT64 gpu_base = 0;
  UINT capacity = 0;
  UINT stride = 0;
};
// Copies scalar observations only; no COM owner escapes the inspection.
bool srv_native_observations(const cgpui::Dx12State& state,
                             std::vector<SrvNativeObservation>& records);
bool srv_topology(const cgpui::Dx12State& state, std::size_t views, std::size_t heaps);
std::shared_ptr<cgpui::Dx12State> srv_native_state(Dx12Window* window = nullptr, bool alpha = false);
cgpui::ImageAsset srv_tile_image(unsigned index);
bool srv_render_tiles(const std::shared_ptr<cgpui::Dx12State>& state, bool upload, bool blur,
                      std::optional<std::size_t> expected_upload = std::nullopt);
int srv_sharing_case(bool blur);
} // namespace cgpui_test
