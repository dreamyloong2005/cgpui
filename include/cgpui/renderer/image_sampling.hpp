#pragma once

#include <string_view>

namespace cgpui {

enum class ImageSamplingMode {
  nearest,
  linear,
};

[[nodiscard]] constexpr std::string_view image_sampling_mode_name(
    ImageSamplingMode mode) {
  switch (mode) {
    case ImageSamplingMode::nearest:
      return "nearest";
    case ImageSamplingMode::linear:
      return "linear";
  }
  return "linear";
}

} // namespace cgpui
