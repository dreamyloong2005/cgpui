#pragma once

#include "cgpui/ui/text_shape.hpp"

namespace native_text_test {

// Advance width of run.text laid out by the platform text stack directly
// with the run's primary face, or a negative value when unavailable.
[[nodiscard]] float reference_layout_width(const cgpui::TextShapeRun& run);

} // namespace native_text_test
