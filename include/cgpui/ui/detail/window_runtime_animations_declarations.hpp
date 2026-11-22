// Declaration fragment included inside WindowRuntime. No runtime state.
void tick_animation(AnimationId id);
[[nodiscard]] bool animation_active(AnimationId id) const;
[[nodiscard]] bool animation_complete(AnimationId id) const;
[[nodiscard]] bool animation_cancelled(AnimationId id) const;
