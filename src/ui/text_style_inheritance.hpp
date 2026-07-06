#pragma once

#include "cgpui/ui/style_box.hpp"

namespace cgpui {

[[nodiscard]] bool text_style_has_font(const Style& style);
[[nodiscard]] bool text_style_has_font_size(const Style& style);
[[nodiscard]] Style inherited_text_style(const Style& style);
[[nodiscard]] Style merge_inherited_text_style(
    const Style& inherited,
    const Style& local);

} // namespace cgpui
