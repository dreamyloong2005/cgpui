// Declaration fragment included inside WindowRuntime. No runtime state.
void remove_subscriptions_for_view(ViewId view_id);
void remove_observers_for_view(ViewId view_id);
void notify_window_observers();
void notify_view_observers(ViewId view_id);
[[nodiscard]] bool notify_entity_changed(
    std::type_index entity_type,
    std::uint64_t entity_id_value);
