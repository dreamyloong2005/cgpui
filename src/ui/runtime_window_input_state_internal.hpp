void update_input_state_for_event(const PlatformEvent& event);
void update_input_state_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event);
void sync_root_input_record();
void capture_pointer_for_window(
    WindowRuntimeId runtime_id,
    PointerCaptureOwner owner);
void release_pointer_for_window(
    WindowRuntimeId runtime_id,
    PointerCaptureOwner owner);
void request_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ViewId view_id);
void request_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id);
void release_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ViewId view_id);
void release_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id);
