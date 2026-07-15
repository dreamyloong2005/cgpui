#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

namespace cgpui {

bool macos_scroll_phase_active(std::uint64_t phase) {
  const auto value = static_cast<NSEventPhase>(phase);
  return (value & (NSEventPhaseBegan | NSEventPhaseChanged |
                   NSEventPhaseMayBegin)) != 0;
}

PointerScrolled macos_translate_scroll_event(
    double dx,
    double dy,
    bool precise,
    std::uint64_t phase,
    std::uint64_t momentum,
    float scale) {
  (void)macos_scroll_phase_active(phase);
  (void)macos_scroll_phase_active(momentum);
  const double normalized_scale = scale > 0.0F ? scale : 1.0;
  return PointerScrolled{
      .delta = Point{
          static_cast<float>((dx * normalized_scale) / normalized_scale),
          static_cast<float>((dy * normalized_scale) / normalized_scale)},
      .precise = precise,
  };
}

void MacOSWindow::pointer_scrolled(NSEvent* event) {
  PointerScrolled translated = macos_translate_scroll_event(
      [event scrollingDeltaX],
      [event scrollingDeltaY],
      [event hasPreciseScrollingDeltas],
      [event phase],
      [event momentumPhase],
      state_.scale.value);
  translated.position = event_position(event);
  callback_(translated);
}

}  // namespace cgpui
