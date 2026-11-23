#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dx12_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"

namespace cgpui_test {
struct Dx12BackdropFallbackFixture {
  Dx12Window window;
  std::shared_ptr<cgpui::Dx12State> state;
  cgpui::RendererEffectSupport actual_support;

  bool initialize(float scale = 1);
  void force_unsupported();
  std::unique_ptr<cgpui::Dx12Frame> begin();
  bool capture(cgpui::Dx12Frame& frame);
  bool pixel(float x, float y, std::array<unsigned char, 4> expected) const;
};
cgpui::BackdropBlurDraw fallback_panel();
void fallback_split_scene(cgpui::RenderFrame& frame);
void fallback_foreground(cgpui::RenderFrame& frame);
bool fallback_diagnostics(const Dx12BackdropFallbackFixture& fixture,
    std::size_t planned_commands, std::size_t submitted_commands,
    std::size_t solids, std::size_t rounded);
} // namespace cgpui_test
