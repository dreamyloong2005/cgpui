#pragma once

#include "dx12_pacing_native_test_support.hpp"

namespace cgpui_test {
bool pacing_begin_debug_enable();
bool pacing_begin_debug_clean(const cgpui::RendererBackendInfo& info);
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> pacing_begin_bounded(
    cgpui::Renderer& renderer, Dx12PacingForwardApi& api);
int pacing_first_begin_case(bool visible);
} // namespace cgpui_test
