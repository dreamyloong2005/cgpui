#include "../../src/renderer/dx12/dx12_image_retention_internal.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace cgpui;
namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
Dx12TextureHandle texture(std::size_t bytes, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM) {
  auto image = std::make_shared<Dx12Texture>();
  image->format = format;
  image->pixels.resize(bytes);
  return image;
}
void ownership() {
  Dx12ImageRetention tracker, independent;
  require(tracker.sample().byte_count == 0, "empty observed payload must be known zero");
  auto red = texture(16), green = texture(16), glyph = texture(8, DXGI_FORMAT_R8_UNORM);
  std::weak_ptr<const Dx12Texture> red_observer = red;
  tracker.track(red); tracker.track(red); tracker.track(glyph);
  auto draw = red;
  require(tracker.sample().byte_count == 16 && independent.sample().byte_count == 0,
      "references, repeated registration, glyphs or other renderers changed the image census");
  tracker.track(green);
  red.reset();
  require(tracker.sample().byte_count == 32, "old immutable draw snapshot was lost on replacement");
  draw.reset();
  require(red_observer.expired() && tracker.sample().byte_count == 16,
      "weak tracker prolonged the old snapshot lifetime");
  green.reset();
  require(tracker.sample().byte_count == 0, "final image owner release was not observed");
  for (unsigned index = 0; index != 512; ++index) {
    auto replacement = texture(4);
    tracker.track(replacement);
    require(tracker.sample().byte_count == 4, "expired entries/addresses became duplicate live images");
  }
  require(tracker.sample().byte_count == 0 && !tracker.sample().saturated,
      "expired replacement observations were retained as payload");
}
void arithmetic() {
  const auto maximum = std::numeric_limits<std::size_t>::max();
  Dx12ImageRetentionSample sample;
  dx12_add_retained_image_bytes(sample, maximum);
  require(sample.byte_count == maximum && !sample.saturated, "exact maximum was labelled overflow");
  dx12_add_retained_image_bytes(sample, 1); // Synthetic arithmetic, no enormous image allocation.
  require(sample.byte_count == maximum && sample.saturated, "payload addition wrapped instead of saturating");
  dx12_add_retained_image_bytes(sample, 0);
  require(sample.saturated, "one sample forgot an earlier overflow");
  Dx12ImageRetentionSample unknown{.byte_count = std::nullopt};
  dx12_add_retained_image_bytes(unknown, 4);
  require(!unknown.byte_count && !unknown.saturated, "unavailable payload turned into a known undercount");
  Dx12ImageRetentionSample next;
  dx12_add_retained_image_bytes(next, 4);
  require(next.byte_count == 4 && !next.saturated, "saturation leaked between independent samples");
}
}
int main() {
  try { ownership(); arithmetic(); return 0; }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
