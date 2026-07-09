#pragma once

#include "cgpui/ui/text_measurement.hpp"

#include <span>
#include <vector>

namespace cgpui {

[[nodiscard]] std::vector<TextGraphemeColumn> build_text_grapheme_columns(
    const TextShapeRun& run);

[[nodiscard]] std::vector<TextBidiRun> build_text_bidi_runs(
    const TextShapeRun& run,
    std::span<const TextGraphemeColumn> columns);

} // namespace cgpui
