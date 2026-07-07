#pragma once

#include "cgpui/ui/element_nodes.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {

class ElementBuilder {
 public:
  [[nodiscard]] static ElementBuilder box();
  [[nodiscard]] static ElementBuilder row();
  [[nodiscard]] static ElementBuilder column();
  [[nodiscard]] static ElementBuilder v_stack();
  [[nodiscard]] static ElementBuilder fixed_size(Size size);
  [[nodiscard]] static ElementBuilder text(TextModel& model);
  [[nodiscard]] static ElementBuilder child_view(ViewId view_id);

  [[nodiscard]] ElementBuilder style(Style style) &&;
  [[nodiscard]] ElementBuilder hover_style(StyleOverlay overlay) &&;
  [[nodiscard]] ElementBuilder focus_style(StyleOverlay overlay) &&;
  [[nodiscard]] ElementBuilder active_style(StyleOverlay overlay) &&;
  [[nodiscard]] ElementBuilder disabled_style(StyleOverlay overlay) &&;
  [[nodiscard]] ElementBuilder class_name(StyleClassId id) &&;
  [[nodiscard]] ElementBuilder class_name(std::string_view value) &&;
  [[nodiscard]] ElementBuilder inline_style(StyleOverlay style) &&;
  [[nodiscard]] ElementBuilder key(ElementKey key) &&;
  [[nodiscard]] ElementBuilder key(std::string_view value) &&;
  [[nodiscard]] ElementBuilder size(Size size) &&;
  [[nodiscard]] ElementBuilder size(float width, float height) &&;
  [[nodiscard]] ElementBuilder w(float width) &&;
  [[nodiscard]] ElementBuilder h(float height) &&;
  [[nodiscard]] ElementBuilder min_size(Size size) &&;
  [[nodiscard]] ElementBuilder max_size(Size size) &&;
  [[nodiscard]] ElementBuilder min_w(float width) &&;
  [[nodiscard]] ElementBuilder min_h(float height) &&;
  [[nodiscard]] ElementBuilder max_w(float width) &&;
  [[nodiscard]] ElementBuilder max_h(float height) &&;
  [[nodiscard]] ElementBuilder size_pct(
      float width_percent,
      float height_percent) &&;
  [[nodiscard]] ElementBuilder w_pct(float percent) &&;
  [[nodiscard]] ElementBuilder h_pct(float percent) &&;
  [[nodiscard]] ElementBuilder padding(EdgeSizes edges) &&;
  [[nodiscard]] ElementBuilder p(float value) &&;
  [[nodiscard]] ElementBuilder px(float value) &&;
  [[nodiscard]] ElementBuilder py(float value) &&;
  [[nodiscard]] ElementBuilder pt(float value) &&;
  [[nodiscard]] ElementBuilder pr(float value) &&;
  [[nodiscard]] ElementBuilder pb(float value) &&;
  [[nodiscard]] ElementBuilder pl(float value) &&;
  [[nodiscard]] ElementBuilder margin(EdgeSizes edges) &&;
  [[nodiscard]] ElementBuilder m(float value) &&;
  [[nodiscard]] ElementBuilder mx(float value) &&;
  [[nodiscard]] ElementBuilder my(float value) &&;
  [[nodiscard]] ElementBuilder mt(float value) &&;
  [[nodiscard]] ElementBuilder mr(float value) &&;
  [[nodiscard]] ElementBuilder mb(float value) &&;
  [[nodiscard]] ElementBuilder ml(float value) &&;
  [[nodiscard]] ElementBuilder background(Color color) &&;
  [[nodiscard]] ElementBuilder bg(Color color) &&;
  [[nodiscard]] ElementBuilder foreground(Color color) &&;
  [[nodiscard]] ElementBuilder text_color(Color color) &&;
  [[nodiscard]] ElementBuilder overflow(Overflow value) &&;
  [[nodiscard]] ElementBuilder overflow_hidden() &&;
  [[nodiscard]] ElementBuilder overflow_visible() &&;
  [[nodiscard]] ElementBuilder font(FontDescriptor descriptor) &&;
  [[nodiscard]] ElementBuilder font_family(std::string_view value) &&;
  [[nodiscard]] ElementBuilder font_size(float value) &&;
  [[nodiscard]] ElementBuilder text_size(float value) &&;
  [[nodiscard]] ElementBuilder border_width(EdgeSizes edges) &&;
  [[nodiscard]] ElementBuilder border_1() &&;
  [[nodiscard]] ElementBuilder border_color(Color color) &&;
  [[nodiscard]] ElementBuilder border_radius(BorderRadii radius) &&;
  [[nodiscard]] ElementBuilder rounded(float radius) &&;
  [[nodiscard]] ElementBuilder shadow(BoxShadow shadow) &&;
  [[nodiscard]] ElementBuilder shadow_sm() &&;
  [[nodiscard]] ElementBuilder opacity(float value) &&;
  [[nodiscard]] ElementBuilder gap(float value) &&;
  [[nodiscard]] ElementBuilder align_items(AlignItems value) &&;
  [[nodiscard]] ElementBuilder items_start() &&;
  [[nodiscard]] ElementBuilder items_center() &&;
  [[nodiscard]] ElementBuilder items_end() &&;
  [[nodiscard]] ElementBuilder justify_content(JustifyContent value) &&;
  [[nodiscard]] ElementBuilder justify_start() &&;
  [[nodiscard]] ElementBuilder justify_center() &&;
  [[nodiscard]] ElementBuilder justify_end() &&;
  [[nodiscard]] ElementBuilder justify_between() &&;
  [[nodiscard]] ElementBuilder flex_grow(float value) &&;
  [[nodiscard]] ElementBuilder flex_shrink(float value) &&;
  [[nodiscard]] ElementBuilder flex_1() &&;
  [[nodiscard]] ElementBuilder layer(int value) &&;
  [[nodiscard]] ElementBuilder z_index(int value) &&;
  [[nodiscard]] ElementBuilder position(Position value) &&;
  [[nodiscard]] ElementBuilder absolute() &&;
  [[nodiscard]] ElementBuilder fixed() &&;
  [[nodiscard]] ElementBuilder relative() &&;
  [[nodiscard]] ElementBuilder inset(EdgeSizes edges) &&;
  [[nodiscard]] ElementBuilder top(float value) &&;
  [[nodiscard]] ElementBuilder right(float value) &&;
  [[nodiscard]] ElementBuilder bottom(float value) &&;
  [[nodiscard]] ElementBuilder left(float value) &&;
  [[nodiscard]] ElementBuilder enabled(bool value) &&;
  [[nodiscard]] ElementBuilder disabled() &&;
  [[nodiscard]] ElementBuilder focusable() &&;
  [[nodiscard]] ElementBuilder tab_index(int value) &&;
  [[nodiscard]] ElementBuilder focus_ring(FocusRingVisibility visibility) &&;
  [[nodiscard]] ElementBuilder on_click(ClickHandler handler) &&;
  [[nodiscard]] ElementBuilder on_pointer_down(
      PointerButtonHandler handler) &&;
  [[nodiscard]] ElementBuilder on_pointer_up(PointerButtonHandler handler) &&;
  [[nodiscard]] ElementBuilder on_pointer_move(PointerMoveHandler handler) &&;
  [[nodiscard]] ElementBuilder on_key(KeyHandler handler) &&;
  [[nodiscard]] ElementBuilder child(std::unique_ptr<Element> child) &&;
  [[nodiscard]] ElementBuilder child(ElementBuilder child) &&;
  [[nodiscard]] ElementBuilder children(std::vector<AnyElement> children) &&;

