#pragma once

#include "cgpui/ui/runtime_events.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace cgpui {

struct InvalidationState {
  bool render = false;
  bool layout = false;
  bool paint = false;
};

struct FrameStatistics {
  int frame_index = 0;
  std::size_t render_pass_count = 0;
  std::size_t layout_pass_count = 0;
  std::size_t paint_pass_count = 0;
  std::size_t paint_command_count = 0;
  std::size_t submitted_command_count = 0;
  std::size_t skipped_command_count = 0;
  std::size_t solid_rect_command_count = 0;
  std::size_t rounded_rect_command_count = 0;
  std::size_t text_command_count = 0;
  std::size_t text_selection_command_count = 0;
  std::size_t text_caret_command_count = 0;
  std::size_t image_command_count = 0;
  std::size_t clip_stack_command_count = 0;
  std::size_t max_clip_stack_depth = 0;
  std::size_t composition_stack_command_count = 0;
  std::size_t max_composition_stack_depth = 0;
  std::size_t begin_frame_count = 0;
  std::size_t clear_count = 0;
  std::size_t present_count = 0;
  double frame_time_ms = 0.0;
  double render_time_ms = 0.0;
  double layout_time_ms = 0.0;
  double paint_time_ms = 0.0;
};

struct RenderRecord {
  int sequence = 0;
  ViewId view_id;
  Size viewport_size;
  std::optional<ElementId> root_element_id;
  std::optional<FrameStatistics> statistics;
};

struct RuntimeDiagnosticsSnapshot {
  std::size_t entity_store_count = 0;
  std::size_t entity_count = 0;
  std::size_t view_entity_subscription_count = 0;
  std::size_t entity_observer_count = 0;
  std::size_t connected_subscription_count = 0;
  InvalidationState invalidation;
  int frame_index = 0;
  std::optional<RenderRecord> last_render_record;
  std::optional<FrameStatistics> last_frame_statistics;
  std::vector<PlatformDiagnosticEvent> platform_diagnostics;
  std::size_t task_count = 0;
  std::size_t active_task_count = 0;
  std::size_t queued_task_count = 0;
  std::size_t completed_task_count = 0;
  std::size_t cancelled_task_count = 0;
  std::size_t background_task_count = 0;
};

struct EntitySubscription {
  ViewId view_id;
  std::type_index entity_type{typeid(void)};
  std::uint64_t entity_id_value = 0;
};

struct EntityObserver {
  SubscriptionId subscription_id;
  std::type_index entity_type{typeid(void)};
  std::uint64_t entity_id_value = 0;
  std::function<void(const WindowRuntimeContext&, std::uint64_t)> callback;
};

} // namespace cgpui
