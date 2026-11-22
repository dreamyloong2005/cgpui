#include "cgpui/renderer/renderer.hpp"

#include <memory>

namespace {
class DefaultRenderer : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::unsupported_platform,
        .message = "no drawing needed by capability test"});
  }
};

class PopupRenderer final : public DefaultRenderer {
 public:
  bool popup_windows_supported() const override { return true; }
};
}  // namespace

int main() {
  const DefaultRenderer unknown;
  if (unknown.popup_windows_supported()) return 1;
  const PopupRenderer capable;
  const cgpui::Renderer& renderer = capable;
  return renderer.popup_windows_supported() ? 0 : 2;
}
