#include "cgpui/ui/animation_curve.hpp"

#include <cmath>

namespace cgpui {
namespace {

bool finite(float value) {
  return std::isfinite(value);
}

float spring_value(AnimationSpring spring, float progress) {
  if (progress <= 0.0F) return 0.0F;
  if (progress >= 1.0F) return 1.0F;

  const float omega = std::sqrt(spring.stiffness / spring.mass);
  const float damping_ratio = spring.damping /
      (2.0F * std::sqrt(spring.stiffness * spring.mass));
  float displacement = 0.0F;
  if (damping_ratio < 1.0F) {
    const float damped_omega =
        omega * std::sqrt(1.0F - damping_ratio * damping_ratio);
    const float coefficient =
        (damping_ratio * omega - spring.initial_velocity) / damped_omega;
    displacement = std::exp(-damping_ratio * omega * progress) *
        (std::cos(damped_omega * progress) +
         coefficient * std::sin(damped_omega * progress));
  } else if (std::fabs(damping_ratio - 1.0F) < 0.0001F) {
    displacement = std::exp(-omega * progress) *
        (1.0F + (omega - spring.initial_velocity) * progress);
  } else {
    const float root = std::sqrt(damping_ratio * damping_ratio - 1.0F);
    const float slow = -omega * (damping_ratio - root);
    const float fast = -omega * (damping_ratio + root);
    const float slow_weight =
        (-spring.initial_velocity - fast) / (slow - fast);
    displacement = slow_weight * std::exp(slow * progress) +
        (1.0F - slow_weight) * std::exp(fast * progress);
  }
  const float value = 1.0F - displacement;
  return finite(value) ? value : progress;
}

} // namespace

bool AnimationCurve::valid() const {
  if (kind_ == AnimationCurveKind::spring) {
    return finite(spring_.mass) && spring_.mass > 0.0F &&
        finite(spring_.stiffness) && spring_.stiffness > 0.0F &&
        finite(spring_.damping) && spring_.damping >= 0.0F &&
        finite(spring_.initial_velocity);
  }
  if (kind_ == AnimationCurveKind::pulsating) {
    return finite(minimum_) && finite(maximum_) && minimum_ <= maximum_;
  }
  return true;
}

float AnimationCurve::value_at(
    float progress,
    AnimationEasing inherited) const {
  const float clamped = clamp_animation_progress(progress);
  if (!valid()) return clamped;
  switch (kind_) {
    case AnimationCurveKind::easing:
      return ease(easing_, clamped);
    case AnimationCurveKind::quadratic:
      return clamped * clamped;
    case AnimationCurveKind::ease_out_quint:
      return 1.0F - std::pow(1.0F - clamped, 5.0F);
    case AnimationCurveKind::bounce: {
      const float reflected = clamped < 0.5F
          ? clamped * 2.0F
          : (1.0F - clamped) * 2.0F;
      return ease(easing_, reflected);
    }
    case AnimationCurveKind::pulsating: {
      constexpr float pi = 3.14159265358979323846F;
      const float wave = std::sin(clamped * 2.0F * pi);
      const float breath = (wave * wave * wave + wave) * 0.5F;
      const float normalized = (breath + 1.0F) * 0.5F;
      return minimum_ + normalized * (maximum_ - minimum_);
    }
    case AnimationCurveKind::spring:
      return spring_value(spring_, clamped);
    case AnimationCurveKind::inherited:
    default:
      return ease(inherited, clamped);
  }
}

} // namespace cgpui
