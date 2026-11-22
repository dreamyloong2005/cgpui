// Declaration fragment included inside WindowRuntime. No runtime state.
void apply_text_input_for_event(const PlatformEvent& event);
void apply_focused_text_ime_placement();
[[nodiscard]] bool apply_text_pointer_selection(
    const PlatformEvent& event,
    const EventRoute& route);
[[nodiscard]] std::optional<std::size_t> text_offset_for_point(
    const TextElement& element,
    Point point) const;
[[nodiscard]] TextInputElement* routed_text_input(ElementId element_id);
[[nodiscard]] const TextInputElement* routed_text_input(
    ElementId element_id) const;
