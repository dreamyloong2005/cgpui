#include "renderer_pixel_test_support.hpp"
#include <cstdio>

using namespace cgpui;
using namespace cgpui_test;
namespace {
void scene(RenderFrame& frame, ImageAssetDescriptor asset) {
  frame.draw_rect({.rect = {{0, 0}, {24, 24}}, .color = {1, 0, 0, 1}});
  frame.draw_image({.bounds = {{8, 8}, {32, 32}}, .asset = asset,
      .sampling = ImageSamplingMode::nearest});
  frame.draw_rect({.rect = {{24, 0}, {24, 24}}, .color = {0, 0, 1, 1}});
}
bool missing_diagnostics(const RendererFrameDiagnosticSnapshot* s) {
  return s && s->planned_draws.image_count == 1 && s->submitted_draws.image_count == 0 &&
      s->planned_draws.solid_rect_count == 2 && s->submitted_draws.solid_rect_count == 2 &&
      s->work.planned_work.command_count == 3 && s->work.submitted_work.command_count == 2 &&
      s->work.pending_command_count == 1 && s->work.pending_draw_count == 1 &&
      s->work.unexpected_command_count == 0 && s->work.unexpected_draw_count == 0 &&
      !s->work.exact_match() && s->upload_bytes.image_byte_count == 0 &&
      s->dropped_resources.resources.size() == 1 &&
      s->dropped_resources.missing_submission_resource_count == 1 &&
      s->dropped_resources.unsupported_primitive_count == 0 &&
      s->dropped_resources.resources[0].resource.primitive_kind == RendererPrimitiveKind::image &&
      s->dropped_resources.resources[0].reason == RendererDroppedResourceReason::missing_submission_resource;
}
int fail(int code, const char* message) { std::fprintf(stderr, "%s\n", message); return code; }
}
int main() {
  RendererPixelTestWindow window(L"CGPUIMissingImageContract", L"CGPUI Missing Image Contract");
  if (!window.valid()) return fail(1, "fixture window unavailable");
  auto renderer = create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return fail(2, renderer.error().message.c_str());
  ImageAsset green{.id = {971413}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
  const auto asset = describe_image_asset(green);
  {
    auto frame = begin_pixel_capture(**renderer);
    if (!frame) return fail(3, "seed begin/capture failed");
    (*frame)->clear({0, 0, 0, 1});
    if (auto result = (*frame)->present(); !result) return fail(4, result.error().message.c_str());
    const auto pixels = completed_pixel_capture(**renderer);
    if (!pixels || !pixel_near(*pixels, 12, 32, {0, 0, 0, 255})) return fail(5, "seed pixels differ");
  }
  bool truthful_missing = false;
  {
    auto frame = begin_pixel_capture(**renderer);
    if (!frame) return fail(6, "missing begin/capture failed");
    (*frame)->clear({0, 1, 1, 1});
    scene(**frame, asset);
    if (auto result = (*frame)->present(); !result) {
      report_pixel_test_error("missing-image Present", result.error());
      return result.error().code == ErrorCode::invalid_argument
          ? fail(40, "MISSING_IMAGE_REJECTED: valid unavailable pixels rejected")
          : fail(7, "unrelated native/presentation failure, not semantic missing-image RED");
    }
    const auto pixels = completed_pixel_capture(**renderer);
    if (!pixels || !pixel_near(*pixels, 12, 12, {255, 0, 0, 255}) ||
        !pixel_near(*pixels, 28, 12, {0, 0, 255, 255}) ||
        !pixel_near(*pixels, 12, 32, {0, 255, 255, 255}) ||
        !pixel_near(*pixels, 56, 56, {0, 255, 255, 255})) return fail(8, "missing pixels/order/history differ");
    truthful_missing = missing_diagnostics((*renderer)->last_frame_diagnostic_snapshot());
  }
  for (unsigned step = 0; step < 2; ++step) {
    auto frame = begin_pixel_capture(**renderer);
    if (!frame) return fail(9, "resupply/cached begin failed");
    (*frame)->clear(step == 0 ? Color{1, 0, 1, 1} : Color{1, 1, 0, 1});
    if (step == 0) {
      (*frame)->upload_image(green);
      green.bitmap.pixels.assign(4, 0); // Caller ownership after recording.
    }
    scene(**frame, asset);
    if (auto result = (*frame)->present(); !result) return fail(10, result.error().message.c_str());
    const auto pixels = completed_pixel_capture(**renderer);
    if (!pixels || !pixel_near(*pixels, 12, 12, {0, 255, 0, 255}) ||
        !pixel_near(*pixels, 12, 32, {0, 255, 0, 255}) ||
        !pixel_near(*pixels, 28, 12, {0, 0, 255, 255}) ||
        !pixel_near(*pixels, 56, 56, step == 0 ? std::array<std::uint8_t, 4>{255, 0, 255, 255}
                                            : std::array<std::uint8_t, 4>{255, 255, 0, 255}))
      return fail(11, "resupply/cached image/order/fresh background differ");
    const auto* s = (*renderer)->last_frame_diagnostic_snapshot();
    if (!s || s->planned_draws.image_count != 1 || s->submitted_draws.image_count != 1 ||
        !s->dropped_resources.empty() || !s->work.exact_match() ||
        s->upload_bytes.image_byte_count != (step == 0 ? 4U : 0U)) return fail(12, "available-image diagnostics differ");
  }
  return truthful_missing ? 0 : fail(41, "MISSING_IMAGE_PHANTOM_SUBMISSION: pixels pass but missing accounting fails");
}
