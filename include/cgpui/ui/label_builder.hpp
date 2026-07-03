#pragma once

#include "cgpui/ui/element_text_nodes.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

class LabelBuilder {
 public:
  explicit LabelBuilder(std::string text);

  [[nodiscard]] LabelBuilder style(Style style) &&;
  [[nodiscard]] LabelBuilder foreground(Color color) &&;
  [[nodiscard]] LabelBuilder font(FontDescriptor descriptor) &&;
  [[nodiscard]] LabelBuilder font_size(float value) &&;
  [[nodiscard]] LabelBuilder key(ElementKey key) &&;
  [[nodiscard]] LabelBuilder key(std::string_view value) &&;
  [[nodiscard]] LabelBuilder enabled(bool value) &&;
  [[nodiscard]] LabelBuilder disabled() &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  std::string text_;
  Style style_;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
};

[[nodiscard]] LabelBuilder label(std::string_view text);

} // namespace cgpui
