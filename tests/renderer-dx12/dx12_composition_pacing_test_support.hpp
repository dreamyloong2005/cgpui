#pragma once

namespace cgpui {
class Renderer;
class RenderFrame;
struct RendererBackendInfo;
}
namespace cgpui_test {
class Dx12CompositionFirstBeginWindows;
struct Dx12PacingForwardApi;
bool composition_pacing_capture(cgpui::RenderFrame& frame, cgpui::Renderer& renderer,
                                bool blue, unsigned extent);
bool composition_pacing_hidden_extent(const Dx12CompositionFirstBeginWindows& windows, unsigned extent);
bool composition_pacing_resize_hidden_window(const Dx12CompositionFirstBeginWindows& windows,
                                             unsigned extent);
void composition_pacing_trace(const char* stage, const Dx12PacingForwardApi& api,
                              const cgpui::RendererBackendInfo& info);
} // namespace cgpui_test
