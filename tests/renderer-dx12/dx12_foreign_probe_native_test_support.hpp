#pragma once
#include "dx12_foreign_probe_control_test_support.hpp"
#include "dx12_pacing_native_test_support.hpp"
#include <vector>

namespace cgpui_test {
struct ForeignProbeSnapshot {
  const cgpui::RendererFramePixels* pixels = nullptr;
  const unsigned char* data = nullptr;
  std::vector<unsigned char> bytes;
  const void* image_cache = nullptr;
  const void* text_cache = nullptr;
  cgpui::RendererBackendInfo info;
  HWND hwnd = nullptr;
  UINT width = 0, height = 0;
  float scale = 0;
  unsigned closes = 0;
};
struct ForeignProbeFixture {
  Dx12Window window;
  Dx12PacingForwardApi pacing;
  std::shared_ptr<cgpui::Dx12State> state;
  ~ForeignProbeFixture();
  bool initialize();
  bool capture(bool origin, bool recovered, HANDLE cancel);
  bool remember(ForeignProbeSnapshot& snapshot);
  bool recover_once(const ForeignProbeSnapshot& before);
};
// Both callers keep stable strong pins outside every operation gate. This leaf
// deliberately bypasses production enumeration, not production native teardown.
int foreign_probe_retire(cgpui::Dx12State& peer, Dx12PacingForwardApi& pacing,
    const ForeignProbeSnapshot& before, ForeignProbeMode mode);
bool foreign_probe_preserved(const cgpui::Dx12State& peer,
    const ForeignProbeSnapshot& before);
bool foreign_probe_filter_excludes(cgpui::Dx12State& origin,
    const std::shared_ptr<cgpui::Dx12State>& peer);
} // namespace cgpui_test
