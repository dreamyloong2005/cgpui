#include "cgpui/ui/ui.hpp"

#include <expected>

namespace cgpui {

void PaintList::clear() {
  commands_.clear();
}

void PaintList::fill_rect(Rect rect, Color color) {
  commands_.push_back(PaintCommand{.solid_rect = SolidRect{.rect = rect, .color = color}});
}

std::span<const PaintCommand> PaintList::commands() const {
  return commands_;
}

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size) {
  auto frame = renderer.begin_frame();
  if (!frame) {
    return std::unexpected(frame.error());
  }

  (*frame)->clear(Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});

  PaintList paint_list;
  view.paint(paint_list, viewport_size);
  for (const auto& command : paint_list.commands()) {
    (*frame)->draw_rect(command.solid_rect);
  }

  return (*frame)->present();
}

} // namespace cgpui
