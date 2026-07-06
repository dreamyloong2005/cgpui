#pragma once

#include "cgpui/ui/element_core.hpp"

#include <memory>
#include <vector>

namespace cgpui {

[[nodiscard]] std::vector<const Element*> paint_ordered_children(
    const std::vector<std::unique_ptr<Element>>& children);
[[nodiscard]] std::vector<const Element*> hit_test_ordered_children(
    const std::vector<std::unique_ptr<Element>>& children);
[[nodiscard]] std::vector<Element*> event_ordered_children(
    std::vector<std::unique_ptr<Element>>& children);

} // namespace cgpui
