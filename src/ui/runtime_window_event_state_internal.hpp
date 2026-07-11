void begin_event_route_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event);
void finish_event_dispatch_for_record(
    WindowRuntimeRecord& record,
    EventResult result);
void sync_root_event_record();
void reset_event_state_for_record(WindowRuntimeRecord& record);
void record_lifecycle_event_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event);
