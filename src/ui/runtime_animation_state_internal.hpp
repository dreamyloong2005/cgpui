struct RuntimeAnimation {
  AnimationId id;
  AnimationOptions options;
  AnimationCallback callback;
  TimerId timer_id;
  std::uint64_t started_ms = 0;
  std::uint64_t last_tick_ms = 0;
  std::uint64_t cancelled_elapsed_ms = 0;
  bool complete = false;
  bool cancelled = false;
};
std::vector<RuntimeAnimation> animations_;
std::uint64_t next_animation_id_ = 1;
std::optional<AnimationCancellationDiagnostic> last_animation_cancellation_;
ElementAnimationStateStore element_animation_state_store_;
TimerId element_animation_timer_id_;
