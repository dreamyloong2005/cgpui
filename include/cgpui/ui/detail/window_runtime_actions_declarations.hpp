// Declaration fragment included inside WindowRuntime. No runtime state.
void upsert_action_registration(ActionRegistration registration);
[[nodiscard]] std::optional<KeyBinding> command_palette_key_binding(const CommandPaletteEntry& entry) const;
[[nodiscard]] std::optional<bool> action_registration_enabled(
    std::string_view name,
    ActionScope dispatch_scope,
    std::optional<ViewId> view_id,
    std::optional<ElementId> element_id) const;
[[nodiscard]] std::optional<ViewId> action_dispatch_view_id() const;
