#pragma once

#include "cgpui/ui/element_builder_core.hpp"

namespace cgpui {

[[nodiscard]] ElementBuilder div();
[[nodiscard]] ElementBuilder h_flex();
[[nodiscard]] ElementBuilder v_flex();
[[nodiscard]] ElementBuilder h_stack();
[[nodiscard]] ElementBuilder v_stack();

} // namespace cgpui
