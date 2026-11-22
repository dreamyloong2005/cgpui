// Declaration fragment included inside WindowRuntime. No runtime state.
[[nodiscard]] WindowRuntimeId normalize_window_context_id(WindowRuntimeId id) const;
DeferredCallback bind_window_callback(WindowRuntimeId id, DeferredCallback callback);
AsyncIoCompletionCallback bind_window_io_callback(WindowRuntimeId id, AsyncIoCompletionCallback callback);
AnimationCallback bind_window_animation_callback(WindowRuntimeId id, AnimationCallback callback);
void invoke_window_callback(WindowRuntimeId id, const DeferredCallback& callback);
TimerId schedule_window_timer(WindowRuntimeId id, std::uint64_t delay, bool repeating, TimerCallback callback);
void cancel_window_timers(WindowRuntimeId id);
