#pragma once

  void set_window_control_regions(
      std::vector<WindowControlRegion> regions) override;
  [[nodiscard]] PlatformWindowFrameMetrics frame_metrics() const override;
  PlatformWindowActionResult perform_window_action(
      PlatformWindowActionRequest request) override;
  void title_bar_pointer_button(
      wl_seat* seat, std::uint32_t serial, std::uint32_t time,
      MouseButton button, bool pressed, Point position);
  [[nodiscard]] bool client_title_bar_active() const;
