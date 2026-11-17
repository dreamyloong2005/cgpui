#pragma once

  PlatformWindowBackdropState apply_window_backdrop(WindowBackdrop backdrop, bool transparent_surface) override;
  [[nodiscard]] PlatformWindowBackdropState window_backdrop_state() const override;
  void attach_backdrop(wl_compositor* compositor, const WaylandBackdropGlobals* globals);
