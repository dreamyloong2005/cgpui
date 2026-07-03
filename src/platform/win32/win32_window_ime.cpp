#include "win32_window_internal.hpp"

namespace cgpui {

void Win32Window::ime_start_composition() {
  apply_ime_text_input_placement();
}

void Win32Window::apply_ime_text_input_placement() {
  if (hwnd_ == nullptr || !state_.ime_text_input_placement.has_value()) {
    return;
  }

  const ImeTextInputPlacement& placement =
      *state_.ime_text_input_placement;
  const POINT point{
      .x = static_cast<LONG>(placement.rect.origin.x),
      .y = static_cast<LONG>(placement.rect.origin.y),
  };
  const auto height = static_cast<LONG>(placement.rect.size.height);
  const RECT area{
      .left = point.x,
      .top = point.y,
      .right = point.x + static_cast<LONG>(placement.rect.size.width),
      .bottom = point.y + height,
  };

  HIMC context = ImmGetContext(hwnd_);
  if (context == nullptr) {
    return;
  }

  COMPOSITIONFORM composition{};
  composition.dwStyle = CFS_POINT;
  composition.ptCurrentPos = point;
  ImmSetCompositionWindow(context, &composition);

  CANDIDATEFORM candidate{};
  candidate.dwIndex = 0;
  candidate.dwStyle = CFS_EXCLUDE;
  candidate.ptCurrentPos = point;
  candidate.rcArea = area;
  ImmSetCandidateWindow(context, &candidate);

  ImmReleaseContext(hwnd_, context);
}

} // namespace cgpui
