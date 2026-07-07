friend class AnimationHandle; friend class TestContextCapability; friend class TaskHandle;
struct RuntimeTaskDiagnostics;
void handle_event(const PlatformEvent& event);
[[nodiscard]] bool handle_window_control_event(const PlatformEvent& event);
void update_input_state_for_event(const PlatformEvent& event);
[[nodiscard]] std::optional<ElementId> hit_test_target_for_event(
    const PlatformEvent& event);
void update_hover_cursor_for_event(
    const PlatformEvent& event,
    std::optional<ElementId> hit_element_id);
void request_style_state_invalidation(std::optional<ElementId> previous, std::optional<ElementId> next);
void update_active_state_for_event(const PlatformEvent& event);
void refresh_disabled_interaction_state();
void resolve_event_route_target(const PlatformEvent& event, std::optional<ElementId> hit_element_id);
void apply_focus_activation_for_event(const PlatformEvent& event);
void apply_keyboard_bindings_for_event(const PlatformEvent& event);
[[nodiscard]] bool dispatch_key_binding_for_event(const KeyboardKey& key);
[[nodiscard]] bool key_binding_context_active(
    const KeyBindingContext& context) const;
void apply_text_input_for_event(const PlatformEvent& event);
[[nodiscard]] EventResult dispatch_current_event_route(
    const PlatformEvent& event);
void finish_event_dispatch(EventResult result);
void handle_resize(const WindowResized& event);
void handle_redraw(), schedule_redraw(), flush_deferred_redraw_request();
void request_platform_wakeup();
void handle_wakeup();
void drain_deferred_callbacks();
void fire_due_timers();
void tick_animation(AnimationId id);
void update_platform_accessibility_tree();
[[nodiscard]] PlatformAccessibilityTreeUpdate build_platform_accessibility_update()
    const;
[[nodiscard]] bool task_active(TaskId id) const;
[[nodiscard]] bool task_complete(TaskId id) const;
[[nodiscard]] bool task_cancelled(TaskId id) const;
[[nodiscard]] bool cancel_task(TaskId id);
[[nodiscard]] bool animation_active(AnimationId id) const;
[[nodiscard]] bool animation_complete(AnimationId id) const;
void apply_cursor_shape(CursorShape cursor_shape);
void apply_focused_text_ime_placement();
void record_platform_diagnostic(PlatformDiagnosticEvent event);
void fail_and_quit(Error error);
void activate_native_window_for_record(WindowRuntimeRecord& record);
[[nodiscard]] Result<Renderer*> try_create_renderer(const RenderSurfaceDescriptor& descriptor, std::string_view empty_renderer_message);
[[nodiscard]] Result<void> try_draw_frame_for_record(WindowRuntimeRecord& record, View& view);
[[nodiscard]] RuntimeTaskDiagnostics task_diagnostics() const;
[[nodiscard]] WindowRuntimeContext context_for_record(
    const WindowRuntimeRecord& record);
void handle_redraw_for_record(WindowRuntimeRecord& record, View& view);
void dispatch_view_event_for_record(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event);
void record_lifecycle_event_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event);
void handle_native_additional_window_event(
    WindowRuntimeId runtime_id,
    const PlatformEvent& event);
void cleanup_closed_additional_window(WindowRuntimeRecord& record);
void remove_subscriptions_for_view(ViewId view_id);
void remove_observers_for_view(ViewId view_id);
void notify_window_observers();
void notify_view_observers(ViewId view_id);
void deactivate_native_additional_windows();
void upsert_action_registration(ActionRegistration registration);
[[nodiscard]] std::optional<KeyBinding> command_palette_key_binding(const CommandPaletteEntry& entry) const;
[[nodiscard]] std::optional<bool> action_registration_enabled(
    std::string_view name,
    ActionScope dispatch_scope,
    std::optional<ViewId> view_id,
    std::optional<ElementId> element_id) const;
[[nodiscard]] WindowRuntimeId allocate_window_runtime_id();
[[nodiscard]] WindowRuntimeRecord* find_window_runtime_record(
    WindowRuntimeId runtime_id);
