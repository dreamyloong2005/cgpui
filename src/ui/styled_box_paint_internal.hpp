#pragma once

#include "cgpui/ui/paint.hpp"

#include <optional>

namespace cgpui {

void paint_styled_box_base(
    PaintList& paint_list,
    const std::optional<Rect>& bounds,
    const Style& style);

} // namespace cgpui
