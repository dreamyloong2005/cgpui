// Declaration fragment included inside WindowRuntime. No runtime state.
void handle_event(const PlatformEvent& event);
[[nodiscard]] bool handle_window_control_event(const PlatformEvent& event);
void handle_pointer_capture_changed(const PointerCaptureChanged& changed);
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
[[nodiscard]] EventResult dispatch_current_event_route(
    const PlatformEvent& event);
void finish_event_dispatch(EventResult result);
void apply_cursor_shape(CursorShape cursor_shape);
void refresh_route_ancestry(EventRoute& route) const;
[[nodiscard]] std::vector<ElementId> element_ancestry_for(
    ElementId element_id) const;
[[nodiscard]] std::vector<ViewId> view_ancestry_for(ViewId view_id) const;
[[nodiscard]] std::optional<ViewId> child_view_target_for(
    ElementId element_id) const;
[[nodiscard]] std::vector<ElementId> ordered_focusable_element_ids() const;
[[nodiscard]] bool focus_next_element(bool reverse);
[[nodiscard]] ScrollState* scroll_state_for_route(const EventRoute& route);
void record_lifecycle_event(const PlatformEvent& event);
[[nodiscard]] EventResult dispatch_routed_element_event(
    const PlatformEvent& event,
    const EventRoute& route);
[[nodiscard]] Element* routed_element(ElementId element_id);
[[nodiscard]] const Element* routed_element(ElementId element_id) const;
