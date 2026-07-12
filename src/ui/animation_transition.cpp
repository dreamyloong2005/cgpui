#include "cgpui/ui/animation_transition.hpp"

#include "cgpui/ui/async_context.hpp"
#include "cgpui/ui/runtime.hpp"

#include <utility>

namespace cgpui {
namespace {

AnimationTransitionSnapshot transition_snapshot(
    AnimationTransition transition,
    const AnimationSnapshot& animation) {
  return AnimationTransitionSnapshot{
      .animation = animation,
      .from = transition.from,
      .to = transition.to,
      .value = transition.value_at(animation.eased_progress),
  };
}

AnimationCallback wrap_transition_callback(
    AnimationTransition transition,
    AnimationTransitionCallback callback) {
  if (!callback) return {};
  return [transition, callback = std::move(callback)](
             const WindowRuntimeContext& context,
             const AnimationSnapshot& animation) {
    callback(context, transition_snapshot(transition, animation));
  };
}

} // namespace

float AnimationTransition::value_at(float eased_progress) const {
  return tween(from, to, eased_progress);
}

AnimationTransitionHandle::AnimationTransitionHandle(
    AnimationHandle animation,
    AnimationTransition transition)
    : animation_(std::move(animation)), transition_(transition) {}

AnimationId AnimationTransitionHandle::id() const {
  return animation_.id();
}

bool AnimationTransitionHandle::active() const {
  return animation_.active();
}

bool AnimationTransitionHandle::complete() const {
  return animation_.complete();
}

bool AnimationTransitionHandle::cancelled() const {
  return animation_.cancelled();
}

std::optional<AnimationTransitionSnapshot>
AnimationTransitionHandle::progress() const {
  const std::optional<AnimationSnapshot> animation = animation_.progress();
  if (!animation.has_value()) return std::nullopt;
  return transition_snapshot(transition_, *animation);
}

bool AnimationTransitionHandle::cancel() {
  return animation_.cancel();
}

AnimationTransitionHandle start_animation_transition(
    WindowRuntime& runtime,
    AnimationTransition transition,
    AnimationOptions options,
    AnimationTransitionCallback callback) {
  AnimationHandle animation = runtime.start_animation(
      options, wrap_transition_callback(transition, std::move(callback)));
  return AnimationTransitionHandle(std::move(animation), transition);
}

AnimationTransitionHandle start_animation_transition(
    const WindowRuntimeContext& context,
    AnimationTransition transition,
    AnimationOptions options,
    AnimationTransitionCallback callback) {
  AnimationHandle animation = context.start_animation(
      options, wrap_transition_callback(transition, std::move(callback)));
  return AnimationTransitionHandle(std::move(animation), transition);
}

AnimationTransitionHandle start_animation_transition(
    const AsyncContextCapability& context,
    AnimationTransition transition,
    AnimationOptions options,
    AnimationTransitionCallback callback) {
  AnimationHandle animation = context.start_animation(
      options, wrap_transition_callback(transition, std::move(callback)));
  return AnimationTransitionHandle(std::move(animation), transition);
}

} // namespace cgpui
