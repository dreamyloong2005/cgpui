#pragma once

#include "cgpui/core/geometry.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>

namespace cgpui {

enum class Overflow {
  visible,
  hidden,
};

enum class AlignItems {
  start,
  center,
  end,
};

enum class JustifyContent {
  start,
  center,
  end,
  space_between,
};

enum class Position {
  relative,
  absolute,
};

enum class AnimationEasing {
  linear,
  ease_in,
  ease_out,
  ease_in_out,
};

[[nodiscard]] constexpr float px(float value) {
  return value;
}

[[nodiscard]] constexpr Color rgb(int red, int green, int blue) {
  return Color{
      .r = static_cast<float>(red) / 255.0F,
      .g = static_cast<float>(green) / 255.0F,
      .b = static_cast<float>(blue) / 255.0F,
      .a = 1.0F,
  };
}

[[nodiscard]] constexpr Color rgba(
    int red,
    int green,
    int blue,
    float alpha) {
  return Color{
      .r = static_cast<float>(red) / 255.0F,
      .g = static_cast<float>(green) / 255.0F,
      .b = static_cast<float>(blue) / 255.0F,
      .a = alpha,
  };
}

[[nodiscard]] inline float clamp_animation_progress(float progress) {
  return std::clamp(progress, 0.0F, 1.0F);
}

[[nodiscard]] inline float ease(AnimationEasing easing, float progress) {
  const float clamped = clamp_animation_progress(progress);
  switch (easing) {
    case AnimationEasing::ease_in:
      return clamped * clamped;
    case AnimationEasing::ease_out:
      return 1.0F - (1.0F - clamped) * (1.0F - clamped);
    case AnimationEasing::ease_in_out:
      if (clamped < 0.5F) {
        return 2.0F * clamped * clamped;
      }
      return 1.0F - 2.0F * (1.0F - clamped) * (1.0F - clamped);
    case AnimationEasing::linear:
    default:
      return clamped;
  }
}

[[nodiscard]] inline float tween(float from, float to, float progress) {
  const double clamped =
      static_cast<double>(clamp_animation_progress(progress));
  return static_cast<float>(
      static_cast<double>(from) +
      (static_cast<double>(to) - static_cast<double>(from)) * clamped);
}

[[nodiscard]] inline float tween_opacity(float from, float to, float progress) {
  const double value = static_cast<double>(tween(from, to, progress));
  return static_cast<float>(std::round(value * 1'000'000.0) / 1'000'000.0);
}

[[nodiscard]] inline Color tween(Color from, Color to, float progress) {
  return Color{
      .r = tween(from.r, to.r, progress),
      .g = tween(from.g, to.g, progress),
      .b = tween(from.b, to.b, progress),
      .a = tween(from.a, to.a, progress),
  };
}

[[nodiscard]] inline AffineTransform tween(
    AffineTransform from,
    AffineTransform to,
    float progress) {
  return AffineTransform{
      .scale_x = tween(from.scale_x, to.scale_x, progress),
      .skew_y = tween(from.skew_y, to.skew_y, progress),
      .skew_x = tween(from.skew_x, to.skew_x, progress),
      .scale_y = tween(from.scale_y, to.scale_y, progress),
      .translate_x = tween(from.translate_x, to.translate_x, progress),
      .translate_y = tween(from.translate_y, to.translate_y, progress),
  };
}

struct EdgeSizes {
  float top = 0.0F;
  float right = 0.0F;
  float bottom = 0.0F;
  float left = 0.0F;

  [[nodiscard]] static constexpr EdgeSizes all(float value) {
    return EdgeSizes{
        .top = value,
        .right = value,
        .bottom = value,
        .left = value,
    };
  }

  [[nodiscard]] static constexpr EdgeSizes axes(
      float horizontal,
      float vertical) {
    return EdgeSizes{
        .top = vertical,
        .right = horizontal,
        .bottom = vertical,
        .left = horizontal,
    };
  }

  [[nodiscard]] static constexpr EdgeSizes trbl(
      float top,
      float right,
      float bottom,
      float left) {
    return EdgeSizes{
        .top = top,
        .right = right,
        .bottom = bottom,
        .left = left,
    };
  }
};

struct PercentageSize {
  std::optional<float> width;
  std::optional<float> height;
};

[[nodiscard]] constexpr EdgeSizes edges(float value) {
  return EdgeSizes::all(value);
}

[[nodiscard]] constexpr EdgeSizes edges(float horizontal, float vertical) {
  return EdgeSizes::axes(horizontal, vertical);
}

[[nodiscard]] constexpr EdgeSizes edges(
    float top,
    float right,
    float bottom,
    float left) {
  return EdgeSizes::trbl(top, right, bottom, left);
}

struct BorderRadii {
  float top_left = 0.0F;
  float top_right = 0.0F;
  float bottom_right = 0.0F;
  float bottom_left = 0.0F;

  [[nodiscard]] static constexpr BorderRadii all(float value) {
    return BorderRadii{
        .top_left = value,
        .top_right = value,
        .bottom_right = value,
        .bottom_left = value,
    };
  }

  [[nodiscard]] static constexpr BorderRadii corners(
      float top_left,
      float top_right,
      float bottom_right,
      float bottom_left) {
    return BorderRadii{
        .top_left = top_left,
        .top_right = top_right,
        .bottom_right = bottom_right,
        .bottom_left = bottom_left,
    };
  }
};

struct BoxShadow {
  Color color;
  Point offset;
  float blur_radius = 0.0F;
  float spread_radius = 0.0F;
};

struct FontDescriptor {
  std::string family;

  friend bool operator==(
      const FontDescriptor&,
      const FontDescriptor&) = default;
};

} // namespace cgpui
