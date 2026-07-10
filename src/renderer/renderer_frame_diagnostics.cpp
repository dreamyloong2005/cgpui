#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

namespace cgpui {
namespace {

[[nodiscard]] std::size_t saturating_difference(
    std::size_t lhs,
    std::size_t rhs) {
  return lhs > rhs ? lhs - rhs : 0;
}

} // namespace

bool RendererFrameDiagnostics::exact_match() const {
  return pending_batch_count == 0 && unexpected_batch_count == 0 &&
         pending_command_count == 0 && unexpected_command_count == 0;
}

RendererFrameDiagnostics compare_renderer_frame_work(
    RendererFrameWork planned_work,
    RendererFrameWork submitted_work) {
  return RendererFrameDiagnostics{
      .planned_work = planned_work,
      .submitted_work = submitted_work,
      .pending_batch_count = saturating_difference(
          planned_work.batch_count,
          submitted_work.batch_count),
      .unexpected_batch_count = saturating_difference(
          submitted_work.batch_count,
          planned_work.batch_count),
      .pending_command_count = saturating_difference(
          planned_work.command_count,
          submitted_work.command_count),
      .unexpected_command_count = saturating_difference(
          submitted_work.command_count,
          planned_work.command_count),
  };
}

} // namespace cgpui
