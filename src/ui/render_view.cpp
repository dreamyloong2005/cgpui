#include "ui_internal.hpp"

namespace cgpui {

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    Color clear_color,
    std::vector<WindowControlRegion>* window_control_regions,
    const ElementTree* element_tree,
    FrameStatistics* statistics) {
  if (statistics != nullptr) {
    statistics->begin_frame_count += 1;
  }
  auto frame = renderer.begin_frame();
  if (!frame) {
    return std::unexpected(frame.error());
  }
  if (!*frame) {
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "Renderer returned an empty frame",
    });
  }

  (*frame)->clear(clear_color);
  if (statistics != nullptr) {
    statistics->clear_count += 1;
  }

  PaintList paint_list;
  paint_list.set_scale(scale);
  paint_list.set_text_measurement_cache(text_measurement_cache);
  view.paint(paint_list, viewport_size);
  if (element_tree != nullptr) {
    element_tree->paint(paint_list);
  }
  if (window_control_regions != nullptr) {
    const auto regions = paint_list.window_control_regions();
    window_control_regions->assign(regions.begin(), regions.end());
  }
  if (statistics != nullptr) {
    statistics->paint_pass_count += 1;
    statistics->paint_command_count = paint_list.commands().size();
  }
  for (const ImageAssetId asset_id : paint_list.image_invalidations()) {
    (*frame)->invalidate_image(asset_id);
  }
  for (const ImageAsset* image : paint_list.image_uploads()) {
    if (image != nullptr) (*frame)->upload_image(*image);
  }
  for (const auto& command : paint_list.commands()) {
    submit_paint_command_to_frame(**frame, command, statistics);
  }

  auto result = (*frame)->present();
  if (result && statistics != nullptr) {
    statistics->present_count += 1;
    apply_runtime_renderer_frame_diagnostics(
        *statistics,
        renderer.last_frame_diagnostic_snapshot());
  }
  return result;
}

} // namespace cgpui
