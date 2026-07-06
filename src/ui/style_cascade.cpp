#include "cgpui/ui/style_cascade.hpp"

#include "style_cascade_overlays.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] bool is_visiting(
    const std::vector<std::string>& visiting,
    const cgpui::StyleClassId& id) {
  return std::find(visiting.begin(), visiting.end(), id.value) !=
         visiting.end();
}

[[nodiscard]] cgpui::Style apply_class_rule(
    const cgpui::StyleCascade& cascade,
    cgpui::Style style,
    const cgpui::StyleClassId& id,
    cgpui::StyleStateFlags flags,
    std::vector<std::string>& visiting) {
  if (is_visiting(visiting, id)) {
    return style;
  }

  const cgpui::StyleClassRule* rule = cascade.class_rule(id);
  if (rule == nullptr) {
    return style;
  }

  visiting.push_back(id.value);
  for (const cgpui::StyleClassId& reused_id :
       rule->reused_classes.ids()) {
    style = apply_class_rule(cascade, style, reused_id, flags, visiting);
  }
  visiting.pop_back();

  return cgpui::style_cascade_detail::apply_style_state(
      style,
      rule->style,
      flags);
}

} // namespace

namespace cgpui {

StyleCascade& StyleCascade::set_class_style(
    StyleClassId id,
    StyleState style) {
  StyleClassRule rule;
  rule.style = std::move(style);
  return set_class_rule(std::move(id), std::move(rule));
}

StyleCascade& StyleCascade::set_class_rule(
    StyleClassId id,
    StyleClassRule rule) {
  class_styles_[std::move(id.value)] = std::move(rule);
  return *this;
}

const StyleState* StyleCascade::class_style(const StyleClassId& id) const {
  const StyleClassRule* rule = class_rule(id);
  return rule == nullptr ? nullptr : &rule->style;
}

const StyleClassRule* StyleCascade::class_rule(
    const StyleClassId& id) const {
  const auto entry = class_styles_.find(id.value);
  if (entry == class_styles_.end()) {
    return nullptr;
  }
  return &entry->second;
}

Style resolved_style(
    const StyleCascade& cascade,
    const StyleState& local,
    const StyleClasses& classes,
    const StyleOverlay& inline_style,
    StyleStateFlags flags) {
  Style style = local.base;
  std::vector<std::string> visiting;
  for (const StyleClassId& class_id : classes.ids()) {
    style = apply_class_rule(cascade, style, class_id, flags, visiting);
  }

  style = resolved_style(StyleState{
                             .base = style,
                             .hover = local.hover,
                             .focus = local.focus,
                             .active = local.active,
                             .disabled = local.disabled,
                         },
                         flags);
  return apply_style_overlay(style, inline_style);
}

} // namespace cgpui
