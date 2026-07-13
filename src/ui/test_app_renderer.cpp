#include "test_app_internal.hpp"

namespace cgpui::detail {

void TestRenderFrame::clear(Color) {}

void TestRenderFrame::draw_rect(const SolidRect&) {}

Result<void> TestRenderFrame::present() { return {}; }

Result<void> TestRenderer::resize(Size, DpiScale) { return {}; }

Result<std::unique_ptr<RenderFrame>> TestRenderer::begin_frame() {
  return std::make_unique<TestRenderFrame>();
}

void TestRootView::paint(PaintList&, Size) {}

} // namespace cgpui::detail
