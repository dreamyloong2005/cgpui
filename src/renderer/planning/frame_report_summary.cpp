#include "cgpui/renderer/renderer_frame_reports.hpp"

namespace cgpui {

RendererFrameReport
renderer_frame_report_from_command_report(
    const RendererCommandReport& command_report) {
  RendererFrameReport frame_report;
  frame_report.command_report = command_report;
  frame_report.supported_primitive_count =
      command_report.supported_command_count;
  frame_report.unsupported_primitive_count =
      command_report.unsupported_command_count;
  frame_report.renderer_batch_count = command_report.batches.size();
  frame_report.submission_plan_record_count =
      command_report.submission_plan_record_count == 0
          ? command_report.submission_plan_records.size()
          : command_report.submission_plan_record_count;
  frame_report.glyph_upload_record_count =
      command_report.text_render.glyph_upload_record_count;
  frame_report.textured_glyph_quad_count =
      command_report.text_render.textured_glyph_quad_count;
  frame_report.image_upload_plan_count =
      command_report.image_render.image_upload_plan_count;
  frame_report.image_upload_byte_count =
      command_report.image_render.image_upload_byte_count;

  if (command_report.unsupported_command_count > 0) {
    frame_report.gaps.push_back(RendererFrameGap{
        .kind = RendererFrameGapKind::unsupported_command,
        .count = command_report.unsupported_command_count,
        .message = "unsupported renderer commands remain in the frame",
    });
  }
  if (command_report.text_render.text_sampler_pipeline_pending_text_draw_count >
      0) {
    frame_report.gaps.push_back(RendererFrameGap{
        .kind = RendererFrameGapKind::pending_text_sampler_pipeline,
        .primitive_kind = RendererPrimitiveKind::text,
        .count = command_report.text_render
                     .text_sampler_pipeline_pending_text_draw_count,
        .message = "text sampler pipeline is not ready for glyph draw work",
    });
  }
  if (command_report.text_render.metadata_only_text_draw_count > 0) {
    frame_report.gaps.push_back(RendererFrameGap{
        .kind = RendererFrameGapKind::metadata_only_text_draw,
        .primitive_kind = RendererPrimitiveKind::text,
        .count = command_report.text_render.metadata_only_text_draw_count,
        .message = "text draw commands had no glyph-backed quads",
    });
  }
  if (command_report.supported_command_count > 0 &&
      frame_report.submission_plan_record_count == 0) {
    frame_report.gaps.push_back(RendererFrameGap{
        .kind = RendererFrameGapKind::missing_submission_plan,
        .count = command_report.supported_command_count,
        .message = "supported commands did not produce submission plans",
    });
  }

  frame_report.gap_count = frame_report.gaps.size();
  return frame_report;
}

} // namespace cgpui
