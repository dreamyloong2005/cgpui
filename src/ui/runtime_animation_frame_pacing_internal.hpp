TimerId animation_frame_timer_id_;
std::uint64_t animation_frame_timer_deadline_ms_ = 0;
std::uint64_t element_animation_frame_deadline_ms_ = 0;
bool element_animation_frame_pending_ = false;
bool delivering_animation_frame_ = false;
AnimationFramePacingSnapshot animation_frame_pacing_diagnostics_;
