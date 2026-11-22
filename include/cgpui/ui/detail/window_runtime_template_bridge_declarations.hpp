// Declaration fragment included inside WindowRuntime. No runtime state.
template <typename T> [[nodiscard]] EntityStore<T>& entity_store();
template <typename T> [[nodiscard]] const EntityStore<T>* find_entity_store() const;
template <typename T> [[nodiscard]] EntityStore<T>* find_entity_store();
std::any& ensure_entity_store_value(std::type_index type, std::any (*factory)());
std::any* entity_store_value(std::type_index type);
const std::any* entity_store_value(std::type_index type) const;
void did_insert_entity();
void did_remove_entity();
void subscribe_view_to_entity_value(ViewId view, std::type_index type, std::uint64_t id);
void set_global_value(std::type_index type, std::any value);
std::any* global_value(std::type_index type);
const std::any* global_value(std::type_index type) const;
SubscriptionId append_entity_observer(std::type_index type, std::uint64_t id,
    std::function<void(const WindowRuntimeContext&, std::uint64_t)> callback, bool tracked);
SubscriptionId append_window_observer(WindowObservationCallback callback, bool tracked);
SubscriptionId append_view_observer(ViewId view, ViewObservationCallback callback, bool tracked);
ElementTree* mutable_element_tree();
std::shared_ptr<detail::CrossThreadEntityQueueState> cross_thread_entity_queue() const;
