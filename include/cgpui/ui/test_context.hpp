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
#include <string_view>

namespace cgpui {

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

 private:
  const WindowRuntimeContext* context_ = nullptr;
};

} // namespace cgpui