[[nodiscard]] const WindowRuntimeRecord* find_window_runtime_record(
    WindowRuntimeId runtime_id) const;
void refresh_route_ancestry(EventRoute& route) const;
[[nodiscard]] std::vector<ElementId> element_ancestry_for(
    ElementId element_id) const;
[[nodiscard]] std::vector<ViewId> view_ancestry_for(ViewId view_id) const;
[[nodiscard]] std::optional<ViewId> child_view_target_for(
    ElementId element_id) const;
[[nodiscard]] std::vector<ElementId> ordered_focusable_element_ids() const;
[[nodiscard]] bool focus_next_element(bool reverse);
[[nodiscard]] std::optional<ViewId> action_dispatch_view_id() const;
[[nodiscard]] ScrollState* scroll_state_for_route(const EventRoute& route);
void record_lifecycle_event(const PlatformEvent& event);
[[nodiscard]] bool apply_text_pointer_selection(
    const PlatformEvent& event,
    const EventRoute& route);
[[nodiscard]] std::optional<std::size_t> text_offset_for_point(
    const TextElement& element,
    Point point) const;
[[nodiscard]] TextInputElement* routed_text_input(ElementId element_id);
[[nodiscard]] const TextInputElement* routed_text_input(
    ElementId element_id) const;
[[nodiscard]] EventResult dispatch_routed_element_event(
    const PlatformEvent& event,
    const EventRoute& route);
[[nodiscard]] Element* routed_element(ElementId element_id);
[[nodiscard]] const Element* routed_element(ElementId element_id) const;
[[nodiscard]] WindowRuntimeContext context();
template <typename T>
[[nodiscard]] EntityStore<T>& entity_store();
template <typename T>
[[nodiscard]] const EntityStore<T>* find_entity_store() const;
template <typename T>
[[nodiscard]] EntityStore<T>* find_entity_store();
[[nodiscard]] bool notify_entity_changed(
    std::type_index entity_type,
    std::uint64_t entity_id_value);
struct RegisteredView {
  View* view = nullptr;
  std::unique_ptr<View> owned_view;
};
struct RuntimeTimer {
  TimerId id;
  std::uint64_t due_ms = 0;
  std::uint64_t interval_ms = 0;
  bool repeating = false;
  TimerCallback callback;
};

struct RuntimeAnimation {
  AnimationId id;
  AnimationOptions options;
  AnimationCallback callback;
  TimerId timer_id;
  std::uint64_t started_ms = 0;
  std::uint64_t last_tick_ms = 0;
  bool complete = false;
};

struct RuntimeTask {
  TaskId id;
  TaskCompletionCallback callback;
  bool queued = false;
  bool completed = false;
  bool cancelled = false;
  bool background = false;
  std::shared_ptr<std::atomic_bool> cancellation_requested;
  std::jthread worker;
};

struct RuntimeTaskDiagnostics {
  std::size_t task_count = 0;
  std::size_t active_task_count = 0;
  std::size_t queued_task_count = 0;
  std::size_t completed_task_count = 0;
  std::size_t cancelled_task_count = 0;
  std::size_t background_task_count = 0;
};

struct TextPointerSelectionDrag {
  ElementId element_id;
  std::size_t anchor_offset = 0;
};

