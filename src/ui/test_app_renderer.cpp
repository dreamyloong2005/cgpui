#include "test_app_internal.hpp"

namespace cgpui::detail {

TestRenderFrame::TestRenderFrame(TestWindowRenderSnapshot& snapshot)
    : snapshot_(&snapshot) {}

void TestRenderFrame::clear(Color) { snapshot_->clear_count += 1; }

void TestRenderFrame::draw_rect(const SolidRect&) {
  snapshot_->draw_rect_count += 1;
}

Result<void> TestRenderFrame::present() {
  snapshot_->present_count += 1;
  return {};
}

Result<void> TestRenderer::resize(Size, DpiScale) {
  snapshot_.resize_count += 1;
  return {};
}

Result<std::unique_ptr<RenderFrame>> TestRenderer::begin_frame() {
  snapshot_.begin_frame_count += 1;
  return std::make_unique<TestRenderFrame>(snapshot_);
}

TestWindowRenderSnapshot TestRenderer::snapshot() const { return snapshot_; }

void TestRootView::paint(PaintList&, Size) {}

} // namespace cgpui::detail
