#include "platform_popup_placement_internal.hpp"

#include <cmath>

namespace {

using cgpui::Rect;
using cgpui::WindowPopupSide;

bool near(float a, float b) { return std::fabs(a - b) < 0.5F; }

bool same(Rect rect, float x, float y, float width, float height) {
  return near(rect.origin.x, x) && near(rect.origin.y, y) &&
         near(rect.size.width, width) && near(rect.size.height, height);
}

} // namespace

// Placement in a 1000 x 800 work area (y-down), the table macOS used.
int main() {
  const Rect work{{0, 0}, {1000, 800}};
  const cgpui::Size menu{200, 300};
  const auto below = cgpui::platform_popup_frame({{100, 100}, {50, 20}}, menu,
                                                 WindowPopupSide::below, 4, work);
  if (!same(below.bounds, 100, 124, 200, 300) || below.flipped || below.shifted) return 1;
  // Near the bottom edge "below" flips above the anchor.
  const auto flipped = cgpui::platform_popup_frame({{100, 700}, {50, 20}}, menu,
                                                   WindowPopupSide::below, 4, work);
  if (!same(flipped.bounds, 100, 396, 200, 300) || !flipped.flipped) return 2;
  // Near the right edge "right" flips left; near the bottom it shifts up.
  const auto left = cgpui::platform_popup_frame({{900, 650}, {60, 20}}, menu,
                                                WindowPopupSide::right, 0, work);
  if (!same(left.bounds, 700, 500, 200, 300) || !left.flipped || !left.shifted) return 3;
  // Below at the right edge shifts left to stay on screen.
  const auto shifted = cgpui::platform_popup_frame({{900, 100}, {50, 20}}, menu,
                                                   WindowPopupSide::below, 0, work);
  if (!same(shifted.bounds, 800, 120, 200, 300) || !shifted.shifted || shifted.flipped) {
    return 4;
  }
  // "above" with room stays above; "left" aligns top edges.
  const auto above = cgpui::platform_popup_frame({{100, 500}, {50, 20}}, menu,
                                                 WindowPopupSide::above, 4, work);
  if (!same(above.bounds, 100, 196, 200, 300) || above.flipped || above.shifted) return 5;
  const auto to_left = cgpui::platform_popup_frame({{500, 100}, {50, 20}}, menu,
                                                   WindowPopupSide::left, 2, work);
  if (!same(to_left.bounds, 298, 100, 200, 300) || to_left.flipped) return 6;
  // No room on either side: keeps the preferred side, then shifts on screen.
  const auto tall = cgpui::platform_popup_frame({{100, 380}, {50, 40}}, {200, 500},
                                                WindowPopupSide::below, 0, work);
  if (!same(tall.bounds, 100, 300, 200, 500) || tall.flipped || !tall.shifted) return 7;
  // A work area off the origin (a second monitor) and a popup larger than it.
  const Rect second{{1000, -200}, {800, 600}};
  const auto offset = cgpui::platform_popup_frame({{1700, -190}, {50, 20}}, menu,
                                                  WindowPopupSide::below, 0, second);
  if (!same(offset.bounds, 1600, -170, 200, 300) || !offset.shifted) return 8;
  const auto huge = cgpui::platform_popup_frame({{1100, 0}, {50, 20}}, {900, 700},
                                                WindowPopupSide::right, 0, second);
  return same(huge.bounds, 1000, -200, 900, 700) && huge.shifted ? 0 : 9;
}
