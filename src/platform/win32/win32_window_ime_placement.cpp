#include "win32_window_internal.hpp"

namespace cgpui {

void Win32Window::apply_ime_text_input_placement() {
  if (hwnd_ == nullptr || !state_.ime_text_input_placement.has_value()) {
    return;
  }

  const ImeTextInputPlacement& placement = *state_.ime_text_input_placement;
  const Rect& candidate_rect = placement.candidate_rect.has_value()
      ? *placement.candidate_rect
      : placement.rect;
  const POINT composition_point{
      .x = static_cast<LONG>(placement.rect.origin.x),
      .y = static_cast<LONG>(placement.rect.origin.y),
  };
  const POINT candidate_point{
      .x = static_cast<LONG>(candidate_rect.origin.x),
      .y = static_cast<LONG>(candidate_rect.origin.y),
  };
  const auto candidate_height = static_cast<LONG>(candidate_rect.size.height);
  const RECT candidate_area{
      .left = candidate_point.x,
      .top = candidate_point.y,
      .right = candidate_point.x + static_cast<LONG>(candidate_rect.size.width),
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
