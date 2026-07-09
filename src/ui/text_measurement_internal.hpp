#pragma once

#include "cgpui/ui/text_measurement.hpp"

#include <vector>

namespace cgpui {

[[nodiscard]] std::vector<TextGraphemeColumn> build_text_grapheme_columns(
    const TextShapeRun& run);

} // namespace cgpui
