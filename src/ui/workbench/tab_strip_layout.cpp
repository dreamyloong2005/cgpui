#include "cgpui/ui/tab_strip.hpp"

#include "cgpui/ui/text_measurement.hpp"
#include "workbench_internal.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

TabStrip::TabStrip(TabStripOptions options) : options_(std::move(options)) {}

void TabStrip::set_tabs(std::vector<TabItem> tabs) {
  bool titles_changed = tabs.size() != tabs_.size();
  for (std::size_t index = 0; !titles_changed && index < tabs.size(); ++index) {
    titles_changed = tabs[index].title != tabs_[index].title;
  }
  tabs_ = std::move(tabs);
  measure_dirty_ = measure_dirty_ || titles_changed;
  if (selected_.has_value() && !index_of(*selected_).has_value()) {
    selected_.reset();
  }
  hovered_.reset();
  pressed_.reset();
  middle_pressed_.reset();
  dragging_ = false;
  place_tabs();
}

std::span<const TabItem> TabStrip::tabs() const {
  return tabs_;
}

void TabStrip::set_selected(std::optional<TabId> tab) {
  selected_ = tab;
  // Before the first layout there is no width to scroll against.
  reveal_pending_ = bounds_.size.width <= 0.0F;
  if (tab.has_value() && !reveal_pending_) {
    if (const auto index = index_of(*tab)) scroll_into_view(*index);
  }
}

std::optional<TabId> TabStrip::selected() const {
  return selected_;
}

std::optional<std::size_t> TabStrip::index_of(TabId tab) const {
  for (std::size_t index = 0; index < tabs_.size(); ++index) {
    if (tabs_[index].id == tab) return index;
  }
  return std::nullopt;
}

void TabStrip::on_event(TabStripEventHandler handler) {
  handler_ = std::move(handler);
}

void TabStrip::layout(Rect bounds, DpiScale scale, const WorkbenchTheme& theme) {
  bounds_ = bounds;
  scale_ = scale;
  const float factor = workbench_detail::scale_of(scale);
  if (measure_dirty_ || measured_font_size_ != theme.font_size ||
      measured_scale_ != factor || font_ != theme.font) {
    title_widths_.resize(tabs_.size());
    for (std::size_t index = 0; index < tabs_.size(); ++index) {
      title_widths_[index] =
          measure_text(tabs_[index].title, theme.font, theme.font_size, scale)
              .logical_size.width;
    }
    measured_font_size_ = theme.font_size;
    measured_scale_ = factor;
    font_ = theme.font;
    measure_dirty_ = false;
  }
  place_tabs();
  if (reveal_pending_ && selected_.has_value() && bounds.size.width > 0.0F) {
    reveal_pending_ = false;
    if (const auto index = index_of(*selected_)) scroll_into_view(*index);
  }
}

void TabStrip::place_tabs() {
  tab_x_.resize(tabs_.size());
  tab_widths_.resize(tabs_.size());
  title_widths_.resize(tabs_.size(), 0.0F);
  float x = 0.0F;
  for (std::size_t index = 0; index < tabs_.size(); ++index) {
    const float natural = options_.padding + title_widths_[index] + 8.0F +
                          options_.close_size + 8.0F;
    const float width = workbench_detail::snap(
        std::clamp(natural, options_.min_tab_width, options_.max_tab_width),
        scale_);
    tab_x_[index] = x;
    tab_widths_[index] = width;
    x += width;
  }
  content_width_ = x;
  scroll_ = clamp_scroll(scroll_);
}

Rect TabStrip::bounds() const {
  return bounds_;
}

Rect TabStrip::tab_rect(std::size_t index) const {
  if (index >= tab_x_.size()) return {};
  const float x = workbench_detail::snap(
      bounds_.origin.x + tab_x_[index] - scroll_, scale_);
  return Rect{{x, bounds_.origin.y}, {tab_widths_[index], bounds_.size.height}};
}

Rect TabStrip::close_rect(std::size_t index) const {
  const Rect tab = tab_rect(index);
  const float size = options_.close_size;
  return Rect{{workbench_detail::right(tab) - 8.0F - size,
               tab.origin.y + (tab.size.height - size) / 2.0F},
              {size, size}};
}

float TabStrip::title_width(std::size_t index) const {
  return index < title_widths_.size() ? title_widths_[index] : 0.0F;
}

float TabStrip::scroll_offset() const {
  return scroll_;
}

float TabStrip::content_width() const {
  return content_width_;
}

float TabStrip::clamp_scroll(float offset) const {
  return std::clamp(offset, 0.0F,
                    std::max(0.0F, content_width_ - bounds_.size.width));
}

void TabStrip::scroll_by(float delta) {
  scroll_ = clamp_scroll(scroll_ + delta);
}

void TabStrip::scroll_into_view(std::size_t index) {
  if (index >= tab_x_.size()) return;
  const float start = tab_x_[index];
  const float end = start + tab_widths_[index];
  if (start < scroll_) {
    scroll_ = start;
  } else if (end > scroll_ + bounds_.size.width) {
    scroll_ = end - bounds_.size.width;
  }
  scroll_ = clamp_scroll(scroll_);
}

std::optional<std::size_t> TabStrip::tab_at(Point point) const {
  if (!contains(bounds_, point)) return std::nullopt;
  for (std::size_t index = 0; index < tabs_.size(); ++index) {
    if (contains(tab_rect(index), point)) return index;
  }
  return std::nullopt;
}

bool TabStrip::dragging() const {
  return dragging_;
}

std::optional<std::size_t> TabStrip::drop_index() const {
  if (!dragging_) return std::nullopt;
  return drop_index_for(drag_x_);
}

// Index the dragged tab would take if dropped at window x `x`: the number
// of other tabs whose centers lie left of the pointer.
std::size_t TabStrip::drop_index_for(float x) const {
  if (!pressed_.has_value()) return 0;
  std::size_t index = 0;
  for (std::size_t other = 0; other < tabs_.size(); ++other) {
    if (other == *pressed_) continue;
    const Rect rect = tab_rect(other);
    if (rect.origin.x + rect.size.width / 2.0F < x) index += 1;
  }
  return index;
}

bool TabStrip::focused() const {
  return focused_;
}

ElementId TabStrip::element_id() const {
  return strip_id_;
}

} // namespace cgpui
