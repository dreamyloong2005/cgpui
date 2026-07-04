#pragma once

#include "cgpui/ui/paint.hpp"
#include "cgpui/ui/render.hpp"

namespace cgpui {

class View {
 public:
  virtual ~View() = default;
  virtual AnyElement render(ViewContext& context);
  virtual void paint(PaintList& paint_list, Size viewport_size) = 0;
  virtual EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
};

} // namespace cgpui
