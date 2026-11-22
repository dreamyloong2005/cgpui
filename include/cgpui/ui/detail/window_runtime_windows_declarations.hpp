// Declaration fragment included inside WindowRuntime. No runtime state.
void activate_native_window_for_record(WindowRuntimeRecord& record), activate_pending_native_windows();
[[nodiscard]] Result<Renderer*> try_create_renderer(const RenderSurfaceDescriptor& descriptor, std::string_view empty_renderer_message);
[[nodiscard]] WindowRuntimeContext context_for_record(
    const WindowRuntimeRecord& record);
void dispatch_view_event_for_record(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event);
void handle_native_additional_window_event(
    WindowRuntimeId runtime_id,
    const PlatformEvent& event);
void cleanup_closed_additional_window(WindowRuntimeRecord& record);
void deactivate_native_additional_windows();
[[nodiscard]] WindowRuntimeId allocate_window_runtime_id();
[[nodiscard]] WindowRuntimeRecord* find_window_runtime_record(
    WindowRuntimeId runtime_id);
[[nodiscard]] const WindowRuntimeRecord* find_window_runtime_record(
    WindowRuntimeId runtime_id) const;
