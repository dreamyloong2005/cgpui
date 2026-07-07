#pragma once

#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/ui/element_core.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

enum class ImageElementKind {
  image,
  icon,
};

class ImageElement : public Element {
 public:
  ImageElement(
      ImageElementKind kind,
      ImageAssetDescriptor asset,
      StyleState style_state,
      std::optional<Rect> source_rect = std::nullopt,
      std::optional<Color> tint = std::nullopt,
      std::string alt = {});

  [[nodiscard]] ImageElementKind kind() const;
  [[nodiscard]] const ImageAssetDescriptor& asset() const;
  [[nodiscard]] const StyleState& style_state() const;
  [[nodiscard]] const std::optional<Rect>& source_rect() const;
  [[nodiscard]] const std::optional<Color>& tint() const;
  [[nodiscard]] std::string_view alt() const;
  [[nodiscard]] AccessibilityRole accessibility_role() const override;
  [[nodiscard]] std::string accessibility_name() const override;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;
  void paint(PaintList& paint_list) const override;
  [[nodiscard]] int z_index() const override;
  [[nodiscard]] int layer() const override;

 private:
  [[nodiscard]] Size intrinsic_size() const;
  [[nodiscard]] Rect content_rect() const;

  ImageElementKind kind_;
  ImageAssetDescriptor asset_;
  StyleState style_state_;
  std::optional<Rect> source_rect_;
  std::optional<Color> tint_;
  std::string alt_;
};

} // namespace cgpui
