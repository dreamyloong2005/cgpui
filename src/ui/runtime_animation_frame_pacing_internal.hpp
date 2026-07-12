void schedule_animation_frame_wakeup(bool count_coalescing);
void deliver_animation_frame();
void request_element_animation_frame(std::uint64_t interval_ms);
void clear_element_animation_frame();
[[nodiscard]] std::uint64_t next_animation_frame_deadline(
    std::uint64_t deadline,
    std::uint64_t interval_ms) const;
[[nodiscard]] AnimationFramePacingSnapshot animation_frame_pacing_snapshot()
    const;
TimerId animation_frame_timer_id_;
std::uint64_t animation_frame_timer_deadline_ms_ = 0;
std::uint64_t element_animation_frame_deadline_ms_ = 0;
bool element_animation_frame_pending_ = false;
bool delivering_animation_frame_ = false;
AnimationFramePacingSnapshot animation_frame_pacing_diagnostics_;
