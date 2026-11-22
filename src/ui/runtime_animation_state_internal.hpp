std::vector<RuntimeAnimation> animations_;
std::uint64_t next_animation_id_ = 1;
std::optional<AnimationCancellationDiagnostic> last_animation_cancellation_;
ElementAnimationStateStore element_animation_state_store_;
