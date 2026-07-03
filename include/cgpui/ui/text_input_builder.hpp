#pragma once

#include "cgpui/ui/element_text_nodes.hpp"

#include <optional>
#include <string_view>

namespace cgpui {

class TextInputBuilder {
 public:
  explicit TextInputBuilder(TextModel& model);

  [[nodiscard]] TextInputBuilder style(Style style) &&;
  [[nodiscard]] TextInputBuilder foreground(Color color) &&;
  [[nodiscard]] TextInputBuilder font(FontDescriptor descriptor) &&;
  [[nodiscard]] TextInputBuilder font_size(float value) &&;
  [[nodiscard]] TextInputBuilder key(ElementKey key) &&;
  [[nodiscard]] TextInputBuilder key(std::string_view value) &&;
  [[nodiscard]] TextInputBuilder enabled(bool value) &&;
  [[nodiscard]] TextInputBuilder disabled() &&;
  [[nodiscard]] AnyElement build() &&;

 private:
  TextModel* model_ = nullptr;
  Style style_;
  std::optional<ElementKey> key_;
  bool enabled_ = true;
};

[[nodiscard]] TextInputBuilder text_input(TextModel& model);

} // namespace cgpui
