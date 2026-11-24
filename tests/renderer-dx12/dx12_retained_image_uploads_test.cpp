#include "../support/frame_wait_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "dx12_image_retention_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "retained/upload payload renderer unavailable");
  const auto image = diagnostic_image(); // Tight16 bytes; source has an extra4 padding bytes.
  auto upload_only = wait_for_test_frame(**renderer);
  if (!upload_only) return dx12_fail(2, "upload-only frame unavailable");
  (*upload_only)->upload_image(image);
  (*upload_only)->clear({0, 0, 1, 1});
  if (!(*upload_only)->request_pixel_capture() || !(*upload_only)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 32, 32, {0, 0, 255, 255}))
    return dx12_fail(3, "actual upload-only Present/readback failed before the retention oracle");
  const auto* first = (*renderer)->last_frame_diagnostic_snapshot();
  if (!first) return dx12_fail(8, "successful real frame lacked its existing diagnostic snapshot");
  std::cout << "IMAGE_RETENTION upload_only=1 reported=" << bool(first->retained_image_byte_count)
            << " payload=" << first->retained_image_byte_count.value_or(0)
            << " saturated=" << first->retained_image_byte_count_saturated
            << " image_uploads=" << first->upload_bytes.image_byte_count << std::endl;
  if (!retention_sample(**renderer, 16, 0))
    return dx12_fail(40, "FAIL_UNREPORTED_RETAINED_PAYLOAD: real upload-only frame must retain16 and submit0 bytes");
  upload_only->reset();
  for (const bool warm : {false, true}) {
    auto frame = wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(4, "padded image frame unavailable");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->draw_image({.bounds = {{0, 0}, {64, 64}}, .asset = describe_image_asset(image),
        .sampling = ImageSamplingMode::nearest});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 48, 8, {0, 255, 0, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 8, 48, {0, 0, 255, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 48, 48, {255, 255, 255, 255}) ||
        !retention_sample(**renderer, 16, warm ? 0 : 16))
      return dx12_fail(5, "tight retained payload, first/warm copy bytes or actual padded-image pixels differ");
  }
  auto glyph = wait_for_test_frame(**renderer);
  if (!glyph) return dx12_fail(6, "glyph exclusion frame unavailable");
  (*glyph)->clear({0, 0, 0, 1});
  (*glyph)->draw_text(diagnostic_text("M", 24, {4, 4}));
  if (!(*glyph)->request_pixel_capture() || !(*glyph)->present() ||
      !diagnostic_ink((*renderer)->last_frame_pixels(), 0, 0, 48, 48) ||
      !retention_sample(**renderer, 16, 0) ||
      !(*renderer)->last_frame_diagnostic_snapshot()->upload_bytes.glyph_atlas_byte_count)
    return dx12_fail(7, "actual glyph upload was omitted or polluted image-only retention");
  return 0;
}
