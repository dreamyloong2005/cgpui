#pragma once

#include "cgpui/renderer/renderer.hpp"
#include "cgpui/renderer/renderer_backend.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace renderer_probe {
using Clock = std::chrono::steady_clock;
enum class SceneKind { rectangles, images, mixed };
struct Options {
  cgpui::RendererBackend backend = cgpui::RendererBackend::unknown;
  SceneKind scene = SceneKind::mixed;
  std::string profile;
  std::string source_label = "unverified";
  std::filesystem::path output;
  std::optional<std::uint64_t> expected_luid;
  std::optional<bool> expected_software;
  std::uint32_t warmup = 120, samples = 600;
  std::uint32_t frame_deadline_ms = 10000, run_deadline_ms = 180000;
};
struct Sample {
  std::uint32_t index = 0, attempts = 0;
  std::uint64_t acquire_wall_ns = 0, begin_api_wall_ns = 0, wait_wall_ns = 0;
  std::uint64_t record_wall_ns = 0, present_wall_ns = 0, frame_wall_ns = 0;
  cgpui::RendererFrameTimings stages;
  std::size_t commands = 0, draws = 0, batches = 0;
  std::size_t image_bytes = 0, glyph_bytes = 0;
};
struct Acquisition {
  std::unique_ptr<cgpui::RenderFrame> frame;
  Clock::time_point started, acquired;
  std::uint32_t attempts = 0;
  std::uint64_t api_wall_ns = 0, wait_wall_ns = 0;
};
struct Scene {
  explicit Scene(SceneKind kind);
  void replay(cgpui::RenderFrame& frame, bool upload) const;
  [[nodiscard]] bool verify(const cgpui::RendererFramePixels& pixels) const;
  [[nodiscard]] std::size_t rectangles() const;
  [[nodiscard]] std::size_t images() const;
  SceneKind kind;
  std::array<cgpui::ImageAsset, 32> assets;
};
struct DisplayFacts {
  std::string device;
  std::int32_t monitor_left = 0, monitor_top = 0, monitor_right = 0, monitor_bottom = 0;
  std::int32_t window_left = 0, window_top = 0;
  std::uint32_t width = 0, height = 0, hz = 0, bits_per_pixel = 0, orientation = 0, dpi = 0;
  std::uint64_t event_epoch = 0;
  bool valid = false;
  friend bool operator==(const DisplayFacts&, const DisplayFacts&) = default;
};
enum class CreationOutcome { factory_error, null_renderer, exception, created };
struct RendererCreationSample {
  std::uint64_t wall_ns = 0;
  CreationOutcome outcome = CreationOutcome::exception;
};
struct Report {
  std::string status = "failed", phase = "setup", error;
  cgpui::RendererBackendInfo actual;
  std::optional<cgpui::RendererBackendInfo> preflight_actual;
  std::vector<Sample> samples;
  std::optional<Sample> cold;
  // Only the measured renderer's factory call. No attempt means no observation.
  std::optional<RendererCreationSample> measured_creation;
  std::uint32_t warmup_completed = 0;
  bool setup_pixels_valid = false, final_pixels_valid = false;
  std::optional<DisplayFacts> display_before, display_after;
  int ac_line_status = -1;
  bool remote_session = false;
};
class Window;
[[nodiscard]] cgpui::Error failure(std::string message);
[[nodiscard]] std::uint64_t elapsed_ns(Clock::time_point a, Clock::time_point b);
[[nodiscard]] const char* backend_name(cgpui::RendererBackend backend);
[[nodiscard]] const char* scene_name(SceneKind kind);
[[nodiscard]] cgpui::Result<Options> parse_options(int argc, char** argv);
[[nodiscard]] cgpui::Result<Acquisition> acquire(
    cgpui::Renderer& renderer, const Window& window, Clock::time_point deadline);
[[nodiscard]] cgpui::Result<void> verify_scene(
    cgpui::Renderer& renderer, const Window& window, const Scene& scene,
    bool upload, Clock::time_point deadline);
[[nodiscard]] cgpui::Result<void> run(const Options& options, Report& report);
[[nodiscard]] cgpui::Result<void> write_report(const Options& options, const Report& report);
} // namespace renderer_probe
