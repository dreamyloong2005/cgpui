#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"
#include <limits>
#include <utility>

int main() {
  using cgpui::RendererFrameDiagnosticSnapshot;
  const RendererFrameDiagnosticSnapshot unreported;
  if (unreported.retained_image_byte_count || unreported.retained_image_byte_count_saturated) return 1;
  RendererFrameDiagnosticSnapshot empty{.retained_image_byte_count = 0};
  if (!empty.retained_image_byte_count || *empty.retained_image_byte_count != 0 ||
      empty.retained_image_byte_count == unreported.retained_image_byte_count) return 2;
  RendererFrameDiagnosticSnapshot measured{.retained_image_byte_count = 16};
  const auto copied = measured;
  const auto moved = std::move(measured);
  if (copied.retained_image_byte_count != 16 || moved.retained_image_byte_count != 16 ||
      copied.retained_image_byte_count_saturated || moved.retained_image_byte_count_saturated) return 3;
  RendererFrameDiagnosticSnapshot clamped{
      .retained_image_byte_count = std::numeric_limits<std::size_t>::max(),
      .retained_image_byte_count_saturated = true};
  const auto history = clamped;
  clamped = {};
  if (clamped.retained_image_byte_count || clamped.retained_image_byte_count_saturated ||
      history.retained_image_byte_count != std::numeric_limits<std::size_t>::max() ||
      !history.retained_image_byte_count_saturated || history.upload_bytes.saturated) return 4;
  return 0;
}
