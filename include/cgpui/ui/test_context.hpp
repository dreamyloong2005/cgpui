#pragma once

#include "cgpui/ui/runtime_actions.hpp"
#include "cgpui/ui/runtime_diagnostics.hpp"
#include "cgpui/ui/runtime_events.hpp"
#include "cgpui/ui/runtime_handles.hpp"
#include "cgpui/ui/runtime_ids.hpp"
#include "cgpui/ui/runtime_input_state.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace cgpui {

class Clipboard;
struct WindowRuntimeContext;

class TestContextCapability {
 public:
  constexpr TestContextCapability() = default;
  constexpr explicit TestContextCapability(
      const WindowRuntimeContext& context)
      : context_(&context) {}

  [[nodiscard]] WindowRuntimeId runtime_id() const;
  [[nodiscard]] ViewId view_id() const;
  [[nodiscard]] int frame_index() const;
  [[nodiscard]] ViewInputState input_state() const;
  [[nodiscard]] std::optional<EventRoute> current_event_route() const;
  [[nodiscard]] std::optional<ActionDispatchResult> last_action_dispatch()
      const;

  [[nodiscard]] InvalidationState invalidation_state() const;
  void clear_invalidation() const;
  [[nodiscard]] RuntimeDiagnosticsSnapshot diagnostics_snapshot() const;
  [[nodiscard]] std::span<const PlatformDiagnosticEvent>
  platform_diagnostics() const;

  void advance_time(std::uint64_t delta_ms) const;
  void run_until_parked() const;
  void request_redraw() const;
  void draw_frame() const;
  void advance_time_until_parked(std::uint64_t delta_ms) const;
  [[nodiscard]] bool cancel_timer(TimerId id) const;
  [[nodiscard]] bool complete_task(TaskId id) const;
  void drain_task_completions() const;
  void dispatch_keystroke(KeyboardKey key) const;
  [[nodiscard]] bool simulate_keystrokes(std::string_view keystrokes) const;
  void dispatch_pointer_move(Point position) const;
  void dispatch_pointer_button(
      MouseButton button,
      bool pressed,
      Point position) const;
  void dispatch_pointer_scroll(Point delta, Point position) const;
  void dispatch_window_activation(bool active) const;
  void dispatch_window_focus(bool focused) const;
  void focus(ElementId element_id) const;
  void release_focus(ElementId element_id) const;
  void set_clipboard(Clipboard* clipboard) const;
  [[nodiscard]] bool write_to_clipboard(std::string_view text) const;
  [[nodiscard]] std::optional<std::string> read_from_clipboard() const;
  [[nodiscard]] bool paste_clipboard_text() const;
  [[nodiscard]] bool copy_selection_to_clipboard() const;
  [[nodiscard]] bool cut_selection_to_clipboard() const;

 private:
  const WindowRuntimeContext* context_ = nullptr;
};

} // namespace cgpui
