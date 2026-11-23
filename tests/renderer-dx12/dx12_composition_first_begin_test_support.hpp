#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "cgpui/renderer/renderer_types.hpp"

namespace cgpui_test {
// Construction never shows either HWND. Only the explicit visible case may show.
class Dx12CompositionFirstBeginWindows {
 public:
  Dx12CompositionFirstBeginWindows();
  ~Dx12CompositionFirstBeginWindows();
  Dx12CompositionFirstBeginWindows(const Dx12CompositionFirstBeginWindows&) = delete;
  Dx12CompositionFirstBeginWindows& operator=(const Dx12CompositionFirstBeginWindows&) = delete;
  bool valid() const;
  bool matches(bool visible) const;
  bool show_explicitly();
  cgpui::RenderSurfaceDescriptor descriptor() const;

 private:
  HWND owner_ = nullptr;
  HWND popup_ = nullptr;
  HWND active_before_ = nullptr;
};

int composition_first_begin_case(bool visible);
} // namespace cgpui_test
