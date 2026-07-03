#include "vulkan_report_internal.hpp"

namespace cgpui {

void vulkan_record_submission_plan_statistics(RendererCommandReport& report) {
  report.submission_plan_record_count = report.submission_plan_records.size();
  report.submission_plan_command_count = 0;
  report.submission_plan_glyph_quad_count = 0;
  for (const RendererSubmissionPlanRecord& record :
       report.submission_plan_records) {
    report.submission_plan_command_count += record.command_count;
    report.submission_plan_glyph_quad_count += record.glyph_quad_count;
  }
}

} // namespace cgpui
