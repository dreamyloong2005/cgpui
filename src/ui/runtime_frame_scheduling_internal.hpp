void begin_frame_scheduling(), complete_frame_scheduling();
void abort_frame_scheduling();
void set_root_redraw_scheduled(bool scheduled);

bool rendering_frame_ = false;
bool next_frame_redraw_requested_ = false;
