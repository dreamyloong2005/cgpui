#include "window_runtime_test_support.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{
                      std::istreambuf_iterator<char>(source),
                      std::istreambuf_iterator<char>()}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

class DiagnosticRenderer final : public cgpui::Renderer {
 public:
  DiagnosticRenderer(
      RecordingFrame& frame,
      cgpui::RendererFrameDiagnosticSnapshot snapshot)
      : frame_(frame), snapshot_(std::move(snapshot)) {}

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>(new BorrowedFrame(frame_));
  }

  [[nodiscard]] const cgpui::RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const override {
    return &snapshot_;
  }

 private:
  class BorrowedFrame final : public cgpui::RenderFrame {
   public:
    explicit BorrowedFrame(RecordingFrame& frame) : frame_(frame) {}

    void clear(cgpui::Color color) override { frame_.clear(color); }
    void draw_rect(const cgpui::SolidRect& rect) override {
      frame_.draw_rect(rect);
    }
    cgpui::Result<void> present() override { return frame_.present(); }

   private:
    RecordingFrame& frame_;
  };

  RecordingFrame& frame_;
  cgpui::RendererFrameDiagnosticSnapshot snapshot_;
};

cgpui::RendererFrameDiagnosticSnapshot renderer_snapshot() {
  using Kind = cgpui::RendererPrimitiveKind;
  return cgpui::RendererFrameDiagnosticSnapshot{
      .work = {
          .planned_work = {
              .batch_count = 4,
              .command_count = 5,
              .upload_byte_count = 13,
              .draw_count = 6,
          },
          .submitted_work = {
              .batch_count = 3,
              .command_count = 4,
              .upload_byte_count = 13,
              .draw_count = 5,
          },
          .pending_batch_count = 1,
          .pending_command_count = 1,
          .pending_draw_count = 1,
      },
      .upload_bytes = {
          .glyph_atlas_byte_count = 8,
          .image_byte_count = 5,
          .total_byte_count = 13,
      },
      .planned_draws = {
          .solid_rect_count = 2,
          .text_count = 2,
          .text_caret_count = 1,
          .image_count = 1,
          .total_count = 6,
      },
      .submitted_draws = {
          .solid_rect_count = 2,
          .text_count = 2,
          .image_count = 1,
          .total_count = 5,
      },
      .dropped_resources = {
          .resources = {
              cgpui::RendererDroppedResourceDiagnostic{
                  .resource = {
                      .primitive_kind = Kind::text_caret,
                      .command_index = 2,
                      .resource_index = 2,
                  },
                  .reason =
                      cgpui::RendererDroppedResourceReason::unsupported_primitive,
              },
          },
          .counts = {
              .text_caret_count = 1,
              .total_count = 1,
          },
          .unsupported_primitive_count = 1,
      },
      .timings = {
          .pacing_wait_nanoseconds = 1'000'000,
          .resource_preparation_nanoseconds = 2'000'000,
          .image_acquisition_nanoseconds = 3'000'000,
          .command_recording_nanoseconds = 4'000'000,
          .queue_submission_nanoseconds = 5'000'000,
          .presentation_nanoseconds = 6'000'000,
          .total_nanoseconds = 21'000'000,
      },
  };
}

int test_propagates_renderer_diagnostics_after_present() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};
  DiagnosticRenderer renderer(fixture.frame, renderer_snapshot());
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&renderer};
      });

  if (runtime.run(cgpui::WindowDescriptor{}) != 0) {
    return 10;
  }
  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      runtime.diagnostics_snapshot();
  if (!diagnostics.last_frame_statistics.has_value() ||
      !diagnostics.last_frame_statistics->renderer_frame.has_value()) {
    return 11;
  }
  const cgpui::RendererFrameStatistics& renderer_statistics =
      *diagnostics.last_frame_statistics->renderer_frame;
  if (renderer_statistics.work.pending_command_count != 1 ||
      renderer_statistics.upload_bytes.total_byte_count != 13 ||
      renderer_statistics.planned_draws.total_count != 6 ||
      renderer_statistics.submitted_draws.total_count != 5 ||
      renderer_statistics.dropped_draws.text_caret_count != 1 ||
      renderer_statistics.dropped_resource_count != 1 ||
      renderer_statistics.unsupported_primitive_count != 1 ||
      renderer_statistics.timings.total_nanoseconds != 21'000'000) {
    return 12;
  }
  if (diagnostics.last_frame_statistics->frame_time_ms != 21.0 ||
      diagnostics.last_frame_statistics->render_time_ms != 0.0 ||
      diagnostics.last_frame_statistics->layout_time_ms != 0.0 ||
      diagnostics.last_frame_statistics->paint_time_ms != 0.0) {
    return 13;
  }
  if (!diagnostics.last_renderer_frame_diagnostics.has_value() ||
      diagnostics.last_renderer_frame_diagnostics->dropped_resources.resources
              .size() != 1 ||
      diagnostics.last_renderer_frame_diagnostics->dropped_resources.resources[0]
              .resource.command_index != 2 ||
      !diagnostics.last_render_record.has_value() ||
      !diagnostics.last_render_record->statistics.has_value() ||
      !diagnostics.last_render_record->statistics->renderer_frame.has_value()) {
    return 14;
  }
  return fixture.frame.present_count == 1 ? 0 : 15;
}

int test_step_521_structure_and_documentation() {
  const std::string public_header =
      read_source("include/cgpui/ui/runtime_renderer_diagnostics.hpp");
  const std::string private_header =
      read_source("src/ui/runtime_renderer_diagnostics_internal.hpp");
  const std::string state_header =
      read_source("src/ui/runtime_renderer_diagnostic_state_internal.hpp");
  const std::string source =
      read_source("src/ui/runtime_renderer_diagnostics.cpp");
  const std::string rendering = read_source("src/ui/render_view.cpp");
  if (!contains(public_header, "struct RendererFrameStatistics") ||
      !contains(private_header,
                "apply_runtime_renderer_frame_diagnostics(") ||
      !contains(state_header, "last_renderer_frame_diagnostics_") ||
      !contains(source, "renderer.last_frame_diagnostic_snapshot()") ||
      !contains(rendering, "apply_runtime_renderer_frame_diagnostics(")) {
    return 50;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 521",
      "RendererFrameStatistics",
      "successful renderer presentation",
      "without Vulkan downcasts",
      "Step 522 renderer diagnostics closeout",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 60;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_propagates_renderer_diagnostics_after_present();
      result != 0) {
    return result;
  }
  return test_step_521_structure_and_documentation();
}
