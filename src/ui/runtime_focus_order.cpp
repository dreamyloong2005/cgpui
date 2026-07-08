#include "ui_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

struct FocusOrderEntry {
  ElementId element_id;
  std::optional<int> tab_index;
  std::size_t tree_order = 0;
};

bool has_positive_tab_index(const FocusOrderEntry& entry) {
  return entry.tab_index.has_value() && *entry.tab_index > 0;
}

} // namespace

std::vector<ElementId> WindowRuntime::ordered_focusable_element_ids() const {
  std::vector<FocusOrderEntry> entries;
  if (static_element_tree_installed()) {
    std::size_t tree_order = 0;
    static_element_tree_.for_each_preorder(
        [&](const StaticElementNode& node) {
          if (!node.enabled || !node.focusable) {
            return;
          }
          entries.push_back(FocusOrderEntry{
              .element_id = node.id,
              .tree_order = tree_order,
          });
          tree_order += 1;
        });
  }

  if (owned_element_tree_ == nullptr) {
    std::vector<ElementId> focusable_ids;
    focusable_ids.reserve(entries.size());
    for (const FocusOrderEntry& entry : entries) {
      focusable_ids.push_back(entry.element_id);
    }
    return focusable_ids;
  }

  std::size_t tree_order = 0;
  for (ElementId element_id : owned_element_tree_->enabled_preorder_ids()) {
    const Element* element = routed_element(element_id);
    if (element == nullptr || !element->focusable()) {
      continue;
    }

    const std::optional<int> tab_index =
        element->focus_metadata().tab_index;
    if (tab_index.has_value() && *tab_index < 0) {
      continue;
    }

    entries.push_back(FocusOrderEntry{
        .element_id = element_id,
        .tab_index = tab_index,
        .tree_order = tree_order,
    });
    tree_order += 1;
  }

  std::stable_sort(
      entries.begin(),
      entries.end(),
      [](const FocusOrderEntry& lhs, const FocusOrderEntry& rhs) {
        const bool lhs_positive = has_positive_tab_index(lhs);
        const bool rhs_positive = has_positive_tab_index(rhs);
        if (lhs_positive != rhs_positive) {
          return lhs_positive;
        }
        if (lhs_positive && lhs.tab_index != rhs.tab_index) {
          return *lhs.tab_index < *rhs.tab_index;
        }
        return lhs.tree_order < rhs.tree_order;
      });

  std::vector<ElementId> focusable_ids;
  focusable_ids.reserve(entries.size());
  for (const FocusOrderEntry& entry : entries) {
    focusable_ids.push_back(entry.element_id);
  }
  return focusable_ids;
}

} // namespace cgpui
