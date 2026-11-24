#include "probe_window.hpp"

namespace renderer_probe {
cgpui::Result<void> verify_scene(cgpui::Renderer& renderer, const Window& window,
    const Scene& scene, bool upload, Clock::time_point deadline) {
  if (auto ready = window.pump(); !ready) return ready;
  auto acquired = acquire(renderer, window, deadline);
  if (!acquired) return std::unexpected(acquired.error());
  scene.replay(*acquired->frame, upload);
  if (auto capture = acquired->frame->request_pixel_capture(); !capture) return capture;
  if (auto presented = acquired->frame->present(); !presented) return presented;
  if (Clock::now() >= deadline) return std::unexpected(failure("verification returned after deadline"));
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || !scene.verify(*pixels)) return std::unexpected(failure("real captured scene pixels differ"));
  return window.pump(); // Postflight notification/facts check also lies outside measured frames.
}
} // namespace renderer_probe
