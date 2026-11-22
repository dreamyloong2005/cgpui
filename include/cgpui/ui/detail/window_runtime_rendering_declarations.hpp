// Declaration fragment included inside WindowRuntime. No runtime state.
void handle_resize(const WindowResized& event);
void layout_element_tree_with_animations(FrameStatistics& frame_statistics);
[[nodiscard]] Result<void> try_draw_frame_for_record(WindowRuntimeRecord& record, View& view);
void handle_redraw_for_record(WindowRuntimeRecord& record, View& view);
