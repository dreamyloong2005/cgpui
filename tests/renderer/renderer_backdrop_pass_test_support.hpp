#pragma once

#include "cgpui/renderer/renderer_frame.hpp"

namespace cgpui_test {
// Backend-specific wrappers provide their exact topology, never a common tolerance.
[[nodiscard]] int check_backdrop_pass_counts(cgpui::Renderer& renderer,
    unsigned effects, unsigned expected_passes, unsigned frames);
} // namespace cgpui_test