PlatformApplication& application_;
View& view_;
RendererFactory renderer_factory_;
WindowRuntimeFrameCallback after_frame_callback_;
WindowRuntimeRenderCallback after_render_callback_;
WindowRuntimeEventCallback after_event_callback_;
WindowRuntimeFrameCallback close_requested_callback_;
WindowRuntimeErrorCallback error_callback_;
PlatformWindow* window_ = nullptr;
Renderer* renderer_ = nullptr;
Size framebuffer_size_{};
Size viewport_size_{};
DpiScale scale_{};
ViewInputState input_{};
std::optional<PointerCaptureOwner> pointer_capture_owner_;
std::optional<ViewId> keyboard_focus_owner_;
std::optional<ElementId> keyboard_focus_element_owner_;
std::optional<TextPointerSelectionDrag> text_pointer_selection_drag_;
std::optional<ElementId> hovered_element_id_;
std::optional<ElementId> active_element_id_;
CursorShape cursor_shape_ = CursorShape::default_arrow;
CursorShape applied_cursor_shape_ = CursorShape::default_arrow;
EventResult last_event_result_{};
std::optional<RenderRecord> last_render_record_;
std::optional<FrameStatistics> last_frame_statistics_;
std::optional<EventDispatchRecord> last_event_dispatch_;
std::optional<ActionDispatchResult> last_action_dispatch_;
std::optional<EventRoute> current_event_route_;
std::unique_ptr<ElementTree> owned_element_tree_;
const Element* element_root_ = nullptr;
ViewId root_view_id_{1};
std::uint64_t next_view_id_ = 2;
std::unordered_map<std::uint64_t, RegisteredView> view_registry_;
std::unordered_set<std::uint64_t> removed_view_ids_;
int event_dispatch_sequence_ = 0;
int render_sequence_ = 0;
int frame_index_ = 0;
std::unordered_map<std::type_index, std::any> globals_;
std::unordered_map<std::type_index, std::any> entity_stores_;
std::size_t entity_count_ = 0;
std::unordered_map<std::string, ActionHandler> action_handlers_;
std::unordered_map<std::string, ActionHandler> window_action_handlers_;
std::unordered_map<
    std::uint64_t,
    std::unordered_map<std::string, ActionHandler>>
    view_action_handlers_;
std::unordered_map<
    std::uint64_t,
    std::unordered_map<std::string, ActionHandler>>
    focused_element_action_handlers_;
std::vector<ActionRegistration> action_registrations_;
std::vector<CommandPaletteEntry> command_palette_entries_;
std::vector<KeyBinding> key_bindings_;
std::vector<KeyBindingChord> pending_key_binding_sequence_;
std::vector<TextEditBinding> text_edit_bindings_;
std::unordered_map<std::uint64_t, TextModel*> text_models_;
Clipboard* clipboard_ = nullptr;
std::unordered_map<std::uint64_t, CursorShape> element_cursors_;
std::optional<ImeTextInputPlacement> applied_ime_text_input_placement_;
std::optional<PlatformAccessibilityTreeUpdate>
    last_platform_accessibility_update_;
std::vector<PlatformDiagnosticEvent> platform_diagnostics_;
int platform_diagnostic_sequence_ = 0;
std::vector<EntitySubscription> entity_subscriptions_;
std::vector<EntityObserver> entity_observers_;
std::vector<WindowObserver> window_observers_;
std::vector<ViewObserver> view_observers_;
std::uint64_t next_subscription_id_ = 1;
std::vector<DeferredCallback> deferred_callbacks_;
std::vector<RuntimeTimer> timers_;
std::uint64_t next_timer_id_ = 1;
std::uint64_t current_time_ms_ = 0;
bool firing_timers_ = false;
std::vector<RuntimeAnimation> animations_;
std::uint64_t next_animation_id_ = 1;
mutable std::mutex tasks_mutex_;
std::vector<RuntimeTask> tasks_;
std::vector<TaskId> task_completion_queue_;
std::uint64_t next_task_id_ = 1;
bool draining_task_completions_ = false;
bool handling_wakeup_ = false;
int update_batch_depth_ = 0;
std::vector<AppOpenedWindow> app_opened_windows_;
WindowRuntimeId root_window_runtime_id_{1};
std::uint64_t next_window_runtime_id_ = 2;
std::vector<WindowRuntimeRecord> window_runtime_records_;
std::vector<std::unique_ptr<PlatformWindow>> native_additional_windows_;
NativeMenuInstallation native_menu_installation_;
NativeFileDialogResult native_file_dialog_result_;
Theme app_theme_;
std::unordered_map<std::uint64_t, Theme> window_themes_;
mutable std::vector<EntitySubscription> subscription_query_buffer_;
InvalidationState invalidation_state_;
bool dispatching_view_event_ = false;
bool draining_deferred_callbacks_ = false;
bool redraw_scheduled_ = false;
bool deferred_redraw_request_ = false;
bool should_quit_ = false;
bool failed_ = false;
