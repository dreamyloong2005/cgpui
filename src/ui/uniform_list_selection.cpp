#include "cgpui/ui/uniform_list_selection.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace cgpui {
namespace {
[[nodiscard]] bool selectable(const UniformListItemIdentity& item) {
  return !item.recycled && item.content_bounds.size.width > 0.0F &&
         item.content_bounds.size.height > 0.0F;
}
[[nodiscard]] UniformListSelection selection_from(
    const UniformListItemIdentity& item, UniformListSelectionSource source) {
  return UniformListSelection{
      .index = item.index, .key = item.key, .element_id = item.element_id,
      .source = source};
}
[[nodiscard]] std::vector<const UniformListItemIdentity*> selectable_items(
    std::span<const UniformListItemIdentity> items) {
  std::vector<const UniformListItemIdentity*> result;
  for (const UniformListItemIdentity& item : items) {
    if (selectable(item)) {
      result.push_back(&item);
    }
  }
  return result;
}
[[nodiscard]] std::optional<std::size_t> selected_position(
    const std::vector<const UniformListItemIdentity*>& items,
    const UniformListSelectionState& state) {
  const std::optional<ElementKey> key = state.selected_key();
  if (key.has_value()) {
    const auto it = std::find_if(
        items.begin(), items.end(), [&key](const UniformListItemIdentity* item) {
          return item != nullptr && item->key == *key;
        });
    if (it != items.end()) {
      return static_cast<std::size_t>(it - items.begin());
    }
  }
  const std::optional<std::size_t> index = state.selected_index();
  if (!index.has_value()) {
    return std::nullopt;
  }
  const auto it = std::find_if(
      items.begin(), items.end(), [index](const UniformListItemIdentity* item) {
        return item != nullptr && item->index == *index;
      });
  return it == items.end()
             ? std::nullopt
             : std::optional<std::size_t>(
                   static_cast<std::size_t>(it - items.begin()));
}
} // namespace
bool UniformListSelectionState::empty() const { return !selection_.has_value(); }
std::optional<UniformListSelection> UniformListSelectionState::selection() const {
  return selection_;
}
std::optional<std::size_t> UniformListSelectionState::selected_index() const {
  return selection_.has_value() ? std::optional<std::size_t>(selection_->index)
                                : std::nullopt;
}
std::optional<ElementKey> UniformListSelectionState::selected_key() const {
  return selection_.has_value() ? std::optional<ElementKey>(selection_->key)
                                : std::nullopt;
}
bool UniformListSelectionState::selected(const ElementKey& key) const {
  return selection_.has_value() && selection_->key == key;
}
void UniformListSelectionState::set(UniformListSelection selection) {
  selection_ = std::move(selection);
}
void UniformListSelectionState::clear() { selection_.reset(); }
std::optional<UniformListSelection> select_uniform_list_item_at_point(
    std::span<const UniformListItemIdentity> items,
    Point point,
    UniformListSelectionSource source) {
  for (const UniformListItemIdentity& item : items) {
    if (selectable(item) && contains(item.content_bounds, point)) {
      return selection_from(item, source);
    }
  }
  return std::nullopt;
}
std::optional<UniformListSelection> move_uniform_list_selection(
    std::span<const UniformListItemIdentity> items,
    const UniformListSelectionState& state,
    UniformListSelectionDirection direction) {
  const std::vector<const UniformListItemIdentity*> selectable =
      selectable_items(items);
  if (selectable.empty()) {
    return std::nullopt;
  }
  std::size_t target = 0;
  const std::optional<std::size_t> current =
      selected_position(selectable, state);
  if (direction == UniformListSelectionDirection::last) {
    target = selectable.size() - 1;
  } else if (direction == UniformListSelectionDirection::next &&
             current.has_value()) {
    target = std::min(*current + 1, selectable.size() - 1);
  } else if (direction == UniformListSelectionDirection::previous &&
             current.has_value()) {
    target = *current == 0 ? 0 : *current - 1;
  }
  return selection_from(*selectable[target], UniformListSelectionSource::keyboard);
}
} // namespace cgpui
