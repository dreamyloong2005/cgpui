#pragma once

#include "cgpui/ui/paint.hpp"

namespace cgpui {

struct WindowRuntimeContext;
using ViewContext = WindowRuntimeContext;

template <typename T>
using Context = ViewContext;

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
