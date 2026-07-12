#include "element_animation_state_internal.hpp"

namespace cgpui {
namespace {

ElementAnimationSnapshot sequence_snapshot(
    std::uint64_t scope_id,
    const ElementKey& key,
    const ElementAnimationStage& stage,
    std::size_t stage_index,
    std::uint64_t iteration,
    std::uint64_t elapsed_ms,
    float linear_progress,
    bool mounted,
    bool complete) {
  return ElementAnimationSnapshot{
      .scope_id = scope_id,
      .key = key,
      .elapsed_ms = elapsed_ms,
      .duration_ms = stage.animation.duration_ms,
      .linear_progress = linear_progress,
      .eased_progress = ease(stage.animation.easing, linear_progress),
      .easing = stage.animation.easing,
      .stage_index = stage_index,
      .iteration = iteration,
      .repeating = stage.repeating,
      .mounted = mounted,
      .complete = complete,
  };
}

} // namespace

ElementAnimationSnapshot ElementAnimationStateStore::resolve(
    const ElementKey& key,
    std::span<const ElementAnimationStage> stages) {
  const std::uint64_t scope_id = impl_->active_scope_id;
  const auto frame = impl_->frames.find(scope_id);
  if (frame == impl_->frames.end() || stages.empty()) {
    return {.scope_id = scope_id, .key = key, .complete = true};
  }

  const detail::ScopedElementAnimationKey scoped_key{
      .scope_id = scope_id,
      .value = key.value,
  };
  auto [record, inserted] = impl_->records.try_emplace(
      scoped_key,
      detail::ElementAnimationRecord{
          .options = stages.front().animation,
          .started_ms = frame->second.now_ms,
          .seen_generation = frame->second.generation,
          .repeating = stages.front().repeating,
      });
  if (record->second.stage_index >= stages.size()) {
    record->second = detail::ElementAnimationRecord{
        .options = stages.front().animation,
        .started_ms = frame->second.now_ms,
        .seen_generation = frame->second.generation,
        .repeating = stages.front().repeating,
    };
    inserted = true;
  }
  record->second.seen_generation = frame->second.generation;

  const std::size_t stage_index = record->second.stage_index;
  const ElementAnimationStage& stage = stages[stage_index];
  record->second.options = stage.animation;
  record->second.repeating = stage.repeating;
  if (record->second.complete) {
    return sequence_snapshot(
        scope_id, key, stage, stage_index, record->second.iteration,
        stage.animation.duration_ms, 1.0F, inserted, true);
  }

  const std::uint64_t duration_ms = stage.animation.duration_ms;
  std::uint64_t elapsed_ms =
      frame->second.now_ms >= record->second.started_ms
          ? frame->second.now_ms - record->second.started_ms
          : 0;
  if (stage.repeating && duration_ms != 0 && elapsed_ms > duration_ms) {
    const std::uint64_t completed_iterations = elapsed_ms / duration_ms;
    record->second.iteration += completed_iterations;
    record->second.started_ms += completed_iterations * duration_ms;
    elapsed_ms %= duration_ms;
  }

  if (!stage.repeating &&
      (duration_ms == 0 || elapsed_ms > duration_ms)) {
    const bool sequence_complete = stage_index + 1 >= stages.size();
    const ElementAnimationSnapshot snapshot = sequence_snapshot(
        scope_id, key, stage, stage_index, record->second.iteration,
        duration_ms, 1.0F, inserted, sequence_complete);
    if (sequence_complete) {
      record->second.complete = true;
    } else {
      record->second.stage_index += 1;
      record->second.iteration = 0;
      record->second.started_ms = frame->second.now_ms;
      record->second.options = stages[record->second.stage_index].animation;
      record->second.repeating = stages[record->second.stage_index].repeating;
    }
    return snapshot;
  }

  if (stage.repeating && duration_ms == 0) {
    record->second.complete = true;
    return sequence_snapshot(
        scope_id, key, stage, stage_index, record->second.iteration,
        0, 1.0F, inserted, true);
  }

  const float linear_progress =
      duration_ms == 0
          ? 1.0F
          : clamp_animation_progress(
                static_cast<float>(elapsed_ms) /
                static_cast<float>(duration_ms));
  return sequence_snapshot(
      scope_id, key, stage, stage_index, record->second.iteration,
      elapsed_ms, linear_progress, inserted, false);
}

} // namespace cgpui
