#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace cgpui {
class Renderer;
class RenderFrame;
struct ImageAsset;
struct ImageAssetDescriptor;
}
namespace cgpui_test {
struct ImageRetentionExpectation {
  std::array<unsigned char, 4> pixel;
  std::size_t payload;
  std::size_t uploads;
};
cgpui::ImageAsset retention_image(std::uint64_t id, std::array<unsigned char, 4> color);
bool retention_sample(const cgpui::Renderer& renderer, std::size_t expected,
                      std::optional<std::size_t> uploads = std::nullopt);
bool retention_present_image(cgpui::Renderer& renderer, cgpui::RenderFrame& frame,
    const cgpui::ImageAssetDescriptor& image, const ImageRetentionExpectation& expected);
bool retention_empty_frame(cgpui::Renderer& renderer, std::size_t expected);
} // namespace cgpui_test
