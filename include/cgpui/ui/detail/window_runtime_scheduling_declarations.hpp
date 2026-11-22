// Declaration fragment included inside WindowRuntime. No runtime state.
void handle_redraw(), schedule_redraw(), flush_deferred_redraw_request();
void request_platform_wakeup();
void handle_wakeup();
void drain_deferred_callbacks();
void fire_due_timers();
void sync_platform_time();
void schedule_next_timer_wakeup();
