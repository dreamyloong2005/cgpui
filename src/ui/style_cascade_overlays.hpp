#pragma once

#include "cgpui/ui/style_cascade.hpp"

namespace cgpui::style_cascade_detail {

[[nodiscard]] StyleOverlay style_base_overlay(const Style& style);
[[nodiscard]] Style apply_style_state(
    Style style,
    const StyleState& state,
    StyleStateFlags flags);

} // namespace cgpui::style_cascade_detail
