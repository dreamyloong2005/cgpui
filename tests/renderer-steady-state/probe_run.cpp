#include "probe_window.hpp"
#include <algorithm>

namespace renderer_probe {
namespace {
cgpui::Result<std::unique_ptr<cgpui::Renderer>> create(const Options& options,
    const Window& window, cgpui::RendererBackendInfo& actual,
    std::optional<RendererCreationSample>* creation = nullptr) {
  const auto descriptor = window.descriptor(); // Fixture preparation is outside factory timing.
  cgpui::Result<std::unique_ptr<cgpui::Renderer>> renderer;
  const auto factory_started = Clock::now();
  try { // Named factories bypass CGPUI_RENDERER, with no backend fallback.
    renderer = options.backend == cgpui::RendererBackend::dx12
        ? cgpui::create_dx12_renderer(descriptor) : cgpui::create_vulkan_renderer(descriptor);
  } catch (...) {
    const auto factory_stopped = Clock::now();
    if (creation) *creation = RendererCreationSample{
        elapsed_ns(factory_started, factory_stopped), CreationOutcome::exception};
    throw;
  }
  const auto factory_stopped = Clock::now();
  if (creation) *creation = RendererCreationSample{elapsed_ns(factory_started, factory_stopped),
      !renderer ? CreationOutcome::factory_error : !*renderer ? CreationOutcome::null_renderer : CreationOutcome::created};
  if (!renderer) return std::unexpected(renderer.error());
  if (!*renderer) return std::unexpected(failure("named factory returned a null renderer"));
  actual = (*renderer)->backend_info();
  const auto expected_selection = options.backend == cgpui::RendererBackend::dx12
      ? cgpui::RendererBackendSelection::dx12 : cgpui::RendererBackendSelection::vulkan;
  if (actual.backend != options.backend || actual.selection != expected_selection ||
      !actual.fallback_reason.empty() || actual.adapter_name.empty() || !actual.device_generation ||
      actual.software_adapter != *options.expected_software ||
      (options.expected_luid && actual.adapter_luid != options.expected_luid))
    return std::unexpected(failure("actual backend/adapter identity does not match explicit request"));
  return renderer;
}
bool same_device(const cgpui::RendererBackendInfo& a, const cgpui::RendererBackendInfo& b) {
  return a.backend == b.backend && a.adapter_name == b.adapter_name &&
      a.vendor_id == b.vendor_id && a.adapter_luid == b.adapter_luid &&
      a.software_adapter == b.software_adapter && a.selection == b.selection &&
      a.fallback_reason == b.fallback_reason && a.device_generation == b.device_generation;
}
cgpui::Result<Sample> sample(cgpui::Renderer& renderer, const Window& window,
    const Scene& scene, bool cold, Clock::time_point deadline) {
  if (auto visible = window.pump(); !visible) return std::unexpected(visible.error());
  auto acquired = acquire(renderer, window, deadline);
  if (!acquired) return std::unexpected(acquired.error());
  scene.replay(*acquired->frame, cold);
  const auto recorded = Clock::now();
  if (recorded >= deadline) return std::unexpected(failure("recording exceeded frame deadline"));
  const auto presented = acquired->frame->present(); // Exactly once, no retry or capture.
  const auto completed = Clock::now();
  if (!presented) return std::unexpected(presented.error());
  if (completed >= deadline) return std::unexpected(failure("Present returned after deadline"));
  Sample result{.attempts = acquired->attempts,
      .acquire_wall_ns = elapsed_ns(acquired->started, acquired->acquired),
      .begin_api_wall_ns = acquired->api_wall_ns, .wait_wall_ns = acquired->wait_wall_ns,
      .record_wall_ns = elapsed_ns(acquired->acquired, recorded),
      .present_wall_ns = elapsed_ns(recorded, completed),
      .frame_wall_ns = elapsed_ns(acquired->started, completed)};
  const auto* diagnostic = renderer.last_frame_diagnostic_snapshot();
  const auto expected_commands = scene.rectangles() + scene.images();
  if (!diagnostic || !diagnostic->work.exact_match() || diagnostic->timings.saturated ||
      diagnostic->upload_bytes.saturated || diagnostic->submitted_draws.saturated ||
      diagnostic->work.submitted_work.command_count != expected_commands ||
      diagnostic->submitted_draws.solid_rect_count != scene.rectangles() ||
      diagnostic->submitted_draws.image_count != scene.images() ||
      diagnostic->submitted_draws.total_count != expected_commands ||
      diagnostic->upload_bytes.image_byte_count != (cold ? scene.images() * 16 : 0) ||
      diagnostic->upload_bytes.glyph_atlas_byte_count || renderer.last_frame_pixels())
    return std::unexpected(failure("uncaptured frame work/upload/diagnostic invariant failed"));
  result.stages = diagnostic->timings;
  result.commands = diagnostic->work.submitted_work.command_count;
  result.draws = diagnostic->submitted_draws.total_count;
  result.batches = diagnostic->work.submitted_work.batch_count;
  result.image_bytes = diagnostic->upload_bytes.image_byte_count;
  result.glyph_bytes = diagnostic->upload_bytes.glyph_atlas_byte_count;
  // Outside t0..t3: drain changes and validate fixed-display premises before accepting the sample.
  if (auto stable = window.pump(); !stable) return std::unexpected(stable.error());
  return result;
}
} // namespace
cgpui::Result<void> run(const Options& options, Report& report) {
  const auto run_deadline = Clock::now() + std::chrono::milliseconds(options.run_deadline_ms);
  const auto deadline = [&] {
    return std::min(run_deadline, Clock::now() + std::chrono::milliseconds(options.frame_deadline_ms));
  };
  Window window;
  if (auto ready = window.pump(); !ready) return ready;
  if (auto fixed = window.lock_display(report); !fixed) return fixed;
  const Scene scene(options.scene);
  report.samples.reserve(options.samples); // Never grows inside measured frames.
  report.phase = "preflight_capture";
  {
    cgpui::RendererBackendInfo identity;
    auto preflight = create(options, window, identity);
    report.preflight_actual = identity;
    if (!preflight) return std::unexpected(preflight.error());
    if (auto verified = verify_scene(**preflight, window, scene, true, deadline()); !verified) return verified;
    report.setup_pixels_valid = true;
  } // Preflight renderer and its capture storage are retired before timing setup.
  report.phase = "create_measured_renderer";
  auto renderer = create(options, window, report.actual, &report.measured_creation);
  if (!renderer) return std::unexpected(renderer.error());
  if (!same_device(*report.preflight_actual, report.actual))
    return std::unexpected(failure("preflight and measured renderer chose different devices"));
  report.phase = "cold_no_capture";
  auto cold = sample(**renderer, window, scene, true, deadline());
  if (!cold) return std::unexpected(cold.error());
  report.cold = *cold;
  report.phase = "warmup_no_capture";
  for (std::uint32_t index = 0; index != options.warmup; ++index) {
    if (auto warm = sample(**renderer, window, scene, false, deadline()); !warm)
      return std::unexpected(warm.error());
    ++report.warmup_completed;
  }
  report.phase = "measured_no_capture";
  for (std::uint32_t index = 0; index != options.samples; ++index) {
    auto measured = sample(**renderer, window, scene, false, deadline());
    if (!measured) return std::unexpected(measured.error());
    measured->index = index;
    report.samples.push_back(*measured); // Reserved, no console/file IO here.
  }
  if (!same_device(report.actual, (*renderer)->backend_info()))
    return std::unexpected(failure("device identity/generation changed during the series"));
  report.phase = "postflight_capture";
  if (auto verified = verify_scene(**renderer, window, scene, false, deadline()); !verified) return verified;
  report.final_pixels_valid = true;
  if (!same_device(report.actual, (*renderer)->backend_info()))
    return std::unexpected(failure("device identity/generation changed during final verification"));
  report.phase = "complete";
  report.status = "valid_baseline"; // Valid measurement, not a performance pass/speedup.
  return {};
}
} // namespace renderer_probe
