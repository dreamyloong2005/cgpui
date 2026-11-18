#include "cgpui/ui/splitter.hpp"

#include "workbench_internal.hpp"

#include <algorithm>
#include <numeric>
#include <utility>

namespace cgpui {

Splitter::Splitter(SplitterOptions options, std::vector<SplitterPaneOptions> panes)
    : options_(std::move(options)), panes_(std::move(panes)) {
  if (panes_.empty()) panes_.push_back(SplitterPaneOptions{});
  const std::size_t count = panes_.size();
  fractions_.resize(count);
  collapsed_.assign(count, false);
  sizes_.assign(count, 0.0F);
  pane_rects_.assign(count, Rect{});
  divider_rects_.assign(count - 1, Rect{});
  divider_ids_.assign(count - 1, ElementId{});
  restore_default_fractions();
}

const SplitterOptions& Splitter::options() const {
  return options_;
}

std::size_t Splitter::pane_count() const {
  return panes_.size();
}

std::span<const float> Splitter::fractions() const {
  return fractions_;
}

void Splitter::set_fractions(std::span<const float> fractions) {
  if (fractions.size() != fractions_.size()) return;
  float total = 0.0F;
  for (const float value : fractions) total += std::max(0.0F, value);
  if (total <= 0.0F) return;
  for (std::size_t index = 0; index < fractions.size(); ++index) {
    fractions_[index] = std::max(0.0F, fractions[index]) / total;
  }
  layout(bounds_, scale_);
}

void Splitter::restore_default_fractions() {
  float total = 0.0F;
  for (const auto& pane : panes_) total += std::max(0.0F, pane.default_fraction);
  for (std::size_t index = 0; index < panes_.size(); ++index) {
    const float value = std::max(0.0F, panes_[index].default_fraction);
    fractions_[index] = total > 0.0F ? value / total : 1.0F / panes_.size();
  }
  collapsed_.assign(panes_.size(), false);
  layout(bounds_, scale_);
}

bool Splitter::collapsed(std::size_t pane) const {
  return pane < collapsed_.size() && collapsed_[pane];
}

void Splitter::set_collapsed(std::size_t pane, bool value) {
  if (pane >= collapsed_.size() || !panes_[pane].collapsible) return;
  collapsed_[pane] = value;
  layout(bounds_, scale_);
}

void Splitter::on_change(SplitterChangeHandler handler) {
  change_handler_ = std::move(handler);
}

float Splitter::main_extent() const {
  return options_.axis == SplitterAxis::horizontal ? bounds_.size.width
                                                   : bounds_.size.height;
}

float Splitter::available() const {
  const float dividers = workbench_detail::snap_length(
      options_.divider_thickness, scale_) * static_cast<float>(panes_.size() - 1);
  return std::max(0.0F, main_extent() - dividers);
}

float Splitter::main_of(Point point) const {
  return options_.axis == SplitterAxis::horizontal ? point.x : point.y;
}

void Splitter::layout(Rect bounds, DpiScale scale) {
  bounds_ = bounds;
  scale_ = scale;
  compute_sizes();
  using workbench_detail::snap;
  const bool horizontal = options_.axis == SplitterAxis::horizontal;
  const float start = horizontal ? bounds.origin.x : bounds.origin.y;
  const float end = start + main_extent();
  const float thickness =
      workbench_detail::snap_length(options_.divider_thickness, scale);
  float cursor = start;
  for (std::size_t index = 0; index < panes_.size(); ++index) {
    const float pane_start = snap(cursor, scale);
    const bool last = index + 1 == panes_.size();
    const float pane_end = last ? end : snap(cursor + sizes_[index], scale);
    const float extent = std::max(0.0F, pane_end - pane_start);
    pane_rects_[index] = horizontal
        ? Rect{{pane_start, bounds.origin.y}, {extent, bounds.size.height}}
        : Rect{{bounds.origin.x, pane_start}, {bounds.size.width, extent}};
    if (!last) {
      divider_rects_[index] = horizontal
          ? Rect{{pane_end, bounds.origin.y}, {thickness, bounds.size.height}}
          : Rect{{bounds.origin.x, pane_end}, {bounds.size.width, thickness}};
    }
    cursor = pane_end + thickness;
  }
}

// Distributes the available extent by fraction over expanded panes, then
// clamps to min / max and hands the remainder to the unclamped panes.
void Splitter::compute_sizes() {
  const std::size_t count = panes_.size();
  const float total = available();
  std::vector<bool>& fixed = scratch_fixed_;
  fixed.assign(count, false);
  for (std::size_t index = 0; index < count; ++index) {
    sizes_[index] = 0.0F;
    fixed[index] = collapsed_[index];
  }
  for (std::size_t round = 0; round <= count; ++round) {
    float remaining = total;
    float weight = 0.0F;
    for (std::size_t index = 0; index < count; ++index) {
      if (fixed[index]) {
        remaining -= sizes_[index];
      } else {
        weight += fractions_[index];
      }
    }
    bool clamped = false;
    for (std::size_t index = 0; index < count; ++index) {
      if (fixed[index]) continue;
      const float share = weight > 0.0F
          ? remaining * fractions_[index] / weight
          : remaining / static_cast<float>(count);
      const auto& pane = panes_[index];
      sizes_[index] = std::clamp(share, pane.min_size,
                                 std::max(pane.min_size, pane.max_size));
      if (sizes_[index] != share) {
        fixed[index] = true;
        clamped = true;
      }
    }
    if (!clamped) break;
  }
}

Rect Splitter::bounds() const {
  return bounds_;
}

Rect Splitter::pane_rect(std::size_t pane) const {
  return pane < pane_rects_.size() ? pane_rects_[pane] : Rect{};
}

Rect Splitter::divider_rect(std::size_t divider) const {
  return divider < divider_rects_.size() ? divider_rects_[divider] : Rect{};
}

// The pointer target is centered on the visible line and wider than it.
Rect Splitter::divider_hit_rect(std::size_t divider) const {
  const Rect line = divider_rect(divider);
  const float hit = std::max(options_.hit_thickness, line.size.width);
  if (options_.axis == SplitterAxis::horizontal) {
    const float grow = std::max(0.0F, hit - line.size.width) / 2.0F;
    return Rect{{line.origin.x - grow, line.origin.y},
                {line.size.width + grow * 2.0F, line.size.height}};
  }
  const float grow =
      std::max(0.0F, std::max(options_.hit_thickness, line.size.height) -
                         line.size.height) / 2.0F;
  return Rect{{line.origin.x, line.origin.y - grow},
              {line.size.width, line.size.height + grow * 2.0F}};
}

std::optional<std::size_t> Splitter::divider_at(Point point) const {
  for (std::size_t index = 0; index < divider_rects_.size(); ++index) {
    if (contains(divider_hit_rect(index), point)) return index;
  }
  return std::nullopt;
}

std::optional<std::size_t> Splitter::dragging_divider() const {
  return dragging_;
}

std::optional<std::size_t> Splitter::focused_divider() const {
  return focused_;
}

ElementId Splitter::divider_element_id(std::size_t divider) const {
  return divider < divider_ids_.size() ? divider_ids_[divider] : ElementId{};
}

} // namespace cgpui
