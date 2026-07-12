#pragma once

#include "cgpui/ui/runtime_handles.hpp"

#include <functional>
#include <optional>

namespace cgpui {

class AsyncContextCapability;
class WindowRuntime;
struct WindowRuntimeContext;

struct AnimationTransition {
  float from = 0.0F;
  float to = 1.0F;

  [[nodiscard]] float value_at(float eased_progress) const;
};

struct AnimationTransitionSnapshot {
  AnimationSnapshot animation;
  float from = 0.0F;
  float to = 1.0F;
  float value = 0.0F;
};

using AnimationTransitionCallback = std::function<void(
    const WindowRuntimeContext&,
    const AnimationTransitionSnapshot&)>;

class AnimationTransitionHandle {
 public:
  AnimationTransitionHandle() = default;

  [[nodiscard]] AnimationId id() const;
  [[nodiscard]] bool active() const;
  [[nodiscard]] bool complete() const;
  [[nodiscard]] std::optional<AnimationTransitionSnapshot> progress() const;
  [[nodiscard]] bool cancel();

 private:
  friend AnimationTransitionHandle start_animation_transition(
      WindowRuntime&,
      AnimationTransition,
      AnimationOptions,
      AnimationTransitionCallback);
  friend AnimationTransitionHandle start_animation_transition(
      const WindowRuntimeContext&,
      AnimationTransition,
      AnimationOptions,
      AnimationTransitionCallback);
  friend AnimationTransitionHandle start_animation_transition(
      const AsyncContextCapability&,
      AnimationTransition,
      AnimationOptions,
      AnimationTransitionCallback);

  AnimationTransitionHandle(
      AnimationHandle animation,
      AnimationTransition transition);

  AnimationHandle animation_;
  AnimationTransition transition_;
};

[[nodiscard]] AnimationTransitionHandle start_animation_transition(
    WindowRuntime& runtime,
    AnimationTransition transition,
    AnimationOptions options,
    AnimationTransitionCallback callback);
[[nodiscard]] AnimationTransitionHandle start_animation_transition(
    const WindowRuntimeContext& context,
    AnimationTransition transition,
    AnimationOptions options,
    AnimationTransitionCallback callback);
[[nodiscard]] AnimationTransitionHandle start_animation_transition(
    const AsyncContextCapability& context,
    AnimationTransition transition,
    AnimationOptions options,
    AnimationTransitionCallback callback);

} // namespace cgpui
