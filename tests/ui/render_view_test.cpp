#include "cgpui/ui/ui.hpp"

#include <memory>

class EmptyFrameRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>{};
  }
};

class EmptyView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

int main() {
  EmptyFrameRenderer renderer;
  EmptyView view;

  const auto result = cgpui::render_view(renderer, view, cgpui::Size{10.0F, 10.0F});
  if (result) {
    return 1;
  }

  return result.error().code == cgpui::ErrorCode::frame_acquisition_failed ? 0 : 2;
}
