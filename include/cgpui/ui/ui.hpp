#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <span>
#include <vector>

namespace cgpui {

struct PaintCommand {
  SolidRect solid_rect;
};

class PaintList {
 public:
  void clear();
  void fill_rect(Rect rect, Color color);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
};

class View {
 public:
  virtual ~View() = default;
  virtual void paint(PaintList& paint_list, Size viewport_size) = 0;
};

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size);

} // namespace cgpui
