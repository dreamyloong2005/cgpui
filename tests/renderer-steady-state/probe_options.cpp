#include "probe_internal.hpp"
#include <charconv>
#include <limits>
#include <string_view>

namespace renderer_probe {
cgpui::Error failure(std::string message) {
  return {cgpui::ErrorCode::invalid_argument, std::move(message)};
}
std::uint64_t elapsed_ns(Clock::time_point a, Clock::time_point b) {
  const auto value = std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
  return value > 0 ? static_cast<std::uint64_t>(value) : 0;
}
const char* backend_name(cgpui::RendererBackend backend) {
  if (backend == cgpui::RendererBackend::dx12) return "dx12";
  if (backend == cgpui::RendererBackend::vulkan) return "vulkan";
  return "unknown";
}
const char* scene_name(SceneKind kind) {
  if (kind == SceneKind::rectangles) return "rect-grid";
  if (kind == SceneKind::images) return "cached-images";
  return "mixed";
}
namespace {
std::optional<std::uint64_t> integer(std::string_view value) {
  std::uint64_t result = 0;
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
  if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size()) return {};
  return result;
}
} // namespace
cgpui::Result<Options> parse_options(int argc, char** argv) {
  Options options;
  for (int index = 1; index < argc; ++index) {
    const std::string_view key = argv[index];
    if (index + 1 == argc) return std::unexpected(failure("missing value for " + std::string(key)));
    const std::string_view value = argv[++index];
    if (key == "--backend") {
      if (value == "dx12") options.backend = cgpui::RendererBackend::dx12;
      else if (value == "vulkan") options.backend = cgpui::RendererBackend::vulkan;
      else return std::unexpected(failure("--backend must be dx12 or vulkan"));
    } else if (key == "--scene") {
      if (value == "rect-grid") options.scene = SceneKind::rectangles;
      else if (value == "cached-images") options.scene = SceneKind::images;
      else if (value == "mixed") options.scene = SceneKind::mixed;
      else return std::unexpected(failure("unknown scene"));
    } else if (key == "--profile") options.profile = value;
    else if (key == "--source-label") options.source_label = value;
    else if (key == "--output") options.output = std::filesystem::u8path(value);
    else if (key == "--adapter-kind") {
      if (value == "hardware") options.expected_software = false;
      else if (value == "software") options.expected_software = true;
      else return std::unexpected(failure("--adapter-kind must be hardware or software"));
    } else if (key == "--expected-luid") {
      options.expected_luid = integer(value);
      if (!options.expected_luid) return std::unexpected(failure("LUID must be an unsigned decimal integer"));
    } else if (key == "--warmup" || key == "--samples" ||
               key == "--frame-deadline-ms" || key == "--run-deadline-ms") {
      const auto number = integer(value);
      const std::uint64_t maximum = key == "--run-deadline-ms" ? 3600000 :
          key == "--frame-deadline-ms" ? 60000 : 100000;
      if (!number || *number == 0 || *number > maximum)
        return std::unexpected(failure("out-of-range positive value for " + std::string(key)));
      const auto bounded = static_cast<std::uint32_t>(*number);
      if (key == "--warmup") options.warmup = bounded;
      else if (key == "--samples") options.samples = bounded;
      else if (key == "--frame-deadline-ms") options.frame_deadline_ms = bounded;
      else options.run_deadline_ms = bounded;
    } else return std::unexpected(failure("unknown option " + std::string(key)));
  }
  if (options.backend == cgpui::RendererBackend::unknown || options.profile.empty() ||
      options.output.empty() || !options.expected_software)
    return std::unexpected(failure("required: --backend dx12|vulkan --profile <actual-build-label> "
        "--adapter-kind hardware|software --output <new-directory>"));
  return options;
}
} // namespace renderer_probe
