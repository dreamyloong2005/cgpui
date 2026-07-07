#pragma once

#include "cgpui/ui/uniform_list.hpp"

#include <optional>
#include <span>

namespace cgpui {

enum class UniformListSelectionSource {
  pointer,
  keyboard,
};

enum class UniformListSelectionDirection {
  previous,
  next,
  first,
  last,
};

struct UniformListSelection {
  std::size_t index = 0;
  ElementKey key;
  ElementId element_id;
  UniformListSelectionSource source = UniformListSelectionSource::pointer;
};

class UniformListSelectionState {
 public:
  [[nodiscard]] bool empty() const;
  [[nodiscard]] std::optional<UniformListSelection> selection() const;
  [[nodiscard]] std::optional<std::size_t> selected_index() const;
  [[nodiscard]] std::optional<ElementKey> selected_key() const;
  [[nodiscard]] bool selected(const ElementKey& key) const;

  void set(UniformListSelection selection);
  void clear();

 private:
  std::optional<UniformListSelection> selection_;
};

[[nodiscard]] std::optional<UniformListSelection>
select_uniform_list_item_at_point(
    std::span<const UniformListItemIdentity> items,
    Point point,
    UniformListSelectionSource source);

[[nodiscard]] std::optional<UniformListSelection> move_uniform_list_selection(
    std::span<const UniformListItemIdentity> items,
    const UniformListSelectionState& state,
    UniformListSelectionDirection direction);

} // namespace cgpui
