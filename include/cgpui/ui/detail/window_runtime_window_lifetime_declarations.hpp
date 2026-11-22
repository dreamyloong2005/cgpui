// Declaration fragment included inside WindowRuntime. No runtime state.
WindowRuntime(PlatformApplication& application, View& view, AppRendererFactory factory, OwnedRendererFactoryTag);
void collect_retired_native_windows();
void reset_root_window_resources();
void retire_window_view(ViewId view_id);
void close_window_record_after_event(WindowRuntimeRecord& record, const PlatformEvent& event);
