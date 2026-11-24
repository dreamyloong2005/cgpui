#include "probe_window.hpp"

namespace renderer_probe {
cgpui::Result<Acquisition> acquire(
    cgpui::Renderer& renderer, const Window& window, Clock::time_point deadline) {
  Acquisition result;
  result.started = Clock::now();
  for (;;) {
    const auto started = Clock::now();
    if (started >= deadline) return std::unexpected(failure("bounded begin deadline expired"));
    auto frame = renderer.begin_frame();
    const auto stopped = Clock::now();
    result.api_wall_ns += elapsed_ns(started, stopped);
    ++result.attempts;
    if (stopped >= deadline) return std::unexpected(failure("begin returned after deadline"));
    if (frame) {
      if (!*frame) return std::unexpected(failure("begin returned a null frame"));
      result.frame = std::move(*frame);
      result.acquired = stopped;
      return result;
    }
    if (frame.error().code != cgpui::ErrorCode::frame_not_ready)
      return std::unexpected(frame.error()); // Recovery invalidates a steady-state series.
    const auto wait_started = Clock::now();
    if (auto waited = window.wait(deadline); !waited) return std::unexpected(waited.error());
    result.wait_wall_ns += elapsed_ns(wait_started, Clock::now());
  }
}
} // namespace renderer_probe
