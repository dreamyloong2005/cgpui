#include "win32_window_internal.hpp"

#include <cmath>

namespace cgpui {

void Win32Window::apply_ime_text_input_placement() {
  if (hwnd_ == nullptr || !state_.ime_text_input_placement.has_value()) {
    return;
  }

  const ImeTextInputPlacement& placement = *state_.ime_text_input_placement;
  const Rect& candidate_rect = placement.candidate_rect.has_value()
      ? *placement.candidate_rect
      : placement.rect;
  // Placement rects are logical pixels; IMM32 wants physical client pixels.
  const float scale = state_.scale.value > 0.0F ? state_.scale.value : 1.0F;
  const auto physical = [scale](float logical) {
    return static_cast<LONG>(std::lround(logical * scale));
  };
  const POINT composition_point{
      .x = physical(placement.rect.origin.x),
      .y = physical(placement.rect.origin.y),
  };
  const POINT candidate_point{
      .x = physical(candidate_rect.origin.x),
      .y = physical(candidate_rect.origin.y),
  };
  const LONG candidate_height = physical(candidate_rect.size.height);
  const RECT candidate_area{
      .left = candidate_point.x,
      .top = candidate_point.y,
      .right = candidate_point.x + physical(candidate_rect.size.width),
      .bottom = candidate_point.y + candidate_height,
  };

  HIMC context = ImmGetContext(hwnd_);
  if (context == nullptr) {
    return;
  }

  COMPOSITIONFORM composition{};
  composition.dwStyle = CFS_POINT;
  composition.ptCurrentPos = composition_point;
  ImmSetCompositionWindow(context, &composition);

  CANDIDATEFORM candidate{};
  candidate.dwIndex = 0;
  candidate.dwStyle = CFS_EXCLUDE;
  candidate.ptCurrentPos = candidate_point;
  candidate.rcArea = candidate_area;
  ImmSetCandidateWindow(context, &candidate);

  ImmReleaseContext(hwnd_, context);
}

} // namespace cgpui
