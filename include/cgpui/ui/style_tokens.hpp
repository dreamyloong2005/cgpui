#pragma once

#include "cgpui/core/geometry.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cgpui {

struct StyleClassId {
  std::string value;

  friend bool operator==(
      const StyleClassId&,
      const StyleClassId&) = default;
};

struct ThemeTokenId {
  std::string value;

  friend bool operator==(
      const ThemeTokenId&,
      const ThemeTokenId&) = default;
};

[[nodiscard]] inline StyleClassId style_class(std::string_view value) {
  return StyleClassId{.value = std::string(value)};
}

[[nodiscard]] inline ThemeTokenId theme_token(std::string_view value) {
  return ThemeTokenId{.value = std::string(value)};
}

class StyleClasses {
 public:
  StyleClasses& add(StyleClassId id) {
    if (!contains(id)) {
      ids_.push_back(std::move(id));
    }
    return *this;
  }

  [[nodiscard]] bool contains(const StyleClassId& id) const {
    return std::find(ids_.begin(), ids_.end(), id) != ids_.end();
  }

  [[nodiscard]] bool empty() const {
    return ids_.empty();
  }

  [[nodiscard]] std::size_t size() const {
    return ids_.size();
  }

  [[nodiscard]] std::span<const StyleClassId> ids() const {
    return ids_;
  }

 private:
  std::vector<StyleClassId> ids_;
};

class Theme {
 public:
  Theme& set_color(ThemeTokenId id, Color color) {
    colors_[std::move(id.value)] = color;
    return *this;
  }

  [[nodiscard]] std::optional<Color> color(const ThemeTokenId& id) const {
    const auto entry = colors_.find(id.value);
    if (entry == colors_.end()) {
      return std::nullopt;
    }
    return entry->second;
  }

  Theme& set_spacing(ThemeTokenId id, float value) {
    spacings_[std::move(id.value)] = value;
    return *this;
  }

  [[nodiscard]] std::optional<float> spacing(const ThemeTokenId& id) const {
    const auto entry = spacings_.find(id.value);
    if (entry == spacings_.end()) {
      return std::nullopt;
    }
    return entry->second;
  }

 private:
  std::unordered_map<std::string, Color> colors_;
  std::unordered_map<std::string, float> spacings_;
};

} // namespace cgpui
