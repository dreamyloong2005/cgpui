#pragma once

#include "cgpui/ui/view.hpp"

#include <cstdint>
#include <vector>

namespace cgpui { struct WindowRuntimeContext; }

namespace cgpui_test {

struct Dx12RuntimeSceneState {
  unsigned epoch = 0;
  unsigned upload_intents = 0;
  unsigned paints = 0;
};

class Dx12RuntimeRecoveryScene final : public cgpui::View {
 public:
  Dx12RuntimeRecoveryScene(cgpui::ImageAssetId id, Dx12RuntimeSceneState& state);
  void paint(cgpui::PaintList& list, cgpui::Size viewport) override;

 private:
  cgpui::ImageAsset image_;
  Dx12RuntimeSceneState& state_;
};

// First call saves a nonblank glyph ROI; later calls compare its actual bytes.
bool dx12_runtime_scene_pixels(const cgpui::WindowRuntimeContext& context,
    unsigned epoch, std::vector<std::uint8_t>& glyph_reference);

} // namespace cgpui_test
