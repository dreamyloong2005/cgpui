#include "element_animation_state_internal.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace cgpui {
namespace detail {

std::size_t ScopedElementAnimationKeyHash::operator()(
    const ScopedElementAnimationKey& key) const {
  const std::size_t scope_hash = std::hash<std::uint64_t>{}(key.scope_id);
  const std::size_t value_hash = std::hash<std::string>{}(key.value);
  return scope_hash ^ (value_hash + 0x9e3779b9U + (scope_hash << 6U) +
                       (scope_hash >> 2U));
}

std::uint64_t normalized_animation_tick_interval(AnimationOptions options) {
  return options.tick_interval_ms == 0 ? 16 : options.tick_interval_ms;
}

} // namespace detail

bool ElementAnimationFrameResult::requests_next_frame() const {
  return active_count != 0;
}

ElementAnimationStateStore::ElementAnimationStateStore()
    : impl_(std::make_unique<Impl>()) {}

ElementAnimationStateStore::~ElementAnimationStateStore() = default;
ElementAnimationStateStore::ElementAnimationStateStore(
    ElementAnimationStateStore&&) noexcept = default;
ElementAnimationStateStore& ElementAnimationStateStore::operator=(
    ElementAnimationStateStore&&) noexcept = default;

void ElementAnimationStateStore::begin_frame(
    std::uint64_t scope_id,
    std::uint64_t now_ms) {
  detail::ElementAnimationFrame& frame = impl_->frames[scope_id];
  frame.now_ms = now_ms;
  frame.generation += 1;
  impl_->active_scope_id = scope_id;
}

ElementAnimationSnapshot ElementAnimationStateStore::resolve(
    const ElementKey& key,
    AnimationOptions options) {
  const std::uint64_t scope_id = impl_->active_scope_id;
  const auto frame = impl_->frames.find(scope_id);
  if (frame == impl_->frames.end()) {
    return {.scope_id = scope_id, .key = key};
  }

  const detail::ScopedElementAnimationKey scoped_key{
      .scope_id = scope_id,
      .value = key.value,
  };
  auto [record, inserted] = impl_->records.try_emplace(
      scoped_key,
      detail::ElementAnimationRecord{
          .options = options,
          .started_ms = frame->second.now_ms,
          .seen_generation = frame->second.generation,
      });
  record->second.options = options;
  record->second.seen_generation = frame->second.generation;
  record->second.stage_index = 0;
  record->second.iteration = 0;
  record->second.repeating = false;

  const std::uint64_t duration_ms = options.duration_ms;
  const std::uint64_t raw_elapsed_ms =
      frame->second.now_ms >= record->second.started_ms
          ? frame->second.now_ms - record->second.started_ms
          : 0;
  const std::uint64_t elapsed_ms =
      duration_ms == 0 ? 0 : std::min(raw_elapsed_ms, duration_ms);
  const float linear_progress =
      duration_ms == 0
          ? 1.0F
          : clamp_animation_progress(
                static_cast<float>(elapsed_ms) /
                static_cast<float>(duration_ms));
  record->second.complete = linear_progress >= 1.0F;

  return ElementAnimationSnapshot{
      .scope_id = scope_id,
      .key = key,
      .elapsed_ms = elapsed_ms,
      .duration_ms = duration_ms,
      .linear_progress = linear_progress,
      .eased_progress = ease(options.easing, linear_progress),
      .easing = options.easing,
      .stage_index = 0,
      .iteration = 0,
      .repeating = false,
      .mounted = inserted,
      .complete = record->second.complete,
  };
}

ElementAnimationFrameResult ElementAnimationStateStore::finish_frame(
    std::uint64_t scope_id) {
  ElementAnimationFrameResult result;
  const auto frame = impl_->frames.find(scope_id);
  if (frame == impl_->frames.end()) return result;

  result.next_frame_delay_ms = std::numeric_limits<std::uint64_t>::max();
  for (auto record = impl_->records.begin(); record != impl_->records.end();) {
    if (record->first.scope_id != scope_id) {
      ++record;
      continue;
    }
    if (record->second.seen_generation != frame->second.generation) {
      record = impl_->records.erase(record);
      result.unmounted_count += 1;
      continue;
    }

    if (record->second.complete) {
      result.completed_count += 1;
    } else {
      result.active_count += 1;
      result.next_frame_delay_ms = std::min(
          result.next_frame_delay_ms,
          detail::normalized_animation_tick_interval(record->second.options));
    }
    ++record;
  }
  if (result.active_count == 0) result.next_frame_delay_ms = 0;
  return result;
}

bool ElementAnimationStateStore::contains(
    std::uint64_t scope_id,
    const ElementKey& key) const {
  return impl_->records.contains(detail::ScopedElementAnimationKey{
      .scope_id = scope_id,
      .value = key.value,
  });
}

std::size_t ElementAnimationStateStore::size() const {
  return impl_->records.size();
}

} // namespace cgpui
