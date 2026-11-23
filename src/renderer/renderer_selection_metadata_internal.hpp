#pragma once

#include "cgpui/renderer/renderer_frame.hpp"

namespace cgpui::detail {

std::unique_ptr<Renderer> renderer_with_selection_metadata(
    std::unique_ptr<Renderer> renderer, RendererBackendSelection selection);

} // namespace cgpui::detail
