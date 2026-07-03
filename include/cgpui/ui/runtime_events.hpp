#pragma once

#include "cgpui/ui/runtime_actions.hpp"
#include "cgpui/ui/runtime_ids.hpp"

#include <optional>
#include <vector>

namespace cgpui {

struct EventRoute {
  ViewId target_view_id;
  std::optional<ElementId> target_element_id;
  std::vector<ElementId> element_ancestry;
  std::vector<ViewId> view_ancestry;
  EventKind event_kind = EventKind::unknown;
};

class EventRouter {
 public:
  [[nodiscard]] static EventRoute route_to_root(
      const PlatformEvent& event,
      ViewId root_view_id);
};

struct EventDispatchRecord {
  int sequence = 0;
  ViewId view_id;
  EventKind event_kind = EventKind::unknown;
  EventRoute route;
  EventResult result;
};

} // namespace cgpui
