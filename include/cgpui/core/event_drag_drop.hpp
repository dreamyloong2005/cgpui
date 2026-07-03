#pragma once

#include "cgpui/core/geometry.hpp"

#include <string>
#include <vector>

namespace cgpui {

enum class DragDropPayloadKind {
  none,
  text,
  files,
};

enum class DragDropAction {
  none,
  copy,
  move,
};

struct DragDropPayload {
  DragDropPayloadKind kind = DragDropPayloadKind::none;
  std::string text;
  std::vector<std::string> files;
};

struct DragEntered {
  Point position;
  DragDropPayload payload;
  DragDropAction action = DragDropAction::none;
};

struct DragUpdated {
  Point position;
  DragDropPayload payload;
  DragDropAction action = DragDropAction::none;
};

struct DragDropped {
  Point position;
  DragDropPayload payload;
  DragDropAction action = DragDropAction::none;
};

struct DragExited {
  Point position;
  DragDropPayload payload;
  DragDropAction action = DragDropAction::none;
};

} // namespace cgpui