  template <typename T>
    requires std::derived_from<T, Element> && (!std::same_as<T, Element>)
  [[nodiscard]] ElementBuilder child(std::unique_ptr<T> child) && {
    return std::move(*this).child(std::unique_ptr<Element>(std::move(child)));
  }

  [[nodiscard]] std::unique_ptr<Element> build() &&;

 private:
  enum class Kind {
    box,
    row,
    column,
    v_stack,
    fixed_size,
    text,
    child_view,
  };

  explicit ElementBuilder(Kind kind);
  void apply_element_attributes(Element& element) const;
  [[nodiscard]] std::unique_ptr<Element> finish(
      std::unique_ptr<Element> element) const;

  Kind kind_ = Kind::box;
  StyleState style_state_;
  StyleClasses style_classes_;
  StyleOverlay inline_style_;
  std::optional<ElementKey> key_;
  Size size_;
  TextModel* text_model_ = nullptr;
  ViewId child_view_id_;
  FocusMetadata focus_metadata_;
  bool enabled_ = true;
  bool focusable_ = false;
  ClickHandler click_handler_;
  PointerButtonHandler pointer_down_handler_;
  PointerButtonHandler pointer_up_handler_;
  PointerMoveHandler pointer_move_handler_;
  KeyHandler key_handler_;
  std::vector<std::unique_ptr<Element>> children_;
};

[[nodiscard]] AnyElement into_element(AnyElement element);
[[nodiscard]] AnyElement into_element(ElementBuilder builder);
[[nodiscard]] ElementBuilder div();
[[nodiscard]] ElementBuilder h_flex();
[[nodiscard]] ElementBuilder v_flex();
[[nodiscard]] ElementBuilder v_stack();
[[nodiscard]] ElementBuilder text(TextModel& model);
[[nodiscard]] ElementBuilder child_view(ViewId view_id);

} // namespace cgpui
