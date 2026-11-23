#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace cgpui_test {
bool composition_probe_dispatch();
class Dx12CompositionProbeWindows {
 public:
  Dx12CompositionProbeWindows();
  ~Dx12CompositionProbeWindows();
  Dx12CompositionProbeWindows(const Dx12CompositionProbeWindows&) = delete;
  Dx12CompositionProbeWindows& operator=(const Dx12CompositionProbeWindows&) = delete;
  bool open(bool visible);
  bool visibility_unchanged() const;
  bool resize(UINT extent);
  bool expect_screen(int r, int g, int b, int x, int y) const;
  bool expect_backdrop() const;
  HWND popup() const;
 private:
  HWND backdrop_ = nullptr, popup_ = nullptr;
  HBRUSH brush_ = nullptr;
  ATOM backdrop_class_ = 0, popup_class_ = 0;
  POINT origin_{};
  bool visible_ = false;
};
} // namespace cgpui_test
