// Which window an action dispatch runs in. While an additional window's
// event is handled (handle_native_additional_window_event), actions fired
// by its elements, views, key bindings, accessibility or menus resolve the
// focused element, view and handler context from that window's record;
// otherwise from the root window. Included inside WindowRuntime's private
// section.
struct ActionDispatchTarget {
  std::optional<ElementId> focus_owner;
  std::optional<ViewId> view_id;
  std::function<WindowRuntimeContext()> context;
};
std::optional<WindowRuntimeId> action_window_runtime_id_;
// Sets action_window_runtime_id_ for one event and restores the outer one.
class ActionWindowScope {
 public:
  ActionWindowScope(std::optional<WindowRuntimeId>& slot, WindowRuntimeId id)
      : slot_(slot), outer_(std::exchange(slot, id)) {}
  ActionWindowScope(const ActionWindowScope&) = delete;
  ActionWindowScope& operator=(const ActionWindowScope&) = delete;
  ~ActionWindowScope() { slot_ = outer_; }

 private:
  std::optional<WindowRuntimeId>& slot_;
  std::optional<WindowRuntimeId> outer_;
};
[[nodiscard]] ActionDispatchTarget action_dispatch_target();
