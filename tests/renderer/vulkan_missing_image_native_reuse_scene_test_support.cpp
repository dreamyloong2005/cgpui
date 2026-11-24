#include "vulkan_missing_image_native_reuse_test_support.hpp"
#include <algorithm>

namespace cgpui_test {
std::array<cgpui::ImageAsset, 2> missing_reuse_images() {
  return {cgpui::ImageAsset{.id = {971421}, .bitmap = {.width = 1, .height = 1, .stride = 4,
              .pixels = {0, 255, 0, 255}}},
      cgpui::ImageAsset{.id = {971422}, .bitmap = {.width = 1, .height = 1, .stride = 4,
              .pixels = {255, 0, 255, 255}}}};
}
void draw_missing_reuse_scene(cgpui::RenderFrame& frame, const std::array<cgpui::ImageAsset, 2>& images) {
  frame.draw_rect({.rect = {{0, 0}, {24, 24}}, .color = {1, 0, 0, 1}});
  frame.draw_image({.bounds = {{8, 8}, {16, 32}}, .asset = cgpui::describe_image_asset(images[0]),
      .sampling = cgpui::ImageSamplingMode::nearest});
  frame.draw_image({.bounds = {{32, 8}, {16, 32}}, .asset = cgpui::describe_image_asset(images[1]),
      .sampling = cgpui::ImageSamplingMode::nearest});
  frame.draw_rect({.rect = {{40, 0}, {16, 24}}, .color = {0, 0, 1, 1}});
}
bool same_missing_reuse_capture(const cgpui::RendererFramePixels* actual, const cgpui::RendererFramePixels& seed) {
  return actual && actual->valid() && actual->width == seed.width && actual->height == seed.height &&
      actual->encoding == seed.encoding && actual->rgba8 == seed.rgba8;
}
bool missing_reuse_diagnostics(const cgpui::RendererFrameDiagnosticSnapshot* s,
    unsigned available_mask, unsigned upload_bytes) {
  const unsigned ready = unsigned(bool(available_mask & 1)) + unsigned(bool(available_mask & 2));
  const unsigned missing = 2 - ready;
  if (!s || s->planned_draws.image_count != 2 || s->submitted_draws.image_count != ready ||
      s->planned_draws.solid_rect_count != 2 || s->submitted_draws.solid_rect_count != 2 ||
      s->work.planned_work.command_count != 4 || s->work.submitted_work.command_count != 2 + ready ||
      s->work.pending_command_count != missing || s->work.pending_draw_count != missing ||
      s->work.unexpected_command_count || s->work.unexpected_draw_count ||
      s->work.exact_match() != (missing == 0) || s->upload_bytes.image_byte_count != upload_bytes ||
      s->dropped_resources.resources.size() != missing ||
      s->dropped_resources.missing_submission_resource_count != missing ||
      s->dropped_resources.unsupported_primitive_count) return false;
  for (unsigned index = 0; index != 2; ++index) {
    const auto count = std::ranges::count_if(s->dropped_resources.resources, [&](const auto& d) {
      return d.resource == cgpui::RendererFrameResource{cgpui::RendererPrimitiveKind::image, index, index} &&
          d.reason == cgpui::RendererDroppedResourceReason::missing_submission_resource;
    });
    if (count != ((available_mask & (1U << index)) ? 0 : 1)) return false;
  }
  return true;
}
} // namespace cgpui_test
