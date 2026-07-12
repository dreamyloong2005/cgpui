#pragma once

#include "cgpui/ui/style_values.hpp"

#include <cstdint>

namespace cgpui {

struct AnimationSpring {
  float mass = 1.0F;
  float stiffness = 170.0F;
  float damping = 26.0F;
  float initial_velocity = 0.0F;

  friend bool operator==(const AnimationSpring&, const AnimationSpring&) =
      default;
};

enum class AnimationCurveKind : std::uint8_t {
  inherited,
  easing,
  quadratic,
  ease_out_quint,
  bounce,
  pulsating,
  spring,
};

class AnimationCurve {
 public:
  constexpr AnimationCurve() = default;

  [[nodiscard]] static constexpr AnimationCurve from_easing(
      AnimationEasing easing);
  [[nodiscard]] static constexpr AnimationCurve quadratic();
  [[nodiscard]] static constexpr AnimationCurve ease_out_quint();
  [[nodiscard]] static constexpr AnimationCurve bounce(
      AnimationEasing easing);
  [[nodiscard]] static constexpr AnimationCurve pulsating_between(
      float minimum,
      float maximum);
  [[nodiscard]] static constexpr AnimationCurve spring(
      AnimationSpring parameters = {});

  [[nodiscard]] AnimationCurveKind kind() const { return kind_; }
  [[nodiscard]] bool valid() const;
  [[nodiscard]] float value_at(
      float progress,
      AnimationEasing inherited = AnimationEasing::linear) const;

  friend bool operator==(const AnimationCurve&, const AnimationCurve&) =
      default;

 private:
  AnimationCurveKind kind_ = AnimationCurveKind::inherited;
  AnimationEasing easing_ = AnimationEasing::linear;
  AnimationSpring spring_;
  float minimum_ = 0.0F;
  float maximum_ = 1.0F;
};

constexpr AnimationCurve AnimationCurve::from_easing(AnimationEasing easing) {
  AnimationCurve curve;
  curve.kind_ = AnimationCurveKind::easing;
  curve.easing_ = easing;
  return curve;
}

constexpr AnimationCurve AnimationCurve::quadratic() {
  AnimationCurve curve;
  curve.kind_ = AnimationCurveKind::quadratic;
  return curve;
}

constexpr AnimationCurve AnimationCurve::ease_out_quint() {
  AnimationCurve curve;
  curve.kind_ = AnimationCurveKind::ease_out_quint;
  return curve;
}

constexpr AnimationCurve AnimationCurve::bounce(AnimationEasing easing) {
  AnimationCurve curve;
  curve.kind_ = AnimationCurveKind::bounce;
  curve.easing_ = easing;
  return curve;
}

constexpr AnimationCurve AnimationCurve::pulsating_between(
    float minimum,
    float maximum) {
  AnimationCurve curve;
  curve.kind_ = AnimationCurveKind::pulsating;
  curve.minimum_ = minimum;
  curve.maximum_ = maximum;
  return curve;
}

constexpr AnimationCurve AnimationCurve::spring(AnimationSpring parameters) {
  AnimationCurve curve;
  curve.kind_ = AnimationCurveKind::spring;
  curve.spring_ = parameters;
  return curve;
}

} // namespace cgpui
